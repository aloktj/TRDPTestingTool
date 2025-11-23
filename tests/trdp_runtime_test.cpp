#include "trdp/pd_endpoint.h"
#include "trdp/trdp_session.h"

#include <chrono>
#include <condition_variable>
#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

using trdp::model::TelegramConfig;
using trdp::model::TelegramEndpoint;
using trdp::runtime::PdEndpointRuntime;
using trdp::runtime::PdMessage;
using trdp::runtime::TrdpSession;
using trdp::runtime::TrdpSessionConfig;

namespace
{
TrdpSessionConfig loopbackSessionConfig()
{
    TrdpSessionConfig config{};
    config.hostIp = "127.0.0.1";
    config.leaderIp = "127.0.0.1";
    config.networkId = 0U;
    return config;
}

TelegramConfig loopbackTelegram(std::uint32_t comId)
{
    TelegramConfig config{};
    config.comId = comId;
    config.serviceId = 0U;
    config.destinations.push_back(TelegramEndpoint{0U, "", "127.0.0.1"});
    config.sources.push_back(TelegramEndpoint{0U, "", "127.0.0.1"});
    return config;
}
}

int main()
{
    auto session = std::make_shared<TrdpSession>(loopbackSessionConfig());
    if (!session->open())
    {
        std::cerr << "Failed to open TRDP session on loopback" << std::endl;
        return 1;
    }

    if (!session->isOpen())
    {
        std::cerr << "TRDP session should report open after successful open()" << std::endl;
        return 1;
    }

    constexpr std::uint32_t kTestComId = 0x12345U;
    auto telegram = loopbackTelegram(kTestComId);

    PdEndpointRuntime runtime(telegram, session);

    std::mutex mutex;
    std::condition_variable cv;
    bool received = false;
    PdMessage receivedMessage{};

    runtime.setSubscriptionSink([&](const PdMessage &msg) {
        std::lock_guard<std::mutex> lock(mutex);
        received = true;
        receivedMessage = msg;
        cv.notify_all();
    });

    session->registerPdSubscriber(kTestComId, [&runtime](const PdMessage &msg) { runtime.handleSubscription(msg); });

    runtime.startPublishing(std::chrono::milliseconds(20));

    {
        std::unique_lock<std::mutex> lock(mutex);
        const auto signaled = cv.wait_for(lock, std::chrono::seconds(2), [&] { return received; });
        if (!signaled)
        {
            std::cerr << "Did not receive PD message within timeout" << std::endl;
            runtime.stopPublishing();
            session->close();
            return 1;
        }
    }

    runtime.stopPublishing();
    session->close();

    if (receivedMessage.comId != kTestComId)
    {
        std::cerr << "Received PD comId does not match expected value" << std::endl;
        return 1;
    }

    if (!runtime.lastPublishTime().has_value())
    {
        std::cerr << "Publisher did not record a publish timestamp" << std::endl;
        return 1;
    }

    if (runtime.publishCount() == 0)
    {
        std::cerr << "Publisher did not send any payloads" << std::endl;
        return 1;
    }

    return 0;
}
