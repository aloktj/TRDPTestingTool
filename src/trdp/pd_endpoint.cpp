#include "trdp/pd_endpoint.h"

#include <algorithm>
#include <vos_sock.h>

#include <sstream>

namespace trdp::runtime
{
namespace
{
std::vector<std::uint8_t> makePayload(std::uint64_t count)
{
    std::vector<std::uint8_t> payload(sizeof(count));
    for (std::size_t i = 0; i < sizeof(count); ++i)
    {
        payload[i] = static_cast<std::uint8_t>((count >> (i * 8U)) & 0xFFU);
    }
    return payload;
}
}

PdEndpointRuntime::PdEndpointRuntime(model::TelegramConfig config, std::shared_ptr<TrdpSession> session)
    : config_(std::move(config)), session_(std::move(session))
{
}

PdEndpointRuntime::~PdEndpointRuntime()
{
    stopPublishing();
    if (worker_.joinable())
    {
        worker_.join();
    }
}

void PdEndpointRuntime::startPublishing(std::chrono::milliseconds cycleTime)
{
    stopPublishing();

    if (session_ == nullptr || !session_->isOpen())
    {
        util::logWarn("Cannot start PD publisher without an open TRDP session");
        return;
    }

    const auto appHandle = session_->appHandle();
    if (appHandle == nullptr)
    {
        util::logWarn("TRDP session handle unavailable; skipping publish start");
        return;
    }

    publishCount_.store(0);
    lastPublish_.reset();

    destIp_ = resolveDestinationIp();
    auto payload = buildPayload(0U);

    const auto intervalUs = static_cast<UINT32>(std::max<std::int64_t>(1, cycleTime.count()) * 1000);
    const auto pubErr = tlp_publish(
        appHandle,
        &pubHandle_,
        this,
        nullptr,
        config_.serviceId,
        config_.comId,
        0U,
        0U,
        session_->hostAddress(),
        destIp_,
        intervalUs,
        0U,
        TRDP_FLAGS_DEFAULT,
        nullptr,
        payload.data(),
        static_cast<UINT32>(payload.size()));

    if (pubErr != TRDP_NO_ERR)
    {
        std::ostringstream err;
        err << "Failed to publish PD comId " << config_.comId << " (error " << static_cast<int>(pubErr) << ")";
        util::logError(err.str());
        return;
    }

    const auto updateErr = tlc_updateSession(appHandle);
    if (updateErr != TRDP_NO_ERR)
    {
        std::ostringstream err;
        err << "tlc_updateSession failed after publish for comId " << config_.comId << " (error "
            << static_cast<int>(updateErr) << ")";
        util::logWarn(err.str());
    }

    {
        std::lock_guard<std::mutex> lock(mutex_);
        lastPublish_ = std::chrono::system_clock::now();
    }

    publishCount_.store(1);

    running_.store(true);
    worker_ = std::thread([this, cycleTime] { publishLoop(cycleTime); });

    std::ostringstream oss;
    oss << "Starting PD publisher for comId " << config_.comId << " every " << cycleTime.count() << " ms";
    util::logInfo(oss.str());
}

void PdEndpointRuntime::stopPublishing()
{
    const bool wasRunning = running_.exchange(false);
    if (wasRunning)
    {
        cv_.notify_all();
        if (worker_.joinable())
        {
            worker_.join();
        }

        if (session_ != nullptr && session_->appHandle() != nullptr && pubHandle_ != nullptr)
        {
            (void)tlp_unpublish(session_->appHandle(), pubHandle_);
        }
        pubHandle_ = nullptr;

        std::ostringstream oss;
        oss << "Stopping PD publisher for comId " << config_.comId;
        util::logInfo(oss.str());
    }
}

bool PdEndpointRuntime::isPublishing() const
{
    return running_.load();
}

std::uint64_t PdEndpointRuntime::publishCount() const
{
    return publishCount_.load();
}

std::optional<std::chrono::system_clock::time_point> PdEndpointRuntime::lastPublishTime() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return lastPublish_;
}

void PdEndpointRuntime::handleSubscription(const PdMessage &message)
{
    std::ostringstream oss;
    oss << "Received PD telegram comId=" << message.comId << " payload=" << message.payload.size() << " bytes";
    util::logDebug(oss.str());

    SubscriptionSink sink;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        sink = subscriptionSink_;
    }

    if (sink)
    {
        sink(message);
    }
}

void PdEndpointRuntime::setSubscriptionSink(SubscriptionSink sink)
{
    std::lock_guard<std::mutex> lock(mutex_);
    subscriptionSink_ = std::move(sink);
    util::logInfo("Registered PD subscription sink");
}

void PdEndpointRuntime::setFixedPayload(std::vector<std::uint8_t> payload)
{
    std::lock_guard<std::mutex> lock(mutex_);
    fixedPayload_ = std::move(payload);
}

void PdEndpointRuntime::clearFixedPayload()
{
    std::lock_guard<std::mutex> lock(mutex_);
    fixedPayload_.reset();
}

bool PdEndpointRuntime::hasFixedPayload() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return fixedPayload_.has_value();
}

std::optional<std::size_t> PdEndpointRuntime::fixedPayloadSize() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (fixedPayload_)
    {
        return fixedPayload_->size();
    }
    return std::nullopt;
}

void PdEndpointRuntime::publishLoop(std::chrono::milliseconds cycleTime)
{
    std::unique_lock<std::mutex> lock(mutex_);
    while (running_.load())
    {
        const auto wakeTime = std::chrono::system_clock::now() + cycleTime;
        cv_.wait_until(lock, wakeTime, [this] { return !running_.load(); });
        if (!running_.load())
        {
            break;
        }

        lastPublish_ = std::chrono::system_clock::now();
        const auto count = publishCount_.fetch_add(1) + 1;
        auto payload = buildPayload(count);

        lock.unlock();
        const auto err = tlp_put(
            session_->appHandle(),
            pubHandle_,
            payload.data(),
            static_cast<UINT32>(payload.size()));
        if (err != TRDP_NO_ERR)
        {
            std::ostringstream oss;
            oss << "Failed to queue PD payload for comId " << config_.comId << " (error " << static_cast<int>(err) << ")";
            util::logError(oss.str());
        }
        lock.lock();
    }
}

std::vector<std::uint8_t> PdEndpointRuntime::buildPayload(std::uint64_t count)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (fixedPayload_)
    {
        return *fixedPayload_;
    }

    return makePayload(count);
}

TRDP_IP_ADDR_T PdEndpointRuntime::resolveDestinationIp() const
{
    if (!config_.destinations.empty() && !config_.destinations.front().uriHost.empty())
    {
        return vos_dottedIP(config_.destinations.front().uriHost.c_str());
    }

    if (!config_.sources.empty() && !config_.sources.front().uriHost.empty())
    {
        return vos_dottedIP(config_.sources.front().uriHost.c_str());
    }

    return session_ != nullptr ? session_->hostAddress() : 0U;
}

} // namespace trdp::runtime
