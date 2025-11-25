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

    PdEndpointRuntime runtime(telegram, session, session->hostIpString());

    std::mutex mutex;
    std::condition_variable cv;
    std::uint64_t received{0};

    session->registerPdSubscriber(kTestComId, [&](const PdMessage &) {
        std::lock_guard<std::mutex> lock(mutex);
        ++received;
        cv.notify_all();
    });

    std::cout << "Starting publisher" << std::endl;
    runtime.startPublishing(std::chrono::milliseconds(20));

    {
        std::unique_lock<std::mutex> lock(mutex);
        cv.wait_for(lock, std::chrono::milliseconds(500), [&] { return received >= 3U; });
    }

    std::cout << "Stopping publisher" << std::endl;
    runtime.stopPublishing();

    std::cout << "Closing session" << std::endl;
    session->close();
    std::cout << "Session closed" << std::endl;

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
