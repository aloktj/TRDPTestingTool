#include "ui/tui_app.h"

#include "ui/screen_config_summary.h"
#include "util/logging.h"

#include <ftxui/component/component.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/dom/elements.hpp>
#include <cctype>
#include <cstdint>
#include <memory>
#include <algorithm>
#include <iomanip>
#include <numeric>
#include <sstream>
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

std::vector<std::uint8_t> parseHexOrAscii(const std::string &input)
{
    std::vector<std::uint8_t> bytes;
    std::vector<std::string> tokens;
    std::string current;
    for (char c : input)
    {
        if (std::isspace(static_cast<unsigned char>(c)))
        {
            if (!current.empty())
            {
                tokens.push_back(current);
                current.clear();
            }
        }
        else
        {
            current.push_back(c);
        }
    }
    if (!current.empty())
    {
        tokens.push_back(current);
    }

    const auto isHexToken = [](const std::string &tok) {
        return std::all_of(tok.begin(), tok.end(), [](unsigned char ch) { return std::isxdigit(ch); });
    };

    const bool useHex = !tokens.empty() && std::all_of(tokens.begin(), tokens.end(), isHexToken);
    if (useHex)
    {
        for (const auto &tok : tokens)
        {
            const auto value = std::strtoul(tok.c_str(), nullptr, 16);
            bytes.push_back(static_cast<std::uint8_t>(value & 0xFFU));
        }
        return bytes;
    }

    bytes.assign(input.begin(), input.end());
    return bytes;
}

std::string bytesToHex(const std::vector<std::uint8_t> &bytes)
{
    std::ostringstream oss;
    oss << std::uppercase << std::hex << std::setfill('0');
    for (std::size_t i = 0; i < bytes.size(); ++i)
    {
        if (i > 0)
        {
            oss << ' ';
        }
        oss << std::setw(2) << static_cast<int>(bytes[i]);
    }
    return oss.str();
}

std::string directionLabel(runtime::PdDirection direction)
{
    switch (direction)
    {
    case runtime::PdDirection::Outgoing:
        return "Outgoing";
    case runtime::PdDirection::Incoming:
        return "Incoming";
    case runtime::PdDirection::Loopback:
        return "Loopback";
    case runtime::PdDirection::Unknown:
    default:
        return "Unknown";
    }
}

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

std::shared_ptr<SimulatorRuntimeContext> BuildRuntimeContext(const config::SimulatorConfigLoadResult &result)
{
    auto context = std::make_shared<SimulatorRuntimeContext>();

    for (const auto &iface : result.config.interfaces)
    {
        auto session = std::make_shared<runtime::TrdpSession>(runtime::TrdpSessionConfig{
            iface.hostIp,
            iface.leaderIp,
            iface.networkId,
        });
        session->open();
        context->sessions.push_back(session);

        for (const auto &telegram : iface.telegrams)
        {
            auto runtime = std::make_shared<runtime::PdEndpointRuntime>(telegram, session, iface.hostIp);
            session->registerPdSubscriber(telegram.comId, [runtime](const runtime::PdMessage &message) {
                runtime->handleSubscription(message);
            });

            runtime->setSubscriptionSink([context, telegram](const runtime::PdMessage &message) {
                if (!context)
                {
                    return;
                }

                std::ostringstream oss;
                oss << util::formatTimestamp(message.timestamp) << " | ComID " << telegram.comId << " → Dataset "
                    << telegram.datasetId << " | " << message.payload.size() << " bytes";
                context->appendSubscriberLog(oss.str());
            });

            auto cycleInput = std::make_shared<std::string>("1000");
            auto cycleInputComponent = ftxui::Input(cycleInput.get(), "cycle ms");
            auto txInput = std::make_shared<std::string>(bytesToHex(runtime->txPayload()));
            auto txInputComponent = ftxui::Input(txInput.get(), "TX payload (hex bytes or text)");

            auto startButton = ftxui::Button("Start", [runtime, cycleInput] {
                if (!runtime->canTransmit())
                {
                    return;
                }

                const auto ms = std::max(1L, std::strtol(cycleInput->c_str(), nullptr, 10));
                runtime->startPublishing(std::chrono::milliseconds(ms));
            });
            auto stopButton = ftxui::Button("Stop", [runtime] { runtime->stopPublishing(); });
            auto applyTxButton = ftxui::Button("Apply TX", [runtime, txInput] {
                if (!runtime->canTransmit())
                {
                    return;
                }

                runtime->setTxPayload(parseHexOrAscii(*txInput));
            });

            auto controls = ftxui::Container::Horizontal({cycleInputComponent, startButton, stopButton});
            auto txControls = ftxui::Container::Horizontal({txInputComponent, applyTxButton});

            auto rowRenderer = ftxui::Renderer(ftxui::Container::Vertical({controls, txControls}),
                                               [runtime, telegram, controls, txControls]() -> ftxui::Element {
                                                   const auto lastPublish = runtime->lastPublishTime();
                                                   const auto lastReceive = runtime->lastReceiveTime();
                                                   auto fixedSize = runtime->fixedPayloadSize();

                                                   auto statusBadge = ftxui::text(runtime->isPublishing() ? " RUNNING " : " STOPPED ") |
                                                                     ftxui::bgcolor(runtime->isPublishing()
                                                                                        ? ftxui::Color::Green
                                                                                        : ftxui::Color::Red) |
                                                                     ftxui::color(ftxui::Color::Black);

                                                   std::string txStatus = runtime->isPublishing() ? "Publishing" : "Stopped";
                                                   if (lastPublish)
                                                   {
                                                       txStatus += " | last TX: " + util::formatTimestamp(*lastPublish);
                                                   }
                                                   txStatus += " | tx count: " + std::to_string(runtime->publishCount());
                                                   if (fixedSize)
                                                   {
                                                       txStatus += " | fixed payload " + std::to_string(*fixedSize) +
                                                                   " bytes";
                                                   }

                                                   std::string rxStatus = "RX count: " + std::to_string(runtime->receiveCount());
                                                   if (lastReceive)
                                                   {
                                                       rxStatus += " | last RX: " + util::formatTimestamp(*lastReceive);
                                                   }

                                                   const auto direction = runtime->direction();
                                                   const auto directionText = directionLabel(direction);
                                                   auto directionBadge = ftxui::text(" " + directionText + " ") |
                                                                         ftxui::bgcolor(direction == runtime::PdDirection::Loopback
                                                                                            ? ftxui::Color::Yellow
                                                                                            : direction == runtime::PdDirection::Outgoing
                                                                                                ? ftxui::Color::Green
                                                                                                : direction == runtime::PdDirection::Incoming
                                                                                                    ? ftxui::Color::Blue
                                                                                                    : ftxui::Color::GrayDark) |
                                                                         ftxui::color(ftxui::Color::Black);

                                                   const auto txPayloadBytes = runtime->txPayload();
                                                   const auto rxPayloadBytes = runtime->rxPayload();
                                                   const auto txPayloadHex = bytesToHex(txPayloadBytes);
                                                   const auto rxPayloadHex = bytesToHex(rxPayloadBytes);

                                                   auto txPane = runtime->canTransmit()
                                                                     ? ftxui::vbox(ftxui::Elements{
                                                                           ftxui::text("TX payload (" +
                                                                                       std::to_string(txPayloadBytes.size()) +
                                                                                       " bytes)") |
                                                                               ftxui::bold,
                                                                           txControls->Render() | ftxui::xflex,
                                                                           ftxui::paragraph(txPayloadHex.empty() ? "<empty>" : txPayloadHex),
                                                                       })
                                                                     : ftxui::vbox(ftxui::Elements{ftxui::text("Transmit disabled for this telegram")});

                                                   auto rxPane = runtime->canReceive()
                                                                     ? ftxui::vbox(ftxui::Elements{
                                                                           ftxui::text("Last RX payload (" +
                                                                                       std::to_string(rxPayloadBytes.size()) +
                                                                                       " bytes)") |
                                                                               ftxui::bold,
                                                                           ftxui::paragraph(rxPayloadHex.empty() ? "<no data yet>" :
                                                                                                               rxPayloadHex),
                                                                       })
                                                                     : ftxui::vbox(ftxui::Elements{ftxui::text("Receive disabled for this telegram")});

                                                   auto controlRender = runtime->canTransmit()
                                                                              ? controls->Render()
                                                                              : ftxui::text("TX controls disabled (receive-only)");

                                                   return ftxui::vbox(ftxui::Elements{
                                                       ftxui::hbox(ftxui::Elements{ftxui::text("ComID " + std::to_string(telegram.comId) +
                                                                                               " (Dataset " +
                                                                                               std::to_string(telegram.datasetId) + ")"),
                                                                   ftxui::separator(),
                                                                   directionBadge}),
                                                       ftxui::separator(),
                                                       ftxui::text(txStatus),
                                                       ftxui::text(rxStatus),
                                                       ftxui::separator(),
                                                       ftxui::hbox(ftxui::Elements{ftxui::window(ftxui::text("TX"), txPane | ftxui::xflex),
                                                                                   ftxui::separator(),
                                                                                   ftxui::window(ftxui::text("RX"), rxPane | ftxui::xflex)}) |
                                                           ftxui::xflex,
                                                       ftxui::separator(),
                                                       controlRender,
                                                   });
                                               });

            context->pdRows.push_back(PdControlRow{telegram, runtime, cycleInput, txInput, rowRenderer});
        }
    }

    return context;
}

ftxui::Component BuildDatasetEditor(const config::SimulatorConfigLoadResult &result,
                                    const std::shared_ptr<SimulatorRuntimeContext> &context)
{
    using namespace ftxui; // NOLINT

    struct DatasetState
    {
        model::Dataset dataset;
        std::vector<std::string> values;
        std::string status;
    };

    std::vector<std::shared_ptr<DatasetState>> datasetStates;
    for (const auto &ds : result.config.datasets)
    {
        auto state = std::make_shared<DatasetState>(DatasetState{ds, std::vector<std::string>(ds.elements.size(), ""), ""});
        datasetStates.push_back(std::move(state));
    }

    std::vector<Component> datasetPanels;
    datasetPanels.reserve(datasetStates.size());

    for (auto &dsState : datasetStates)
    {
        std::vector<Component> elementRows;
        for (std::size_t idx = 0; idx < dsState->dataset.elements.size(); ++idx)
        {
            auto &element = dsState->dataset.elements[idx];
            auto input = Input(&dsState->values[idx], element.name + " (hex bytes or text)");
            auto clearButton = Button("Clear", [dsState, idx] { dsState->values[idx].clear(); });
            elementRows.push_back(Container::Horizontal({input, clearButton}));
        }

        auto applyDataset = Button("Apply dataset", [dsState, context] {
            std::vector<std::uint8_t> payload;
            for (const auto &value : dsState->values)
            {
                auto bytes = parseHexOrAscii(value);
                payload.insert(payload.end(), bytes.begin(), bytes.end());
            }

            for (auto &row : context->pdRows)
            {
                if (row.config.datasetId == dsState->dataset.id)
                {
                    row.runtime->setFixedPayload(payload);
                }
            }

            std::ostringstream oss;
            oss << "Dataset " << dsState->dataset.id << " fixed to " << payload.size() << " bytes";
            dsState->status = oss.str();
        });

        auto clearDataset = Button("Clear dataset override", [dsState, context] {
            for (auto &row : context->pdRows)
            {
                if (row.config.datasetId == dsState->dataset.id)
                {
                    row.runtime->clearFixedPayload();
                }
            }
            dsState->status = "Dataset override cleared";
        });

        std::vector<Component> panelComponents = elementRows;
        auto controlRow = Container::Horizontal({applyDataset, clearDataset});
        panelComponents.push_back(controlRow);

        auto panelContainer = Container::Vertical(std::move(panelComponents));

        auto panel = Renderer(panelContainer, [dsState, elementRows, controlRow, applyDataset, clearDataset] {
            std::vector<Element> rows;
            for (std::size_t idx = 0; idx < dsState->dataset.elements.size(); ++idx)
            {
                const auto &element = dsState->dataset.elements[idx];
                rows.push_back(hbox({text(element.name + " : " + element.type +
                                         (element.arraySize > 1 ? "[" + std::to_string(element.arraySize) + "]" : "")),
                                     separator(),
                                     elementRows[idx]->Render() | xflex}));
            }

            rows.push_back(separator());
            rows.push_back(controlRow->Render());
            if (!dsState->status.empty())
            {
                rows.push_back(text(dsState->status) | color(Color::Green));
            }

            return window(text("Dataset " + std::to_string(dsState->dataset.id) + " - " + dsState->dataset.name),
                          vbox(std::move(rows)) | yframe | vscroll_indicator);
        });

        datasetPanels.push_back(panel);
    }

    if (datasetPanels.empty())
    {
        datasetPanels.push_back(Renderer([] { return text("No datasets available."); }));
    }

    auto container = Container::Vertical(datasetPanels);
    return Renderer(container, [container] { return vbox({container->Render() | flex | yframe | vscroll_indicator}); });
}
} // namespace

ftxui::Component MakeTuiApp(const config::SimulatorConfigLoadResult &result,
                            const std::string &sourcePath,
                            std::function<void()> onQuit)
{
    using namespace ftxui; // NOLINT

    auto navState = std::make_shared<NavigationState>();
    auto runtime = BuildRuntimeContext(result);

    auto dashboard = BuildDashboard(result, sourcePath);
    auto pdView = MakeConfigSummaryScreen(result, sourcePath, runtime, onQuit);
    auto mdView = BuildPlaceholderPanel("MD View", "MD session monitoring and controls (upcoming)");
    auto datasetEditor = BuildDatasetEditor(result, runtime);
    auto logs = BuildPlaceholderPanel("Logs", "TRDP runtime logs and filtering (upcoming)");
    auto stats = BuildPlaceholderPanel("Stats", "PD/MD statistics and counters (upcoming)");

    auto contentPages = Container::Tab({dashboard, pdView, mdView, datasetEditor, logs, stats}, &navState->selected);
    auto menu = Menu(&navState->entries, &navState->selected);

    auto layout = Container::Horizontal({menu, contentPages});

    auto renderer = Renderer(layout, [menu, contentPages] {
        return ftxui::hbox({menu->Render() | ftxui::size(ftxui::WIDTH, ftxui::LESS_THAN, 24) | ftxui::border,
                            contentPages->Render() | ftxui::flex});
    });

    auto quitHandler = CatchEvent(renderer, [menu, navState, onQuit, runtime](const Event &event) {
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

    return quitHandler;
}
} // namespace trdp::ui

