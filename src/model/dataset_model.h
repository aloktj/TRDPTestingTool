#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace trdp::model
{
struct DatasetElement
{
    std::string name;
    std::string type;
    std::uint32_t arraySize{1};
};

struct Dataset
{
    std::uint32_t id{0};
    std::string name;
    std::vector<DatasetElement> elements;
};

struct ComIdDatasetMapping
{
    std::uint32_t comId{0};
    std::uint32_t datasetId{0};
};
} // namespace trdp::model
