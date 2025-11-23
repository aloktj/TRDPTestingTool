#pragma once

#include "model/sim_config.h"

#include <string>
#include <vector>

namespace trdp::config
{
struct SimulatorConfigLoadResult
{
    model::SimulatorConfig config;
    std::vector<std::string> errors;

    [[nodiscard]] bool hasErrors() const { return !errors.empty(); }
};

SimulatorConfigLoadResult loadSimulatorConfigFromXml(const std::string &path);
} // namespace trdp::config
