#include "config/xml_loader.h"
#include "ui/tui_app.h"

#include <ftxui/component/screen_interactive.hpp>
#include <iostream>

int main(int argc, char **argv)
{
    const std::string configPath = argc > 1 ? argv[1] : "config.xml";

    auto result = trdp::config::loadSimulatorConfigFromXml(configPath);

    auto screen = ftxui::ScreenInteractive::TerminalOutput();
    auto app = trdp::ui::MakeTuiApp(result, configPath, screen.ExitLoopClosure());
    screen.Loop(app);

    return 0;
}
