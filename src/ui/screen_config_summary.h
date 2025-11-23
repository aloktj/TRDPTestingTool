#pragma once

#include "config/xml_loader.h"
#include "trdp/pd_endpoint.h"
#include "trdp/trdp_session.h"

#include <ftxui/component/component.hpp>
#include <functional>

namespace trdp::ui
{
struct PdControlRow
{
    model::TelegramConfig config;
    std::shared_ptr<runtime::PdEndpointRuntime> runtime;
    std::shared_ptr<std::string> cycleInput;
    ftxui::Component rowRenderer;
};

struct SimulatorRuntimeContext
{
    std::vector<std::shared_ptr<runtime::TrdpSession>> sessions;
    std::vector<PdControlRow> pdRows;
    std::shared_ptr<std::vector<std::string>> subscriberLog = std::make_shared<std::vector<std::string>>();
    bool shutdownRequested{false};

    void shutdown();
    ~SimulatorRuntimeContext();
};

ftxui::Component MakeConfigSummaryScreen(const config::SimulatorConfigLoadResult &result,
                                         const std::string &sourcePath,
                                         const std::shared_ptr<SimulatorRuntimeContext> &runtime,
                                         std::function<void()> onQuit = {});
}
