#pragma once

#include "util/logging.h"

#include <trdp_if_light.h>

#include <atomic>
#include <chrono>
#include <cstdint>
#include <functional>
#include <mutex>
#include <optional>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

namespace trdp::runtime
{
struct TrdpSessionConfig
{
    std::string hostIp;
    std::string leaderIp;
    std::uint8_t networkId{0U};
};

struct PdMessage
{
    std::uint32_t comId{0};
    std::vector<std::uint8_t> payload;
    std::chrono::system_clock::time_point timestamp{std::chrono::system_clock::now()};
};

class TrdpSession
{
public:
    using PdCallback = std::function<void(const PdMessage &)>;

    explicit TrdpSession(TrdpSessionConfig config);
    ~TrdpSession();

    bool open();
    void close();
    [[nodiscard]] bool isOpen() const;

    void registerPdSubscriber(std::uint32_t comId, PdCallback callback);

    [[nodiscard]] TRDP_APP_SESSION_T appHandle() const;
    [[nodiscard]] TRDP_IP_ADDR_T hostAddress() const;

private:
    static void pdCallback(
        void *refCon,
        TRDP_APP_SESSION_T appHandle,
        const TRDP_PD_INFO_T *pMsg,
        UINT8 *pData,
        UINT32 dataSize);

    void onPdMessage(const TRDP_PD_INFO_T &msg, const std::uint8_t *data, std::uint32_t size);
    bool initializeStack();
    void startProcessThread();
    void stopProcessThread();
    void processLoop();

    TrdpSessionConfig config_;
    TRDP_APP_SESSION_T appHandle_{nullptr};
    TRDP_PD_CONFIG_T pdConfig_{};
    TRDP_PROCESS_CONFIG_T processConfig_{};
    TRDP_MEM_CONFIG_T memConfig_{};
    TRDP_IP_ADDR_T hostAddr_{0U};
    TRDP_IP_ADDR_T leaderAddr_{0U};

    bool opened_{false};
    std::atomic<bool> running_{false};
    std::thread processThread_{};
    mutable std::mutex mutex_;
    std::unordered_multimap<std::uint32_t, PdCallback> pdCallbacks_;
    std::unordered_map<std::uint32_t, TRDP_SUB_T> pdSubscriptions_;
};

} // namespace trdp::runtime
