#include "trdp/pd_endpoint.h"

#include <algorithm>
#include <vos_sock.h>

#include <future>
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
    publishBuffer_ = buildPayload(0U);

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
        publishBuffer_.data(),
        static_cast<UINT32>(publishBuffer_.size()));

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
        publishCount_.store(1);
    }

    running_.store(true);

    std::ostringstream oss;
    oss << "Starting PD publisher for comId " << config_.comId << " every " << cycleTime.count() << " ms";
    util::logInfo(oss.str());
}

void PdEndpointRuntime::stopPublishing()
{
    util::logDebug("stopPublishing invoked");
    const bool wasRunning = running_.exchange(false);
    if (wasRunning)
    {
        if (session_ != nullptr && session_->appHandle() != nullptr && pubHandle_ != nullptr)
        {
            auto *appHandle = session_->appHandle();
            auto *pubHandle = pubHandle_;
            util::logDebug("Calling tlp_unpublish");
            auto future = std::async(std::launch::async, [appHandle, pubHandle] {
                return tlp_unpublish(appHandle, pubHandle);
            });

            if (future.wait_for(std::chrono::milliseconds(500)) == std::future_status::ready)
            {
                (void)future.get();
                util::logDebug("tlp_unpublish completed");
            }
            else
            {
                util::logWarn("tlp_unpublish timed out; continuing shutdown");
            }
        }
        pubHandle_ = nullptr;
        publishBuffer_.clear();

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
        lastPublish_ = message.timestamp;
        publishCount_.fetch_add(1);
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
