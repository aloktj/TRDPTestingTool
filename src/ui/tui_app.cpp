#include "ui/tui_app.h"

#include "ui/screen_config_summary.h"

#include <ftxui/component/component.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/dom/elements.hpp>
#include <memory>
#include <numeric>
#include <string>
#include <utility>
#include <vector>

namespace trdp::ui
{
namespace
{
struct NavigationState
{
    std::vector<std::string> entries{"Dashboard", "PD View", "MD View", "Dataset Editor", "Logs", "Stats"};
    int selected{0};
};

ftxui::Component BuildDashboard(const config::SimulatorConfigLoadResult &result, const std::string &sourcePath)
{
    using namespace ftxui; // NOLINT

    const auto summaryText = [&]() {
        std::vector<Element> rows;
        const auto telegramCount = std::accumulate(
            result.config.interfaces.begin(), result.config.interfaces.end(), std::size_t{0},
            [](std::size_t total, const model::InterfaceConfig &iface) { return total + iface.telegrams.size(); });
        rows.push_back(text("TRDP Simulator – keyboard navigation"));
        rows.push_back(text("Loaded configuration: " + sourcePath));
        rows.push_back(text("Interfaces: " + std::to_string(result.config.interfaces.size())));
        rows.push_back(text("Datasets:   " + std::to_string(result.config.datasets.size())));
        rows.push_back(text("Telegrams:  " + std::to_string(telegramCount)));
        rows.push_back(separator());
        rows.push_back(text("Use Up/Down or j/k to move the menu, Enter/Space to select"));
        rows.push_back(text("Press Tab/Shift+Tab to cycle focus between menu and panel"));
        rows.push_back(text("Press q or Esc to quit safely"));
        return rows;
    }();

    return Renderer([summaryText] {
        return vbox({window(text("Dashboard"), vbox(summaryText)) | flex});
    });
}

ftxui::Component BuildPlaceholderPanel(const std::string &title, const std::string &body)
{
    using namespace ftxui; // NOLINT
    return Renderer([=] {
        return window(text(title),
                      vbox({
                          text(body),
                          text("Navigation: Arrow keys / Tab, quit: q or Esc"),
                      })) |
               flex;
    });
}
} // namespace

ftxui::Component MakeTuiApp(const config::SimulatorConfigLoadResult &result,
                            const std::string &sourcePath,
                            std::function<void()> onQuit)
{
    using namespace ftxui; // NOLINT

    auto navState = std::make_shared<NavigationState>();

    auto dashboard = BuildDashboard(result, sourcePath);
    auto pdView = MakeConfigSummaryScreen(result, sourcePath, onQuit);
    auto mdView = BuildPlaceholderPanel("MD View", "MD session monitoring and controls (upcoming)");
    auto datasetEditor = BuildPlaceholderPanel("Dataset Editor", "Edit dataset values and marshalling (upcoming)");
    auto logs = BuildPlaceholderPanel("Logs", "TRDP runtime logs and filtering (upcoming)");
    auto stats = BuildPlaceholderPanel("Stats", "PD/MD statistics and counters (upcoming)");

    auto contentPages = Container::Tab({dashboard, pdView, mdView, datasetEditor, logs, stats}, &navState->selected);
    auto menu = Menu(&navState->entries, &navState->selected);

    auto layout = Container::Horizontal({menu, contentPages});

    auto renderer = Renderer(layout, [menu, contentPages] {
        return ftxui::hbox({menu->Render() | ftxui::size(ftxui::WIDTH, ftxui::LESS_THAN, 24) | ftxui::border,
                            contentPages->Render() | ftxui::flex});
    });

    auto quitHandler = CatchEvent(renderer, [menu, navState, onQuit](const Event &event) {
        if (event == Event::Character('k'))
        {
            navState->selected = (navState->selected - 1 + static_cast<int>(navState->entries.size())) %
                                 static_cast<int>(navState->entries.size());
            return true;
        }
        if (event == Event::Character('j'))
        {
            navState->selected = (navState->selected + 1) % static_cast<int>(navState->entries.size());
            return true;
        }
        if (event == Event::Character('q') || event == Event::Character('Q') || event == Event::Escape)
        {
            if (onQuit)
            {
                onQuit();
            }
            return true;
        }
        return false;
    });

    return quitHandler;
}
} // namespace trdp::ui

