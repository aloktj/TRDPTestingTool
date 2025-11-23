#pragma once

#include "model/sim_config.h"
#include "trdp/trdp_session.h"
#include "util/logging.h"

#include <trdp_if_light.h>

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <thread>

namespace trdp::runtime
{
class PdEndpointRuntime
{
public:
    using SubscriptionSink = std::function<void(const PdMessage &)>;

    PdEndpointRuntime(model::TelegramConfig config, std::shared_ptr<TrdpSession> session);
    ~PdEndpointRuntime();

    PdEndpointRuntime(const PdEndpointRuntime &) = delete;
    PdEndpointRuntime &operator=(const PdEndpointRuntime &) = delete;

    void startPublishing(std::chrono::milliseconds cycleTime);
    void stopPublishing();

    [[nodiscard]] bool isPublishing() const;
    [[nodiscard]] std::uint64_t publishCount() const;
    [[nodiscard]] std::optional<std::chrono::system_clock::time_point> lastPublishTime() const;

    void handleSubscription(const PdMessage &message);
    void setSubscriptionSink(SubscriptionSink sink);

    void setFixedPayload(std::vector<std::uint8_t> payload);
    void clearFixedPayload();
    [[nodiscard]] bool hasFixedPayload() const;
    [[nodiscard]] std::optional<std::size_t> fixedPayloadSize() const;

private:
    void publishLoop(std::chrono::milliseconds cycleTime);
    TRDP_IP_ADDR_T resolveDestinationIp() const;
    std::vector<std::uint8_t> buildPayload(std::uint64_t count);

    model::TelegramConfig config_;
    std::shared_ptr<TrdpSession> session_;
    TRDP_PUB_T pubHandle_{nullptr};
    TRDP_IP_ADDR_T destIp_{0U};
    std::thread worker_;
    std::atomic<bool> running_{false};
    std::atomic<std::uint64_t> publishCount_{0};
    std::optional<std::chrono::system_clock::time_point> lastPublish_;
    std::optional<std::vector<std::uint8_t>> fixedPayload_{};
    std::condition_variable cv_;
    mutable std::mutex mutex_;
    SubscriptionSink subscriptionSink_{};
};

} // namespace trdp::runtime
