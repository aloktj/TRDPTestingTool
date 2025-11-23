#pragma once

#include "config/xml_loader.h"

#include <ftxui/component/component.hpp>
#include <functional>
#include <string>

namespace trdp::ui
{
/**
 * Build the root TUI component with keyboard navigation across the primary panels
 * described in the SRS/SAS (Dashboard, PD, MD, Dataset Editor, Logs, Stats).
 */
ftxui::Component MakeTuiApp(const config::SimulatorConfigLoadResult &result,
                            const std::string &sourcePath,
                            std::function<void()> onQuit = {});
} // namespace trdp::ui

