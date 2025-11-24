#include "ui/screen_config_summary.h"

#include "trdp/pd_endpoint.h"
#include "util/logging.h"

#include <algorithm>
#include <cstdlib>
#include <ftxui/component/component.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/dom/elements.hpp>
#include <mutex>
#include <memory>
#include <sstream>

namespace trdp::ui
{
void SimulatorRuntimeContext::shutdown()
{
    if (shutdownRequested)
    {
        return;
    }

    shutdownRequested = true;
    for (auto &row : pdRows)
    {
        if (row.runtime)
        {
            row.runtime->stopPublishing();
        }
    }

    for (auto &session : sessions)
    {
        if (session)
        {
            session->close();
        }
    }
}

void SimulatorRuntimeContext::appendSubscriberLog(std::string entry)
{
    std::lock_guard<std::mutex> lock(subscriberMutex);
    subscriberLog.push_back(std::move(entry));
    if (subscriberLog.size() > 50U)
    {
        subscriberLog.erase(subscriberLog.begin());
    }
}

std::vector<std::string> SimulatorRuntimeContext::snapshotSubscriberLog() const
{
    std::lock_guard<std::mutex> lock(subscriberMutex);
    return subscriberLog;
}

SimulatorRuntimeContext::~SimulatorRuntimeContext()
{
    shutdown();
}

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
                              vbox(elements.empty() ? ftxui::Elements{text("No members parsed.")} : elements)));
    }

    if (rows.empty())
    {
        rows.push_back(text("No datasets available."));
    }

    return vbox(rows);
}
}

ftxui::Component MakeConfigSummaryScreen(const config::SimulatorConfigLoadResult &result,
                                         const std::string &sourcePath,
                                         const std::shared_ptr<SimulatorRuntimeContext> &runtime,
                                         std::function<void()> onQuit)
{
    using namespace ftxui; // NOLINT

    std::vector<Component> controlRows;
    controlRows.reserve(runtime->pdRows.size());
    for (auto &row : runtime->pdRows)
    {
        controlRows.push_back(row.rowRenderer);
    }

    auto controlContainer = Container::Vertical(controlRows);

    auto summaryRenderer = Renderer(controlContainer, [result, sourcePath, runtime, controlContainer] {
        std::vector<Element> sections;
        sections.push_back(text("Configuration source: " + sourcePath));
        sections.push_back(text("Press 'q' or Esc to quit") | color(Color::Yellow));

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

        std::vector<Element> controlRenders;
        if (runtime->pdRows.empty())
        {
            controlRenders.push_back(text("No PD telegrams available."));
        }
        else
        {
            for (const auto &row : runtime->pdRows)
            {
                controlRenders.push_back(row.rowRenderer->Render());
            }
        }

        std::vector<Element> subscriberRows;
        const auto logSnapshot = runtime->snapshotSubscriberLog();
        for (const auto &entry : logSnapshot)
        {
            subscriberRows.push_back(text(entry));
        }
        if (subscriberRows.empty())
        {
            subscriberRows.push_back(text("No PD updates received yet."));
        }

        sections.push_back(window(text("Interfaces"), vbox(interfacePanels)));
        sections.push_back(window(text("PD Publisher Control"), vbox(controlRenders)));
        sections.push_back(window(text("Subscriber updates"), vbox(subscriberRows)));
        sections.push_back(window(text("Datasets"), BuildDatasetPanel(result)));

        return vbox(sections) | border | flex;
    });

    auto quittingRenderer = CatchEvent(summaryRenderer, [runtime, onQuit](const Event &event) {
        if (event == Event::Character('q') || event == Event::Character('Q') || event == Event::Escape)
        {
            if (runtime)
            {
                runtime->shutdown();
            }
            if (onQuit)
            {
                onQuit();
            }
            return true;
        }
        return false;
    });

    return quittingRenderer;
}
} // namespace trdp::ui
