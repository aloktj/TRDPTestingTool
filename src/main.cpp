#include "config/xml_loader.h"
#include "ui/screen_config_summary.h"

#include <ftxui/component/screen_interactive.hpp>
#include <iostream>

int main(int argc, char **argv)
{
    const std::string configPath = argc > 1 ? argv[1] : "config.xml";

    auto result = trdp::config::loadSimulatorConfigFromXml(configPath);

    auto screen = ftxui::ScreenInteractive::TerminalOutput();
    auto summary = trdp::ui::MakeConfigSummaryScreen(result, configPath, screen.ExitLoopClosure());
    screen.Loop(summary);

    return 0;
}
