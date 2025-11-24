#include "trdp/trdp_session.h"

#include <vos_sock.h>
#include <vos_utils.h>

#include <algorithm>
#include <cstring>
#include <sstream>

namespace trdp::runtime
{
namespace
{
std::atomic_uint32_t g_sessionCount{0U};
std::once_flag g_stackInitFlag;
TRDP_ERR_T g_stackInitResult = TRDP_NO_ERR;

std::string makeErrorMessage(const std::string &context, TRDP_ERR_T err)
{
    std::ostringstream oss;
    oss << context << " (error " << static_cast<int>(err) << ": "
        << (err == TRDP_NO_ERR ? "ok" : vos_getErrorString(static_cast<VOS_ERR_T>(err))) << ")";
    return oss.str();
}
}

TrdpSession::TrdpSession(TrdpSessionConfig config) : config_(std::move(config)) {}

TrdpSession::~TrdpSession()
{
    close();
}

bool TrdpSession::initializeStack()
{
    std::call_once(g_stackInitFlag, [&] {
        memConfig_.p = nullptr;
        memConfig_.size = 0U;
        std::fill(std::begin(memConfig_.prealloc), std::end(memConfig_.prealloc), 0U);

        g_stackInitResult = tlc_init(nullptr, nullptr, &memConfig_);
        if (g_stackInitResult == TRDP_NO_ERR)
        {
            util::logInfo("Initialized TRDP stack");
        }
        else
        {
            util::logError(makeErrorMessage("Failed to initialize TRDP stack", g_stackInitResult));
        }
    });

    if (g_stackInitResult != TRDP_NO_ERR)
    {
        return false;
    }
    return true;
}

bool TrdpSession::open()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (opened_)
    {
        util::logWarn("TRDP session already open; skipping reinitialization");
        return true;
    }

    if (!initializeStack())
    {
        return false;
    }

    hostAddr_ = vos_dottedIP(config_.hostIp.c_str());
    leaderAddr_ = vos_dottedIP(config_.leaderIp.c_str());

    pdConfig_.pfCbFunction = &TrdpSession::pdCallback;
    pdConfig_.pRefCon = this;
    pdConfig_.sendParam = TRDP_PD_DEFAULT_SEND_PARAM;
    pdConfig_.flags = TRDP_FLAGS_CALLBACK;
    pdConfig_.timeout = TRDP_PD_DEFAULT_TIMEOUT;
    pdConfig_.toBehavior = TRDP_TO_SET_TO_ZERO;
    pdConfig_.port = 0U;

    processConfig_.cycleTime = TRDP_PROCESS_DEFAULT_CYCLE_TIME;
    processConfig_.priority = 0U;
    processConfig_.options = TRDP_OPTION_BLOCK;
    std::memset(processConfig_.hostName, 0, sizeof(processConfig_.hostName));
    std::memset(processConfig_.leaderName, 0, sizeof(processConfig_.leaderName));
    std::memset(processConfig_.type, 0, sizeof(processConfig_.type));
    std::strncpy(processConfig_.hostName, config_.hostIp.c_str(), sizeof(processConfig_.hostName) - 1U);
    std::strncpy(processConfig_.leaderName, config_.leaderIp.c_str(), sizeof(processConfig_.leaderName) - 1U);

    const auto openErr = tlc_openSession(
        &appHandle_,
        hostAddr_,
        leaderAddr_,
        nullptr,
        &pdConfig_,
        nullptr,
        &processConfig_);
    if (openErr != TRDP_NO_ERR)
    {
        util::logError(makeErrorMessage("Failed to open TRDP session", openErr));
        return false;
    }

    g_sessionCount.fetch_add(1U);
    startProcessThread();

    opened_ = true;
    std::ostringstream oss;
    oss << "Opened TRDP Light session on host " << config_.hostIp << " (leader " << config_.leaderIp
        << ", network " << static_cast<int>(config_.networkId) << ")";
    util::logInfo(oss.str());
    return true;
}

void TrdpSession::startProcessThread()
{
    running_.store(true);
    processThread_ = std::thread([this] { processLoop(); });
}

void TrdpSession::stopProcessThread()
{
    running_.store(false);
    if (processThread_.joinable())
    {
        processThread_.join();
    }
}

void TrdpSession::close()
{
    TRDP_APP_SESSION_T handleToClose = nullptr;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!opened_)
        {
            return;
        }

        opened_ = false;
        running_.store(false);
        handleToClose = appHandle_;
    }

    stopProcessThread();

    {
        std::lock_guard<std::mutex> lock(mutex_);
        for (auto &entry : pdSubscriptions_)
        {
            (void)tlp_unsubscribe(handleToClose, entry.second);
        }
        pdSubscriptions_.clear();
    }

    if (handleToClose != nullptr)
    {
        const auto err = tlc_closeSession(handleToClose);
        if (err != TRDP_NO_ERR)
        {
            util::logError(makeErrorMessage("Failed to close TRDP session", err));
        }
        appHandle_ = nullptr;
    }

    const auto remaining = g_sessionCount.fetch_sub(1U) - 1U;
    if (remaining == 0U)
    {
        const auto termErr = tlc_terminate();
        if (termErr != TRDP_NO_ERR)
        {
            util::logError(makeErrorMessage("Failed to terminate TRDP stack", termErr));
        }
        else
        {
            util::logInfo("Terminated TRDP stack");
        }
    }

    util::logInfo("Closed TRDP Light session");
}

bool TrdpSession::isOpen() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return opened_;
}

TRDP_APP_SESSION_T TrdpSession::appHandle() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return appHandle_;
}

TRDP_IP_ADDR_T TrdpSession::hostAddress() const
{
    return hostAddr_;
}

const std::string &TrdpSession::hostIpString() const
{
    return config_.hostIp;
}

void TrdpSession::registerPdSubscriber(std::uint32_t comId, PdCallback callback)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (!opened_ || appHandle_ == nullptr)
    {
        util::logWarn("TRDP session not open; cannot register PD subscriber");
        return;
    }

    pdCallbacks_.emplace(comId, std::move(callback));

    if (pdSubscriptions_.find(comId) == pdSubscriptions_.end())
    {
        TRDP_SUB_T subHandle{};
        const auto err = tlp_subscribe(
            appHandle_,
            &subHandle,
            this,
            &TrdpSession::pdCallback,
            0U,
            comId,
            0U,
            0U,
            0U,
            0U,
            hostAddr_,
            TRDP_FLAGS_DEFAULT,
            nullptr,
            TRDP_PD_DEFAULT_TIMEOUT,
            TRDP_TO_SET_TO_ZERO);

        if (err == TRDP_NO_ERR)
        {
            pdSubscriptions_.emplace(comId, subHandle);
            util::logDebug("Subscribed for PD comId " + std::to_string(comId));

            const auto updateErr = tlc_updateSession(appHandle_);
            if (updateErr != TRDP_NO_ERR)
            {
                util::logWarn(makeErrorMessage("tlc_updateSession failed after subscribe", updateErr));
            }
        }
        else
        {
            util::logError(makeErrorMessage("Failed to subscribe PD", err));
        }
    }
}

void TrdpSession::processLoop()
{
    while (running_.load())
    {
        TRDP_TIME_T interval{};
        TRDP_FDS_T rfds{};
        TRDP_SOCK_T noDesc = 0;
        FD_ZERO(&rfds);

        const auto intervalErr = tlc_getInterval(appHandle_, &interval, &rfds, &noDesc);
        if (intervalErr != TRDP_NO_ERR)
        {
            interval.tv_sec = 0;
            interval.tv_usec = TRDP_PROCESS_DEFAULT_CYCLE_TIME;
        }

        const INT32 ready = vos_select(noDesc, &rfds, nullptr, nullptr, &interval);
        (void)ready;

        INT32 count = 0;
        const auto processErr = tlc_process(appHandle_, &rfds, &count);
        if (processErr != TRDP_NO_ERR)
        {
            util::logWarn(makeErrorMessage("tlc_process reported error", processErr));
        }
    }
}

void TrdpSession::pdCallback(
    void *refCon,
    TRDP_APP_SESSION_T appHandle,
    const TRDP_PD_INFO_T *pMsg,
    UINT8 *pData,
    UINT32 dataSize)
{
    if (refCon == nullptr || pMsg == nullptr)
    {
        return;
    }

    auto *session = static_cast<TrdpSession *>(refCon);
    session->onPdMessage(*pMsg, pData, dataSize);
}

void TrdpSession::onPdMessage(const TRDP_PD_INFO_T &msg, const std::uint8_t *data, std::uint32_t size)
{
    if (msg.resultCode != TRDP_NO_ERR)
    {
        util::logWarn(makeErrorMessage("PD reception reported error", msg.resultCode));
    }

    std::vector<PdCallback> callbacks;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto range = pdCallbacks_.equal_range(msg.comId);
        for (auto it = range.first; it != range.second; ++it)
        {
            callbacks.push_back(it->second);
        }
    }

    if (callbacks.empty())
    {
        util::logWarn("No PD subscribers registered for comId " + std::to_string(msg.comId));
        return;
    }

    std::vector<std::uint8_t> payload;
    payload.reserve(size);
    payload.insert(payload.end(), data, data + size);

    PdMessage message{msg.comId, std::move(payload), std::chrono::system_clock::now()};
    for (const auto &callback : callbacks)
    {
        callback(message);
    }
}

} // namespace trdp::runtime
