#include "config/xml_loader.h"

#include <cstdint>
#include <sstream>

#include <tau_xml.h>

namespace trdp::config
{
namespace
{
std::string toIpString(std::uint32_t ip)
{
    std::ostringstream ss;
    ss << ((ip >> 24) & 0xFF) << '.' << ((ip >> 16) & 0xFF) << '.' << ((ip >> 8) & 0xFF) << '.' << (ip & 0xFF);
    return ss.str();
}

std::string exchangeTypeToString(TRDP_EXCHG_OPTION_T type)
{
    switch (type)
    {
    case TRDP_EXCHG_SOURCE:
        return "source";
    case TRDP_EXCHG_SINK:
        return "sink";
    case TRDP_EXCHG_SOURCESINK:
        return "source+sink";
    case TRDP_EXCHG_UNSET:
    default:
        return "unset";
    }
}

std::string makeErrorMessage(const std::string &context, TRDP_ERR_T error)
{
    std::ostringstream ss;
    ss << context << " (error " << static_cast<int>(error) << ")";
    return ss.str();
}

std::string uriUserToString(const TRDP_URI_USER_T *uri)
{
    if (uri == nullptr)
    {
        return {};
    }
    return std::string(*uri);
}

std::string uriHostToString(const TRDP_URI_HOST_T *uri)
{
    if (uri == nullptr)
    {
        return {};
    }
    return std::string(*uri);
}

model::TelegramEndpoint convertDest(const TRDP_DEST_T &dest)
{
    model::TelegramEndpoint endpoint{};
    endpoint.id = dest.id;
    endpoint.uriUser = uriUserToString(dest.pUriUser);
    endpoint.uriHost = uriHostToString(dest.pUriHost);
    return endpoint;
}

model::TelegramEndpoint convertSrc(const TRDP_SRC_T &src)
{
    model::TelegramEndpoint endpoint{};
    endpoint.id = src.id;
    endpoint.uriUser = uriUserToString(src.pUriUser);
    endpoint.uriHost = uriHostToString(src.pUriHost1);
    return endpoint;
}

model::TelegramConfig convertTelegram(const TRDP_EXCHG_PAR_T &telegram)
{
    model::TelegramConfig cfg{};
    cfg.comId = telegram.comId;
    cfg.datasetId = telegram.datasetId;
    cfg.comParId = telegram.comParId;
    cfg.exchangeType = exchangeTypeToString(telegram.type);
    cfg.createEndpoint = telegram.create != 0;
    cfg.serviceId = telegram.serviceId;

    for (std::uint32_t i = 0; i < telegram.destCnt; ++i)
    {
        cfg.destinations.push_back(convertDest(telegram.pDest[i]));
    }
    for (std::uint32_t i = 0; i < telegram.srcCnt; ++i)
    {
        cfg.sources.push_back(convertSrc(telegram.pSrc[i]));
    }
    return cfg;
}

model::Dataset convertDataset(const TRDP_DATASET_T &dataset)
{
    model::Dataset result{};
    result.id = dataset.id;
    result.name = dataset.name;
    for (std::uint32_t i = 0; i < dataset.numElement; ++i)
    {
        const auto &element = dataset.pElement[i];
        model::DatasetElement converted{};
        converted.name = element.name != nullptr ? element.name : "";
        converted.arraySize = element.size;
        converted.type = std::to_string(element.type);
        result.elements.push_back(converted);
    }
    return result;
}
}

SimulatorConfigLoadResult loadSimulatorConfigFromXml(const std::string &path)
{
    SimulatorConfigLoadResult result{};

    TRDP_XML_DOC_HANDLE_T docHandle{};
    const auto loadErr = tau_prepareXmlDoc(path.c_str(), &docHandle);
    if (loadErr != TRDP_NO_ERR)
    {
        result.errors.emplace_back(makeErrorMessage("Unable to parse XML file: " + path, loadErr));
        return result;
    }

    TRDP_MEM_CONFIG_T memConfig{};
    TRDP_DBG_CONFIG_T dbgConfig{};
    UINT32 numComPar = 0U;
    TRDP_COM_PAR_T *pComPar = nullptr;
    UINT32 numIfConfig = 0U;
    TRDP_IF_CONFIG_T *pIfConfig = nullptr;

    const auto deviceErr = tau_readXmlDeviceConfig(&docHandle, &memConfig, &dbgConfig, &numComPar, &pComPar, &numIfConfig, &pIfConfig);
    if (deviceErr != TRDP_NO_ERR)
    {
        result.errors.emplace_back(makeErrorMessage("Device configuration missing or invalid", deviceErr));
    }

    UINT32 numComId = 0U;
    TRDP_COMID_DSID_MAP_T *pComIdDsIdMap = nullptr;
    UINT32 numDataset = 0U;
    TRDP_DATASET_T **pDatasets = nullptr;
    const auto datasetErr = tau_readXmlDatasetConfig(&docHandle, &numComId, &pComIdDsIdMap, &numDataset, &pDatasets);
    if (datasetErr == TRDP_NO_ERR)
    {
        for (std::uint32_t i = 0; i < numComId; ++i)
        {
            model::ComIdDatasetMapping mapping{};
            mapping.comId = pComIdDsIdMap[i].comId;
            mapping.datasetId = pComIdDsIdMap[i].datasetId;
            result.config.comIdDatasetMappings.push_back(mapping);
        }

        for (std::uint32_t i = 0; i < numDataset; ++i)
        {
            if (pDatasets != nullptr && pDatasets[i] != nullptr)
            {
                result.config.datasets.push_back(convertDataset(*pDatasets[i]));
            }
        }
    }
    else
    {
        result.errors.emplace_back(makeErrorMessage("Dataset definitions missing or invalid", datasetErr));
    }

    for (std::uint32_t i = 0; i < numIfConfig; ++i)
    {
        const auto &iface = pIfConfig[i];
        model::InterfaceConfig interfaceCfg{};
        interfaceCfg.name = iface.ifName;
        interfaceCfg.networkId = iface.networkId;
        interfaceCfg.hostIp = toIpString(iface.hostIp);
        interfaceCfg.leaderIp = toIpString(iface.leaderIp);

        TRDP_PROCESS_CONFIG_T processConfig{};
        TRDP_PD_CONFIG_T pdConfig{};
        TRDP_MD_CONFIG_T mdConfig{};
        TRDP_EXCHG_PAR_T *pExchgPar = nullptr;
        UINT32 numExchgPar = 0U;
        const auto ifErr = tau_readXmlInterfaceConfig(
            &docHandle,
            iface.ifName,
            &processConfig,
            &pdConfig,
            &mdConfig,
            &numExchgPar,
            &pExchgPar);
        if (ifErr == TRDP_NO_ERR)
        {
            for (std::uint32_t t = 0; t < numExchgPar; ++t)
            {
                interfaceCfg.telegrams.push_back(convertTelegram(pExchgPar[t]));
            }
        }
        else
        {
            result.errors.emplace_back(makeErrorMessage(std::string("Telegrams missing or invalid for interface ") + iface.ifName, ifErr));
        }

        tau_freeTelegrams(numExchgPar, pExchgPar);
        result.config.interfaces.push_back(interfaceCfg);
    }

    if (datasetErr == TRDP_NO_ERR)
    {
        tau_freeXmlDatasetConfig(numComId, pComIdDsIdMap, numDataset, pDatasets);
    }
    tau_freeXmlDoc(&docHandle);
    return result;
}
} // namespace trdp::config
