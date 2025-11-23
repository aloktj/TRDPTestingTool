# TCNOpen TRDP – `tau_xml.h` API Reference

This document describes the public API exposed by **`tau_xml.h`** from the TCNOpen TRDP stack, as used by the TRDP Simulator.

It is intended as a quick reference for Codex and for developers implementing XML‑driven configuration of TRDP sessions, interfaces, telegrams and datasets.

---

## 1. Header and Dependencies

To use the XML utility API, include:

```c
#include "tau_xml.h"
```

This in turn depends on:

```c
#include "vos_types.h"
#include "trdp_types.h"
```

Link against the TRDP/VOS libraries as required by your build system.

All functions return a `TRDP_ERR_T` (defined in `trdp_types.h`) unless explicitly documented otherwise.

---

## 2. Core Types Defined in `tau_xml.h`

These types are produced/consumed by the XML functions and are important for configuring the simulator.

### 2.1 Telegram Exchange Direction

```c
typedef enum
{
    TRDP_EXCHG_UNSET        = 0,
    TRDP_EXCHG_SOURCE       = 1,
    TRDP_EXCHG_SINK         = 2,
    TRDP_EXCHG_SOURCESINK   = 3
} TRDP_EXCHG_OPTION_T;
```

- **Purpose**: Indicates whether a telegram defined in XML shall be sent, received or both.
- `TRDP_EXCHG_SOURCE`: publish PD / act as source.
- `TRDP_EXCHG_SINK`: subscribe PD / act as sink.
- `TRDP_EXCHG_SOURCESINK`: publish and subscribe for same ComID.

---

### 2.2 SDT Parameters

```c
typedef struct
{
    UINT32  smi1;
    UINT32  smi2;
    UINT32  cmThr;
    UINT16  udv;
    UINT16  rxPeriod;
    UINT16  txPeriod;
    UINT16  nGuard;
    UINT8   nrxSafe;
    UINT8   reserved1;
    UINT16  lmiMax;
} TRDP_SDT_PAR_T;
```

- **Purpose**: Safety (SDT) configuration for a PD connection as parsed from XML `<sdt-parameter>`.


### 2.3 PD Parameter Block

```c
typedef struct
{
    UINT32              cycle;
    UINT32              redundant;
    UINT32              timeout;
    TRDP_TO_BEHAVIOR_T  toBehav;
    TRDP_FLAGS_T        flags;
    UINT16              offset;
} TRDP_PD_PAR_T;
```

- **Purpose**: PD‑specific configuration for a telegram (cycle time, redundancy group, timeout behavior, etc.).
- Usually corresponds to `<pd-parameter>` attributes in XML.


### 2.4 MD Parameter Block

```c
typedef struct
{
    UINT32          confirmTimeout;
    UINT32          replyTimeout;
    TRDP_FLAGS_T    flags;
} TRDP_MD_PAR_T;
```

- **Purpose**: MD‑specific parameters (timeouts and flags) parsed from XML for one telegram.


### 2.5 Destination Descriptor

```c
typedef struct
{
    UINT32          id;
    TRDP_SDT_PAR_T  *pSdtPar;
    TRDP_URI_USER_T *pUriUser;
    TRDP_URI_HOST_T *pUriHost;
} TRDP_DEST_T;
```

- **Purpose**: Describes one destination of a telegram (URI, safety parameters, identifier).


### 2.6 Source Descriptor

```c
typedef struct
{
    UINT32          id;
    TRDP_SDT_PAR_T  *pSdtPar;
    TRDP_URI_USER_T *pUriUser;
    TRDP_URI_HOST_T *pUriHost1;
    TRDP_URI_HOST_T *pUriHost2;
} TRDP_SRC_T;
```

- **Purpose**: Describes one source of a telegram (for PD source / MD sender), including redundancy URIs.


### 2.7 Exchange (Telegram) Parameters

```c
typedef struct
{
    UINT32              comId;
    UINT32              datasetId;
    UINT32              comParId;
    TRDP_MD_PAR_T       *pMdPar;
    TRDP_PD_PAR_T       *pPdPar;
    UINT32              destCnt;
    TRDP_DEST_T         *pDest;
    UINT32              srcCnt;
    TRDP_SRC_T          *pSrc;
    TRDP_EXCHG_OPTION_T type;
    BOOL8               create;
    UINT32              serviceId;
} TRDP_EXCHG_PAR_T;
```

- **Purpose**: Fully describes a logical telegram from XML:
  - which ComID, which Dataset ID,
  - which send parameters (`comParId` → `TRDP_COM_PAR_T`),
  - PD and MD settings,
  - list of sources and destinations,
  - whether to create publisher/subscriber/listener automatically (`create`).


### 2.8 Interface Configuration

```c
typedef struct
{
    TRDP_LABEL_T    ifName;
    UINT8           networkId;
    TRDP_IP_ADDR_T  hostIp;
    TRDP_IP_ADDR_T  leaderIp;
} TRDP_IF_CONFIG_T;
```

- **Purpose**: Basic interface configuration parsed from XML `<bus-interface>` and related attributes.


### 2.9 Communication Parameters (Com Parameter List)

```c
typedef struct
{
    UINT32              id;
    TRDP_SEND_PARAM_T   sendParam;
} TRDP_COM_PAR_T;
```

- **Purpose**: Links an ID to a `TRDP_SEND_PARAM_T` (QoS, TTL, VLAN, TSN, etc.) from `<com-parameter>` tags.


### 2.10 Service‑Oriented Types

These are used when your XML includes service definitions (events, fields, methods, instances, devices).

```c
typedef struct
{
    TRDP_URI_USER_T eventName;
    UINT32          comId;
    UINT16          eventId;
    BOOL8           usesPd;
} TRDP_EVENT_T;

typedef struct
{
    TRDP_URI_USER_T fieldName;
    UINT32          comId;
    UINT16          fieldId;
} TRDP_FIELD_T;

typedef struct
{
    TRDP_URI_USER_T methodName;
    UINT32          comId;
    UINT32          replyComId;
    UINT16          methodId;
    BOOL8           confirm;
} TRDP_METHOD_T;

typedef struct
{
    TRDP_URI_USER_T instanceName;
    TRDP_URI_T      dstUri;
    UINT8           instanceId;
} TRDP_INSTANCE_T;

typedef struct
{
    TRDP_URI_HOST_T     dstUri;
    TRDP_URI_HOST_T     hostUri;
    TRDP_URI_HOST_T     redUri;
    UINT32              instanceCnt;
    TRDP_INSTANCE_T     *pInstance;
} TRDP_SERVICE_DEVICE_T;

typedef struct
{
    UINT32  comId;
    UINT32  srcId;
    UINT32  dstId;
    UINT32  id;
} TRDP_TELEGRAM_REF_T;

typedef struct
{
    TRDP_URI_USER_T         serviceName;
    UINT32                  serviceId;
    UINT32                  serviceTTL;
    BOOL8                   dummyService;
    UINT32                  eventCnt;
    TRDP_EVENT_T            *pEvent;
    UINT32                  fieldCnt;
    TRDP_FIELD_T            *pField;
    UINT32                  methodCnt;
    TRDP_METHOD_T           *pMethod;
    UINT32                  deviceCnt;
    TRDP_SERVICE_DEVICE_T   *pDevice;
    UINT32                  telegramRefCnt;
    TRDP_TELEGRAM_REF_T     *pTelegramRef;
} TRDP_SERVICE_DEF_T;
```

- **Purpose**: Allow XML‑driven description of services, events, methods, and device mapping in a service‑oriented architecture (SOA).


### 2.11 Debug Configuration Types

```c
#define TRDP_DBG_DEFAULT    0
#define TRDP_DBG_OFF        0x01
#define TRDP_DBG_ERR        0x02
#define TRDP_DBG_WARN       0x04
#define TRDP_DBG_INFO       0x08
#define TRDP_DBG_DBG        0x10
#define TRDP_DBG_TIME       0x20
#define TRDP_DBG_LOC        0x40
#define TRDP_DBG_CAT        0x80

typedef UINT8 TRDP_DBG_OPTION_T;

typedef struct
{
    TRDP_DBG_OPTION_T   option;
    UINT32              maxFileSize;
    TRDP_FILE_NAME_T    fileName;
} TRDP_DBG_CONFIG_T;
```

- **Purpose**: Debug output configuration parsed from XML (debug options, log file name, max file size).


### 2.12 XML Document Handle

```c
struct XML_HANDLE;

typedef struct
{
    struct XML_HANDLE *pXmlDocument;
} TRDP_XML_DOC_HANDLE_T;
```

- **Purpose**: Opaque handle representing a parsed XML configuration document. Must be obtained by `tau_prepareXmlDoc()` or `tau_prepareXmlMem()` and later freed by `tau_freeXmlDoc()`.

---

## 3. XML Document Lifecycle API

### 3.1 `tau_prepareXmlDoc` – Load XML from File

```c
EXT_DECL TRDP_ERR_T tau_prepareXmlDoc(
    const CHAR8             *pFileName,
    TRDP_XML_DOC_HANDLE_T   *pDocHnd
);
```

- **Purpose**: Parse an XML configuration file from disk and prepare internal structures for XPath‑like access.
- **Parameters**:
  - `pFileName`: Path to the XML configuration file (null‑terminated C string).
  - `pDocHnd`: Output pointer receiving the initialized XML document handle.
- **Returns**:
  - `TRDP_NO_ERR` on success.
  - `TRDP_PARAM_ERR` if the file does not exist or parameters are invalid.
- **Usage**: Call once at startup when loading an XML config from a file path.


### 3.2 `tau_prepareXmlMem` – Load XML from Memory Buffer

```c
EXT_DECL TRDP_ERR_T tau_prepareXmlMem(
    const char              *pBuffer,
    size_t                  bufSize,
    TRDP_XML_DOC_HANDLE_T   *pDocHnd
);
```

- **Purpose**: Parse an XML configuration stored in a memory buffer (e.g. received via MD or read from another source).
- **Parameters**:
  - `pBuffer`: Pointer to in‑memory XML text.
  - `bufSize`: Size of the buffer in bytes.
  - `pDocHnd`: Output pointer receiving the initialized XML document handle.
- **Returns**:
  - `TRDP_NO_ERR` on success.
  - `TRDP_PARAM_ERR` on invalid buffer or parsing failure.
- **Usage**: Useful when the simulator reads configuration from a file system abstraction or network rather than a direct path.


### 3.3 `tau_freeXmlDoc` – Free XML Document Handle

```c
EXT_DECL void tau_freeXmlDoc(
    TRDP_XML_DOC_HANDLE_T *pDocHnd
);
```

- **Purpose**: Release all resources associated with a `TRDP_XML_DOC_HANDLE_T` previously created by `tau_prepareXmlDoc()` or `tau_prepareXmlMem()`.
- **Parameters**:
  - `pDocHnd`: Pointer to the document handle to free. The structure is invalid after this call.
- **Returns**: None.
- **Usage**: Call once the application is done using all XML‑derived data or wants to reload configuration.

---

## 4. Device‑Level Configuration API

These functions read high‑level device configuration (memory, debug settings, com‑parameter list, and interface list) from the prepared XML document.

### 4.1 `tau_readXmlDeviceConfig` – Device and Communication Parameters

```c
EXT_DECL TRDP_ERR_T tau_readXmlDeviceConfig(
    const TRDP_XML_DOC_HANDLE_T *pDocHnd,
    TRDP_MEM_CONFIG_T           *pMemConfig,
    TRDP_DBG_CONFIG_T           *pDbgConfig,
    UINT32                      *pNumComPar,
    TRDP_COM_PAR_T              **ppComPar,
    UINT32                      *pNumIfConfig,
    TRDP_IF_CONFIG_T            **ppIfConfig
);
```

- **Purpose**:
  - Read global device configuration:
    - TRDP memory configuration (`TRDP_MEM_CONFIG_T`),
    - debug output configuration (`TRDP_DBG_CONFIG_T`),
    - list of communication parameter sets (`TRDP_COM_PAR_T`),
    - list of interface configurations (`TRDP_IF_CONFIG_T`).
- **Parameters**:
  - `pDocHnd`: Prepared XML document handle.
  - `pMemConfig`: Output – memory configuration; typically fed into `tlc_init`.
  - `pDbgConfig`: Output – debug configuration for the application.
  - `pNumComPar`: Output – number of `TRDP_COM_PAR_T` entries.
  - `ppComPar`: Output – pointer to an array of `TRDP_COM_PAR_T` (allocated by the XML subsystem).
  - `pNumIfConfig`: Output – number of `TRDP_IF_CONFIG_T` entries.
  - `ppIfConfig`: Output – pointer to an array of `TRDP_IF_CONFIG_T` entries.
- **Returns**:
  - `TRDP_NO_ERR` on success.
  - `TRDP_MEM_ERR` if provided buffers or internal allocations are insufficient.
  - `TRDP_PARAM_ERR` for invalid parameters or missing data.
- **Memory Ownership**:
  - Memory for `*ppComPar` and `*ppIfConfig` is allocated by the XML subsystem; free using `vos_memFree()` (according to the TRDP/VOS documentation) when no longer needed.

---

## 5. Interface and Telegram Configuration API

These functions deal with the configuration bound to specific interfaces and telegrams.

### 5.1 `tau_readXmlInterfaceConfig` – Per‑Interface TRDP Setup

```c
EXT_DECL TRDP_ERR_T tau_readXmlInterfaceConfig(
    const TRDP_XML_DOC_HANDLE_T *pDocHnd,
    const CHAR8                 *pIfName,
    TRDP_PROCESS_CONFIG_T       *pProcessConfig,
    TRDP_PD_CONFIG_T            *pPdConfig,
    TRDP_MD_CONFIG_T            *pMdConfig,
    UINT32                      *pNumExchgPar,
    TRDP_EXCHG_PAR_T            **ppExchgPar
);
```

- **Purpose**:
  - Read interface‑specific TRDP configuration for a given interface name (e.g. `"eth0"`):
    - Process config (`TRDP_PROCESS_CONFIG_T`),
    - default PD config (`TRDP_PD_CONFIG_T`),
    - default MD config (`TRDP_MD_CONFIG_T`),
    - array of telegram exchange parameters (`TRDP_EXCHG_PAR_T`).
- **Parameters**:
  - `pDocHnd`: Prepared XML document handle.
  - `pIfName`: Interface name to query (must match XML `<bus-interface name="...">`).
  - `pProcessConfig`: Output – filled with TRDP process/session configuration for the interface.
  - `pPdConfig`: Output – default PD configuration for the interface.
  - `pMdConfig`: Output – default MD configuration for the interface.
  - `pNumExchgPar`: Output – number of telegram configurations in `*ppExchgPar`.
  - `ppExchgPar`: Output – pointer to array of `TRDP_EXCHG_PAR_T` entries.
- **Returns**:
  - `TRDP_NO_ERR` on success.
  - `TRDP_MEM_ERR` if buffers are too small or allocation fails.
  - `TRDP_PARAM_ERR` if XML is missing expected elements or parameters are invalid.
- **Memory Ownership**:
  - `*ppExchgPar` is allocated by XML subsystem; free using `tau_freeTelegrams()` when done.

---

## 6. Dataset Configuration API

### 6.1 `tau_readXmlDatasetConfig` – Read Dataset Definitions

```c
EXT_DECL TRDP_ERR_T tau_readXmlDatasetConfig(
    const TRDP_XML_DOC_HANDLE_T *pDocHnd,
    UINT32                      *pNumComId,
    TRDP_COMID_DSID_MAP_T       **ppComIdDsIdMap,
    UINT32                      *pNumDataset,
    papTRDP_DATASET_T           papDataset
);
```

- **Purpose**: Parse `<data-set-list>` section:
  - ComID → DatasetID mapping table,
  - array of dataset definitions (`TRDP_DATASET_T` structures).
- **Parameters**:
  - `pDocHnd`: Prepared XML document handle.
  - `pNumComId`: Output – number of entries in the ComID–DatasetID mapping list.
  - `ppComIdDsIdMap`: Output – pointer to array of `TRDP_COMID_DSID_MAP_T` entries.
  - `pNumDataset`: Output – number of datasets found.
  - `papDataset`: Output – pointer to an array of dataset pointers (`TRDP_DATASET_T *`).
- **Returns**:
  - `TRDP_NO_ERR` on success.
  - `TRDP_MEM_ERR` if buffers are too small or allocation fails.
  - `TRDP_PARAM_ERR` if XML is missing or invalid.
- **Memory Ownership**:
  - All allocated memory for ComID/DSID map and dataset descriptors must be released using `tau_freeXmlDatasetConfig()`.


### 6.2 `tau_freeXmlDatasetConfig` – Free Dataset Config Memory

```c
EXT_DECL void tau_freeXmlDatasetConfig(
    UINT32                  numComId,
    TRDP_COMID_DSID_MAP_T   *pComIdDsIdMap,
    UINT32                  numDataset,
    TRDP_DATASET_T          **ppDataset
);
```

- **Purpose**: Free memory previously allocated when parsing dataset configuration with `tau_readXmlDatasetConfig()`.
- **Parameters**:
  - `numComId`: Number of entries in `pComIdDsIdMap`.
  - `pComIdDsIdMap`: Pointer to ComID–DatasetID mapping table.
  - `numDataset`: Number of datasets.
  - `ppDataset`: Array of pointers to `TRDP_DATASET_T` instances.
- **Returns**: None.
- **Usage**: Call once you no longer need dataset definitions (e.g. at shutdown or when reloading XML).

---

## 7. Telegram Configuration Cleanup

### 7.1 `tau_freeTelegrams` – Free Telegram Array

```c
EXT_DECL void tau_freeTelegrams(
    UINT32              numExchgPar,
    TRDP_EXCHG_PAR_T    *pExchgPar
);
```

- **Purpose**: Free memory allocated for telegram exchange parameters by `tau_readXmlInterfaceConfig()` or `tau_readXmlMappedInterfaceConfig()`.
- **Parameters**:
  - `numExchgPar`: Number of telegram configurations in the array.
  - `pExchgPar`: Pointer to the array of `TRDP_EXCHG_PAR_T` entries.
- **Returns**: None.

---

## 8. Service Definition Configuration API

### 8.1 `tau_readXmlServiceConfig` – Read Service‑Oriented Definitions

```c
EXT_DECL TRDP_ERR_T tau_readXmlServiceConfig(
    const TRDP_XML_DOC_HANDLE_T *pDocHnd,
    UINT32                      *pNumServiceDefs,
    TRDP_SERVICE_DEF_T          **ppServiceDefs
);
```

- **Purpose**:
  - Parse service definitions from XML (services, events, fields, methods, and associated devices/instances).
- **Parameters**:
  - `pDocHnd`: Prepared XML document handle.
  - `pNumServiceDefs`: Output – number of defined services.
  - `ppServiceDefs`: Output – pointer to array of `TRDP_SERVICE_DEF_T` entries.
- **Returns**:
  - `TRDP_NO_ERR` on success.
  - `TRDP_MEM_ERR` if buffers are too small or allocation fails.
  - `TRDP_PARAM_ERR` if XML is missing or invalid.
- **Memory Ownership**:
  - Memory for `*ppServiceDefs` is allocated by XML subsystem. Free using `vos_memFree()` as per TRDP/VOS guidelines.

---

## 9. Mapped Device Configuration API

These functions are used for configurations involving **mapped devices**, i.e. logical devices whose telegrams are mapped onto this host/interface.

### 9.1 `tau_readXmlMappedDevices` – Read All Mapped Devices

```c
EXT_DECL TRDP_ERR_T tau_readXmlMappedDevices(
    const TRDP_XML_DOC_HANDLE_T *pDocHnd,
    UINT32                      *pNumProcConfig,
    TRDP_PROCESS_CONFIG_T       **ppProcessConfig
);
```

- **Purpose**:
  - Read all mapped device process configurations from the XML `<mapped-device-list>`.
- **Parameters**:
  - `pDocHnd`: Prepared XML document handle.
  - `pNumProcConfig`: Output – number of mapped device process configurations.
  - `ppProcessConfig`: Output – pointer to array of `TRDP_PROCESS_CONFIG_T` representing each mapped device.
- **Returns**:
  - `TRDP_NO_ERR` on success.
  - `TRDP_MEM_ERR` on allocation issues.
  - `TRDP_PARAM_ERR` on invalid parameters or missing configuration.
- **Memory Ownership**:
  - `*ppProcessConfig` must be freed using `vos_memFree()` when no longer required.

---

### 9.2 `tau_readXmlMappedDeviceConfig` – Mapped Device Interfaces for a Host

```c
EXT_DECL TRDP_ERR_T tau_readXmlMappedDeviceConfig(
    const TRDP_XML_DOC_HANDLE_T *pDocHnd,
    const CHAR8                 *pHostname,
    UINT32                      *pNumIfConfig,
    TRDP_IF_CONFIG_T            **ppIfConfig
);
```

- **Purpose**:
  - For a particular mapped device (identified by `host-name`), read its interface configurations.
- **Parameters**:
  - `pDocHnd`: Prepared XML document handle.
  - `pHostname`: Host name attribute as used in `<mapped-device host-name="...">`.
  - `pNumIfConfig`: Output – number of interface configurations.
  - `ppIfConfig`: Output – pointer to array of `TRDP_IF_CONFIG_T` for that mapped device.
- **Returns**:
  - `TRDP_NO_ERR` on success.
  - `TRDP_MEM_ERR` on allocation issues.
  - `TRDP_PARAM_ERR` on invalid parameters or missing configuration.
- **Memory Ownership**:
  - `*ppIfConfig` must be freed using `vos_memFree()` when no longer required.

---

### 9.3 `tau_readXmlMappedInterfaceConfig` – Mapped Telegrams for Host & Interface

```c
EXT_DECL TRDP_ERR_T tau_readXmlMappedInterfaceConfig(
    const TRDP_XML_DOC_HANDLE_T *pDocHnd,
    const CHAR8                 *pHostname,
    const CHAR8                 *pIfName,
    UINT32                      *pNumExchgPar,
    TRDP_EXCHG_PAR_T            **ppExchgPar
);
```

- **Purpose**:
  - Read the interface‑relevant mapped telegram parameters (ComIDs, PD/MD settings, etc.) for a given mapped device host and interface.
- **Parameters**:
  - `pDocHnd`: Prepared XML document handle.
  - `pHostname`: Mapped device host name.
  - `pIfName`: Interface name.
  - `pNumExchgPar`: Output – number of telegram configurations.
  - `ppExchgPar`: Output – pointer to array of `TRDP_EXCHG_PAR_T` entries.
- **Returns**:
  - `TRDP_NO_ERR` on success.
  - `TRDP_MEM_ERR` if allocation fails.
  - `TRDP_PARAM_ERR` on invalid parameters or missing configuration.
- **Memory Ownership**:
  - `*ppExchgPar` must be freed using `tau_freeTelegrams()` when no longer needed.

---

## 10. Typical Usage Pattern in a Simulator

For easy reference by Codex, a typical XML loading sequence is:

1. **Load XML**  
   ```c
   TRDP_XML_DOC_HANDLE_T xmlHandle;
   TRDP_ERR_T err = tau_prepareXmlDoc("config.xml", &xmlHandle);
   ```

2. **Read device‑level configuration**  
   ```c
   TRDP_MEM_CONFIG_T   memConfig;
   TRDP_DBG_CONFIG_T   dbgConfig;
   UINT32              numComPar;
   TRDP_COM_PAR_T     *pComPar;
   UINT32              numIfCfg;
   TRDP_IF_CONFIG_T   *pIfCfg;

   err = tau_readXmlDeviceConfig(&xmlHandle,
                                 &memConfig,
                                 &dbgConfig,
                                 &numComPar,
                                 &pComPar,
                                 &numIfCfg,
                                 &pIfCfg);
   ```

3. **Read datasets**  
   ```c
   UINT32                numComId;
   TRDP_COMID_DSID_MAP_T *pComIdDsIdMap;
   UINT32                numDataset;
   papTRDP_DATASET_T     pDatasets;

   err = tau_readXmlDatasetConfig(&xmlHandle,
                                  &numComId,
                                  &pComIdDsIdMap,
                                  &numDataset,
                                  pDatasets);
   ```

4. **Read interface configuration (per interface)**  
   ```c
   TRDP_PROCESS_CONFIG_T processCfg;
   TRDP_PD_CONFIG_T      pdCfg;
   TRDP_MD_CONFIG_T      mdCfg;
   UINT32                numExchg;
   TRDP_EXCHG_PAR_T     *pExchg;

   err = tau_readXmlInterfaceConfig(&xmlHandle,
                                    "eth0",
                                    &processCfg,
                                    &pdCfg,
                                    &mdCfg,
                                    &numExchg,
                                    &pExchg);
   ```

5. **Read mapped devices (optional)**  
   Use `tau_readXmlMappedDevices`, `tau_readXmlMappedDeviceConfig`, and `tau_readXmlMappedInterfaceConfig` as needed.

6. **Free allocated XML‑related structures on shutdown**  
   ```c
   tau_freeTelegrams(numExchg, pExchg);
   tau_freeXmlDatasetConfig(numComId, pComIdDsIdMap, numDataset, (TRDP_DATASET_T **)pDatasets);
   tau_freeXmlDoc(&xmlHandle);
   ```

This pattern should give Codex everything needed to correctly plug XML configuration into the TRDP Simulator architecture while staying consistent with the TCNOpen TRDP stack.
