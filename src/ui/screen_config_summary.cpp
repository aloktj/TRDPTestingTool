#include "ui/screen_config_summary.h"

#include "trdp/pd_endpoint.h"
#include "util/logging.h"

#include <algorithm>
#include <cstdlib>
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <memory>
#include <sstream>

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
                              vbox(elements.empty() ? ftxui::Elements{text("No members parsed.")} : elements)));
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

    struct TelegramRuntimeRow
    {
        model::TelegramConfig config;
        std::shared_ptr<runtime::PdEndpointRuntime> runtime;
        std::shared_ptr<std::string> cycleInput;
        Component rowRenderer;
    };

    std::vector<std::shared_ptr<runtime::TrdpSession>> sessions;
    std::vector<TelegramRuntimeRow> runtimeRows;
    auto subscriberLog = std::make_shared<std::vector<std::string>>();

    for (const auto &iface : result.config.interfaces)
    {
        auto session = std::make_shared<runtime::TrdpSession>(runtime::TrdpSessionConfig{
            iface.hostIp,
            iface.leaderIp,
            iface.networkId,
        });
        session->open();
        sessions.push_back(session);

        for (const auto &telegram : iface.telegrams)
        {
            auto runtime = std::make_shared<runtime::PdEndpointRuntime>(telegram, session);
            session->registerPdSubscriber(telegram.comId, [runtime](const runtime::PdMessage &message) {
                runtime->handleSubscription(message);
            });

            runtime->setSubscriptionSink([subscriberLog, telegram](const runtime::PdMessage &message) {
                std::ostringstream oss;
                oss << util::formatTimestamp(message.timestamp) << " | ComID " << telegram.comId << " → Dataset "
                    << telegram.datasetId << " | " << message.payload.size() << " bytes";
                subscriberLog->push_back(oss.str());
                if (subscriberLog->size() > 50U)
                {
                    subscriberLog->erase(subscriberLog->begin());
                }
            });

            auto cycleInput = std::make_shared<std::string>("1000");
            auto cycleInputComponent = Input(cycleInput.get(), "cycle ms");
            auto startButton = Button("Start", [runtime, cycleInput] {
                const auto ms = std::max(1L, std::strtol(cycleInput->c_str(), nullptr, 10));
                runtime->startPublishing(std::chrono::milliseconds(ms));
            });
            auto stopButton = Button("Stop", [runtime] { runtime->stopPublishing(); });
            auto controls = Container::Horizontal({cycleInputComponent, startButton, stopButton});

            auto rowRenderer = Renderer(controls, [runtime, telegram, controls] {
                const auto lastPublish = runtime->lastPublishTime();
                std::string status = runtime->isPublishing() ? "Publishing" : "Stopped";
                if (lastPublish)
                {
                    status += " | last: " + util::formatTimestamp(*lastPublish);
                }
                status += " | count: " + std::to_string(runtime->publishCount());

                return hbox({
                    text("ComID " + std::to_string(telegram.comId) + " (Dataset " + std::to_string(telegram.datasetId) + ")"),
                    separator(),
                    controls->Render() | xflex,
                    separator(),
                    text(status),
                });
            });

            runtimeRows.push_back({telegram, runtime, cycleInput, rowRenderer});
        }
    }

    std::vector<Component> controlRows;
    controlRows.reserve(runtimeRows.size());
    for (auto &row : runtimeRows)
    {
        controlRows.push_back(row.rowRenderer);
    }

    auto controlContainer = Container::Vertical(controlRows);

    auto summaryRenderer = Renderer(controlContainer, [result, sourcePath, &runtimeRows, controlContainer, subscriberLog] {
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

        std::vector<Element> controlRenders;
        if (runtimeRows.empty())
        {
            controlRenders.push_back(text("No PD telegrams available."));
        }
        else
        {
            for (const auto &row : runtimeRows)
            {
                controlRenders.push_back(row.rowRenderer->Render());
            }
        }

        std::vector<Element> subscriberRows;
        for (const auto &entry : *subscriberLog)
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

    return summaryRenderer;
}
} // namespace trdp::ui
