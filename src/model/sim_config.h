#pragma once

#include "model/dataset_model.h"

#include <cstdint>
#include <string>
#include <vector>

namespace trdp::model
{
struct TelegramEndpoint
{
    std::uint32_t id{0};
    std::string uriUser;
    std::string uriHost;
};

struct TelegramConfig
{
    std::uint32_t comId{0};
    std::uint32_t datasetId{0};
    std::uint32_t comParId{0};
    std::string exchangeType;
    bool createEndpoint{false};
    std::uint32_t serviceId{0};
    std::vector<TelegramEndpoint> destinations;
    std::vector<TelegramEndpoint> sources;
};

struct InterfaceConfig
{
    std::string name;
    std::uint8_t networkId{0U};
    std::string hostIp;
    std::string leaderIp;
    std::vector<TelegramConfig> telegrams;
};

struct SimulatorConfig
{
    std::vector<InterfaceConfig> interfaces;
    std::vector<Dataset> datasets;
    std::vector<ComIdDatasetMapping> comIdDatasetMappings;
};
} // namespace trdp::model
