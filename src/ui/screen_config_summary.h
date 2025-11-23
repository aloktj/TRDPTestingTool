#pragma once

#include "config/xml_loader.h"

#include <ftxui/component/component.hpp>

namespace trdp::ui
{
ftxui::Component MakeConfigSummaryScreen(const config::SimulatorConfigLoadResult &result,
                                         const std::string &sourcePath);
}
