# TRDP API Documentation — tau_marshall.h

## Overview
`tau_marshall.h` provides functionality for TRDP **marshalling and unmarshalling** of Process Data (PD) and Message Data (MD) datasets.  
Marshalling converts structured dataset elements into network byte-order telegram payloads.  
Unmarshalling reverses that process, producing structured data usable by the application.

These APIs depend on dataset definitions parsed from the TRDP XML configuration and support dynamic dataset caching and size calculation.

---

## Included Header Files
| Header | Description |
|--------|-------------|
| `trdp_types.h` | Core TRDP type and structure definitions |

---

## Constants
| Define | Value | Description |
|--------|-------|-------------|
| `TAU_MAX_DS_LEVEL` | `5` | Max levels of dataset nesting |

---

## API Functions

---

### `tau_initMarshall()`
Initializes marshalling and unmarshalling subsystem using Dataset configuration.

```c
EXT_DECL TRDP_ERR_T tau_initMarshall(
    void * *ppRefCon,
    UINT32 numComId,
    TRDP_COMID_DSID_MAP_T  *pComIdDsIdMap,
    UINT32 numDataSet,
    TRDP_DATASET_T         *pDataset[]);
Parameter	Direction	Description
ppRefCon	OUT	returns reference pointer for marshalling context
numComId	IN	number of ComID–Dataset mapping entries
pComIdDsIdMap	IN	array of mapping structures
numDataSet	IN	number of dataset definitions
pDataset[]	IN	pointer to dataset structures

Returns	Meaning
TRDP_NO_ERR	success
TRDP_MEM_ERR	insufficient buffer
TRDP_PARAM_ERR	invalid arguments

tau_marshall()
Convert raw dataset payload into TRDP encoded format by ComId.

c
Copy code
EXT_DECL TRDP_ERR_T tau_marshall(
    void *pRefCon,
    UINT32 comId,
    const UINT8 *pSrc,
    UINT32 srcSize,
    UINT8 *pDest,
    UINT32 *pDestSize,
    TRDP_DATASET_T **ppDSPointer);
Parameter	Direction	Description
pRefCon	IN	context returned from init
comId	IN	ComID identifying dataset
pSrc	IN	buffer containing original structure
srcSize	IN	source buffer size
pDest	OUT	encoded TRDP payload
pDestSize	IN/OUT	destination buffer size / resulting size
ppDSPointer	IN/OUT	pointer to dataset cache entry

tau_marshallDs()
Marshalling by DatasetId.

c
Copy code
EXT_DECL TRDP_ERR_T tau_marshallDs(
    void *pRefCon,
    UINT32 dsId,
    UINT8 *pSrc,
    UINT32 srcSize,
    UINT8 *pDest,
    UINT32 *pDestSize,
    TRDP_DATASET_T **ppDSPointer);
tau_unmarshall()
Convert TRDP payload into structured data by ComId.

c
Copy code
EXT_DECL TRDP_ERR_T tau_unmarshall(
    void *pRefCon,
    UINT32 comId,
    UINT8 *pSrc,
    UINT32 srcSize,
    UINT8 *pDest,
    UINT32 *pDestSize,
    TRDP_DATASET_T **ppDSPointer);
tau_unmarshallDs()
Unmarshall based on Dataset ID.

c
Copy code
EXT_DECL TRDP_ERR_T tau_unmarshallDs(
    void *pRefCon,
    UINT32 dsId,
    UINT8 *pSrc,
    UINT32 srcSize,
    UINT8 *pDest,
    UINT32 *pDestSize,
    TRDP_DATASET_T **ppDSPointer);
tau_calcDatasetSize()
Calculate dataset size from DataSet ID and raw source buffer.

c
Copy code
EXT_DECL TRDP_ERR_T tau_calcDatasetSize(
    void *pRefCon,
    UINT32 dsId,
    UINT8 *pSrc,
    UINT32 srcSize,
    UINT32 *pDestSize,
    TRDP_DATASET_T **ppDSPointer);
tau_calcDatasetSizeByComId()
c
Copy code
EXT_DECL TRDP_ERR_T tau_calcDatasetSizeByComId(
    void *pRefCon,
    UINT32 comId,
    UINT8 *pSrc,
    UINT32 srcSize,
    UINT32 *pDestSize,
    TRDP_DATASET_T **ppDSPointer);
| Purpose | Determine final message payload size for PD/MD |
| Useful for | Memory allocation, validation |

Return Error Codes
Code	Meaning
TRDP_NO_ERR	success
TRDP_MEM_ERR	insufficient destination buffer
TRDP_COMID_ERR	unknown ComId/dsId
TRDP_INIT_ERR	marshalling not initialized
TRDP_PARAM_ERR	invalid input parameters

Typical Usage Flow
c
Copy code
TRDP_DATASET_T *datasetList[MAX_DATASETS];
void *marshallContext = NULL;

tau_initMarshall(&marshallContext, numMap, mapList, numDs, datasetList);

// Marshall outgoing message
UINT32 destSize = 1024;
tau_marshall(marshallContext, comId, srcBuffer, srcSize, destBuffer, &destSize, NULL);

// Unmarshall incoming telegram
UINT32 structSize = sizeof(MyStructure);
tau_unmarshall(marshallContext, comId, rxBuffer, rxSize, structOut, &structSize, NULL);
Notes
Caller must allocate destination buffers.

Can cache datasets for fast repeated operations.

Required for TRDP PD and MD application processing.
