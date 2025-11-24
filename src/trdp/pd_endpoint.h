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
#include <string>
#include <thread>

namespace trdp::runtime
{
enum class PdDirection
{
    Unknown,
    Outgoing,
    Incoming,
    Loopback,
};

class PdEndpointRuntime
{
public:
    using SubscriptionSink = std::function<void(const PdMessage &)>;

    PdEndpointRuntime(model::TelegramConfig config, std::shared_ptr<TrdpSession> session, std::string hostIp);
    ~PdEndpointRuntime();

    PdEndpointRuntime(const PdEndpointRuntime &) = delete;
    PdEndpointRuntime &operator=(const PdEndpointRuntime &) = delete;

    void startPublishing(std::chrono::milliseconds cycleTime);
    void stopPublishing();

    [[nodiscard]] bool isPublishing() const;
    [[nodiscard]] std::uint64_t publishCount() const;
    [[nodiscard]] std::optional<std::chrono::system_clock::time_point> lastPublishTime() const;
    [[nodiscard]] std::optional<std::chrono::system_clock::time_point> lastReceiveTime() const;
    [[nodiscard]] std::uint64_t receiveCount() const;

    void handleSubscription(const PdMessage &message);
    void setSubscriptionSink(SubscriptionSink sink);

    void setFixedPayload(std::vector<std::uint8_t> payload);
    void clearFixedPayload();
    [[nodiscard]] bool hasFixedPayload() const;
    [[nodiscard]] std::optional<std::size_t> fixedPayloadSize() const;

    void setTxPayload(std::vector<std::uint8_t> payload);
    [[nodiscard]] std::vector<std::uint8_t> txPayload() const;
    [[nodiscard]] std::vector<std::uint8_t> rxPayload() const;

    [[nodiscard]] PdDirection direction() const;
    [[nodiscard]] bool canTransmit() const;
    [[nodiscard]] bool canReceive() const;

private:
    static PdDirection classifyDirection(const std::string &hostIp, const model::TelegramConfig &config);

    TRDP_IP_ADDR_T resolveDestinationIp() const;
    std::vector<std::uint8_t> buildPayload(std::uint64_t count);

    model::TelegramConfig config_;
    std::shared_ptr<TrdpSession> session_;
    std::string hostIp_;
    PdDirection direction_{PdDirection::Unknown};
    TRDP_PUB_T pubHandle_{nullptr};
    TRDP_IP_ADDR_T destIp_{0U};
    std::vector<std::uint8_t> publishBuffer_{};
    std::vector<std::uint8_t> txPayload_{};
    std::vector<std::uint8_t> rxPayload_{};
    std::atomic<bool> running_{false};
    std::atomic<std::uint64_t> publishCount_{0};
    std::atomic<std::uint64_t> receiveCount_{0};
    std::optional<std::chrono::system_clock::time_point> lastPublish_;
    std::optional<std::chrono::system_clock::time_point> lastReceive_;
    std::optional<std::vector<std::uint8_t>> fixedPayload_{};
    mutable std::mutex mutex_;
    SubscriptionSink subscriptionSink_{};
};

} // namespace trdp::runtime
