#include "ui/screen_config_summary.h"

#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>

namespace trdp::ui
{
namespace
{
ftxui::Element BuildInterfacePanel(const model::InterfaceConfig &iface)
{
    using namespace ftxui; // NOLINT

    std::vector<Element> telegramElements;
    for (const auto &telegram : iface.telegrams)
    {
        std::vector<Element> endpointRows;
        for (const auto &dst : telegram.destinations)
        {
            endpointRows.push_back(text("Dest " + std::to_string(dst.id) + ": " + dst.uriHost + " (" + dst.uriUser + ")"));
        }
        for (const auto &src : telegram.sources)
        {
            endpointRows.push_back(text("Src  " + std::to_string(src.id) + ": " + src.uriHost + " (" + src.uriUser + ")"));
        }

        telegramElements.push_back(
            vbox({
                     text("ComID " + std::to_string(telegram.comId) + " → Dataset " + std::to_string(telegram.datasetId) +
                          " [" + telegram.exchangeType + "]"),
                     vbox(endpointRows),
                 }) |
            border);
    }

    if (telegramElements.empty())
    {
        telegramElements.push_back(text("No telegrams parsed."));
    }

    return window(text(iface.name),
                  vbox({
                           text("Network ID: " + std::to_string(iface.networkId)),
                           text("Host IP: " + iface.hostIp + " | Leader IP: " + iface.leaderIp),
                           separator(),
                           vbox(telegramElements),
                       }));
}

ftxui::Element BuildDatasetPanel(const config::SimulatorConfigLoadResult &result)
{
    using namespace ftxui; // NOLINT
    std::vector<Element> rows;
    for (const auto &dataset : result.config.datasets)
    {
        std::vector<Element> elements;
        for (const auto &member : dataset.elements)
        {
            elements.push_back(text("- " + member.name + " : " + member.type +
                                   (member.arraySize > 1 ? "[" + std::to_string(member.arraySize) + "]" : "")));
        }

        rows.push_back(window(text("Dataset " + std::to_string(dataset.id) + " - " + dataset.name),
                              vbox(elements.empty() ? Elements{text("No members parsed.")} : elements)));
    }

    if (rows.empty())
    {
        rows.push_back(text("No datasets available."));
    }

    return vbox(rows);
}
}

ftxui::Component MakeConfigSummaryScreen(const config::SimulatorConfigLoadResult &result, const std::string &sourcePath)
{
    using namespace ftxui; // NOLINT

    auto summaryRenderer = Renderer([result, sourcePath] {
        std::vector<Element> sections;
        sections.push_back(text("Configuration source: " + sourcePath));

        if (result.hasErrors())
        {
            std::vector<Element> errorRows;
            for (const auto &err : result.errors)
            {
                errorRows.push_back(text("• " + err) | color(Color::Red));
            }
            sections.push_back(window(text("Validation errors"), vbox(errorRows)) | bgcolor(Color::DarkRed));
        }

        std::vector<Element> interfacePanels;
        for (const auto &iface : result.config.interfaces)
        {
            interfacePanels.push_back(BuildInterfacePanel(iface));
        }
        if (interfacePanels.empty())
        {
            interfacePanels.push_back(text("No interfaces found."));
        }

        sections.push_back(window(text("Interfaces"), vbox(interfacePanels)));
        sections.push_back(window(text("Datasets"), BuildDatasetPanel(result)));

        return vbox(sections) | border | flex;
    });

    return summaryRenderer;
}
} // namespace trdp::ui
