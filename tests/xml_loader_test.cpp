#include "config/xml_loader.h"

#include <cassert>
#include <filesystem>
#include <iostream>
#include <optional>

using trdp::config::loadSimulatorConfigFromXml;
using trdp::model::Dataset;
using trdp::model::InterfaceConfig;
using trdp::model::SimulatorConfig;

namespace
{
std::filesystem::path exampleXmlPath()
{
    auto path = std::filesystem::path(__FILE__).parent_path();
    return path / ".." / "external" / "TCNopen" / "trdp" / "example" / "example.xml";
}

std::optional<Dataset> findDataset(const SimulatorConfig &config, std::uint32_t id)
{
    for (const auto &dataset : config.datasets)
    {
        if (dataset.id == id)
        {
            return dataset;
        }
    }
    return std::nullopt;
}

std::optional<InterfaceConfig> findInterface(const SimulatorConfig &config, const std::string &name)
{
    for (const auto &iface : config.interfaces)
    {
        if (iface.name == name)
        {
            return iface;
        }
    }
    return std::nullopt;
}
} // namespace

int main()
{
    const auto xmlPath = exampleXmlPath();
    const auto result = loadSimulatorConfigFromXml(xmlPath.string());

    if (result.hasErrors())
    {
        std::cerr << "Unexpected XML load errors:\n";
        for (const auto &err : result.errors)
        {
            std::cerr << "  - " << err << '\n';
        }
        return 1;
    }

    assert(!result.config.datasets.empty());
    assert(!result.config.interfaces.empty());

    const auto dataset = findDataset(result.config, 1000);
    if (!dataset.has_value())
    {
        std::cerr << "Expected dataset 1000 to be parsed" << std::endl;
        return 1;
    }
    if (dataset->elements.empty())
    {
        std::cerr << "Dataset 1000 should contain at least one element" << std::endl;
        return 1;
    }

    const auto iface = findInterface(result.config, "eth0");
    if (!iface.has_value())
    {
        std::cerr << "Expected interface eth0 to be parsed" << std::endl;
        return 1;
    }
    if (iface->telegrams.empty())
    {
        std::cerr << "Interface eth0 should include telegram definitions" << std::endl;
        return 1;
    }

    return 0;
}
