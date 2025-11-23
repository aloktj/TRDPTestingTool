````markdown
# TRDP Utility API — Train Topology Interface (`tau_tti.h`)

## 1. Purpose

The header `tau_tti.h` provides the **Train Topology Information (TTI) access API** for TRDP.  
It encapsulates access to the **train topology / directory / consist / vehicle information** via the TTI subsystem, using information typically supplied by an ECSP / TTDB over TRDP.

This interface is used **in addition to** the standard TRDP light API (`trdp_if_light.h`) and relies on:

- `trdp_types.h` — TRDP base types and error codes  
- `tau_tti_types.h` — TTI-specific structures (train directory, consist info, vehicle info, etc.)

All functions return **`TRDP_ERR_T`** unless explicitly stated otherwise.

---

## 2. Included Headers and Dependencies

```c
#include "trdp_types.h"
#include "tau_tti_types.h"
````

Key dependent types (defined in `tau_tti_types.h`):

* `TRDP_OP_TRAIN_DIR_STATE_T`
* `TRDP_OP_TRAIN_DIR_T`
* `TRDP_OP_TRAIN_DIR_STATUS_INFO_T`
* `TRDP_TRAIN_DIR_T`
* `TRDP_TRAIN_NET_DIR_T`
* `TRDP_CONSIST_INFO_T`
* `TRDP_VEHICLE_INFO_T`
* `TRDP_FUNCTION_INFO_T`

From `trdp_types.h`:

* `TRDP_APP_SESSION_T` — TRDP session handle
* `TRDP_LABEL_T` — label type (device/vehicle/consist)
* `TRDP_IP_ADDR_T` — IP address type
* `TRDP_UUID_T` — UUID type
* `TRDP_ERR_T` — error codes
* `VOS_SEMA_T` — semaphore type (from VOS)

---

## 3. Initialization / Termination

### 3.1 `tau_initTTIaccess`

**Prototype**

```c
EXT_DECL TRDP_ERR_T tau_initTTIaccess(
    TRDP_APP_SESSION_T  appHandle,
    VOS_SEMA_T          userAction,
    TRDP_IP_ADDR_T      ecspIpAddr,
    CHAR8               *hostsFileName);
```

**Description**

Initialises the TTI subsystem for a given TRDP application session.
It sets up subscriptions (e.g. PD 100) and internal state to access train topology information.

**Parameters**

* `appHandle`
  TRDP application session handle returned by `tlc_openSession()`.

* `userAction`
  VOS semaphore that will be signalled when **inauguration has taken place** (i.e. topology data becomes valid).

* `ecspIpAddr`
  IP address of the ECSP/TTDB server providing train topology information.

* `hostsFileName`
  Optional pointer to host file name used as ECSP replacement.
  May be `NULL` if not used.

**Return**

* `TRDP_NO_ERR` — initialisation successful
* `TRDP_INIT_ERR` — initialisation error

---

### 3.2 `tau_deInitTTI`

**Prototype**

```c
EXT_DECL void tau_deInitTTI(
    TRDP_APP_SESSION_T appHandle);
```

**Description**

Terminates and cleans up the TTI subsystem for the given application session.

**Parameters**

* `appHandle`
  TRDP application session handle returned by `tlc_openSession()`.

**Return**

* None (void).

---

## 4. Train and Train Directory Access

### 4.1 `tau_getOpTrnDirectory`

**Prototype**

```c
EXT_DECL TRDP_ERR_T tau_getOpTrnDirectory(
    TRDP_APP_SESSION_T          appHandle,
    TRDP_OP_TRAIN_DIR_STATE_T  *pOpTrnDirState,
    TRDP_OP_TRAIN_DIR_T        *pOpTrnDir);
```

**Description**

Retrieves the **operational train directory state** and the corresponding **operational train directory**.

**Parameters**

* `appHandle`
  TRDP application session handle.

* `pOpTrnDirState`
  Pointer to an `TRDP_OP_TRAIN_DIR_STATE_T` structure where the current state will be stored.

* `pOpTrnDir`
  Pointer to an `TRDP_OP_TRAIN_DIR_T` structure where the operational directory will be stored.

**Return**

* `TRDP_NO_ERR` — data successfully retrieved
* `TRDP_PARAM_ERR` — parameter error (e.g. null pointer)

---

### 4.2 `tau_getOpTrnDirectoryStatusInfo`

**Prototype**

```c
EXT_DECL TRDP_ERR_T tau_getOpTrnDirectoryStatusInfo(
    TRDP_APP_SESSION_T              appHandle,
    TRDP_OP_TRAIN_DIR_STATUS_INFO_T *pOpTrnDirStatusInfo);
```

**Description**

Returns a copy of the **last received PD 100 telegram**, containing **operational train directory state information**.

> Note: Values are already in **host endianness**. When validating SDTv2, caller must convert to network order if needed.

**Parameters**

* `appHandle`
  TRDP application session handle.

* `pOpTrnDirStatusInfo`
  Pointer to a `TRDP_OP_TRAIN_DIR_STATUS_INFO_T` structure to be filled with the latest status info.

**Return**

* `TRDP_NO_ERR` — info retrieved
* `TRDP_PARAM_ERR` — parameter error

---

### 4.3 `tau_getTrnDirectory`

**Prototype**

```c
EXT_DECL TRDP_ERR_T tau_getTrnDirectory(
    TRDP_APP_SESSION_T  appHandle,
    TRDP_TRAIN_DIR_T    *pTrnDir);
```

**Description**

Retrieves the **static train directory (TRAIN_DIRECTORY)**.
If data is not yet available, the function indicates that the caller should **try again later**.

**Parameters**

* `appHandle`
  TRDP application session handle.

* `pTrnDir`
  Pointer to a `TRDP_TRAIN_DIR_T` structure to be filled with the train directory.

**Return**

* `TRDP_NO_ERR` — directory available and copied
* `TRDP_PARAM_ERR` — parameter error
* `TRDP_NODATA_ERR` — no directory available yet (try again later)

---

### 4.4 `tau_getTTI`

**Prototype**

```c
EXT_DECL TRDP_ERR_T tau_getTTI(
    TRDP_APP_SESSION_T          appHandle,
    TRDP_OP_TRAIN_DIR_STATE_T  *pOpTrnDirState,
    TRDP_OP_TRAIN_DIR_T        *pOpTrnDir,
    TRDP_TRAIN_DIR_T           *pTrnDir,
    TRDP_TRAIN_NET_DIR_T       *pTrnNetDir);
```

**Description**

Convenience function to retrieve **all relevant TTI structures** in a single call:

* Operational train directory state
* Operational train directory
* Static train directory
* Train network directory

**Parameters**

* `appHandle`         — TRDP session handle
* `pOpTrnDirState`    — pointer to state structure
* `pOpTrnDir`         — pointer to operational train directory
* `pTrnDir`           — pointer to static train directory
* `pTrnNetDir`        — pointer to train network directory

All output pointers must be valid non-`NULL` pointers.

**Return**

* `TRDP_NO_ERR` — structures filled successfully
* `TRDP_PARAM_ERR` — any invalid or `NULL` pointer passed

---

## 5. Consist-Level Queries

### 5.1 `tau_getStaticCstInfo`

**Prototype**

```c
EXT_DECL TRDP_ERR_T tau_getStaticCstInfo(
    TRDP_APP_SESSION_T   appHandle,
    TRDP_CONSIST_INFO_T **ppCstInfo,
    TRDP_UUID_T          const cstUUID);
```

**Description**

Allocates memory and retrieves **static consist information** for a specific consist, identified by its **UUID**.

The pointer at `*ppCstInfo` must later be freed using `vos_memFree()` by the caller.

**Parameters**

* `appHandle`
  TRDP application session handle.

* `ppCstInfo`
  Pointer to a pointer that will receive the allocated `TRDP_CONSIST_INFO_T` buffer.

* `cstUUID`
  UUID of the consist whose info is requested.

**Return**

* `TRDP_NO_ERR` — information retrieved and memory allocated
* `TRDP_PARAM_ERR` — parameter error

---

### 5.2 `tau_getCstInfo`

**Prototype**

```c
EXT_DECL TRDP_ERR_T tau_getCstInfo(
    TRDP_APP_SESSION_T    appHandle,
    TRDP_CONSIST_INFO_T **ppCstInfo,
    const TRDP_LABEL_T    pCstLabel);
```

**Description**

Allocates memory and retrieves consist information for a train consist identified by its **label**.

Memory must be freed by the caller via `vos_memFree()`.

**Parameters**

* `appHandle`
  TRDP application session handle.

* `ppCstInfo`
  Pointer to pointer receiving a newly allocated `TRDP_CONSIST_INFO_T` structure.

* `pCstLabel`
  Consist label (string).
  The label identifies which consist’s information is requested.

**Return**

* `TRDP_NO_ERR` — information retrieved and allocated
* `TRDP_PARAM_ERR` — parameter error

---

### 5.3 `tau_getTrnCstCnt`

**Prototype**

```c
EXT_DECL TRDP_ERR_T tau_getTrnCstCnt(
    TRDP_APP_SESSION_T   appHandle,
    UINT16              *pTrnCstCnt);
```

**Description**

Returns the **total number of consists** in the train.

**Parameters**

* `appHandle`
  TRDP application session handle.

* `pTrnCstCnt`
  Pointer to `UINT16` where the consist count is stored.

**Return**

* `TRDP_NO_ERR` — value successfully returned
* `TRDP_PARAM_ERR` — parameter error

---

### 5.4 `tau_getCstVehCnt`

**Prototype**

```c
EXT_DECL TRDP_ERR_T tau_getCstVehCnt(
    TRDP_APP_SESSION_T   appHandle,
    UINT16              *pCstVehCnt,
    const TRDP_LABEL_T   pCstLabel);
```

**Description**

Returns the **number of vehicles in a given consist**.

If `pCstLabel == NULL`, the **own consist** is used.

**Parameters**

* `appHandle`
  TRDP application session handle.

* `pCstVehCnt`
  Pointer to `UINT16` where number of vehicles is returned.

* `pCstLabel`
  Consist label, or `NULL` to reference own consist.

**Return**

* `TRDP_NO_ERR` — value returned successfully
* `TRDP_PARAM_ERR` — parameter error

---

### 5.5 `tau_getCstFctCnt`

**Prototype**

```c
EXT_DECL TRDP_ERR_T tau_getCstFctCnt(
    TRDP_APP_SESSION_T   appHandle,
    UINT16              *pCstFctCnt,
    const TRDP_LABEL_T   pCstLabel);
```

**Description**

Returns the **number of functions** (logical functions / function devices) in a given consist.

If `pCstLabel == NULL`, the **own consist** is used.

**Parameters**

* `appHandle`
  TRDP application session handle.

* `pCstFctCnt`
  Pointer to `UINT16` receiving function count.

* `pCstLabel`
  Consist label, or `NULL` for own consist.

**Return**

* `TRDP_NO_ERR` — value returned
* `TRDP_PARAM_ERR` — parameter error

---

### 5.6 `tau_getCstFctInfo`

**Prototype**

```c
EXT_DECL TRDP_ERR_T tau_getCstFctInfo(
    TRDP_APP_SESSION_T      appHandle,
    TRDP_FUNCTION_INFO_T   *pFctInfo,
    const TRDP_LABEL_T      pCstLabel,
    UINT16                  maxFctCnt);
```

**Description**

Retrieves the **function information list** for the specified consist into a caller-provided buffer.

**Parameters**

* `appHandle`
  TRDP application session handle.

* `pFctInfo`
  Pointer to array of `TRDP_FUNCTION_INFO_T` with capacity `maxFctCnt`.
  May be `NULL` if function info is not needed.

* `pCstLabel`
  Consist label; `NULL` means own consist.

* `maxFctCnt`
  Maximum number of entries that `pFctInfo` can hold.

**Return**

* `TRDP_NO_ERR` — information filled (up to `maxFctCnt` entries)
* `TRDP_PARAM_ERR` — invalid parameters

---

## 6. Vehicle-Level Queries

### 6.1 `tau_getTrnVehCnt`

**Prototype**

```c
EXT_DECL TRDP_ERR_T tau_getTrnVehCnt(
    TRDP_APP_SESSION_T   appHandle,
    UINT16              *pTrnVehCnt);
```

**Description**

Returns the **total number of vehicles** in the train.

**Parameters**

* `appHandle`
  TRDP application session handle.

* `pTrnVehCnt`
  Pointer to `UINT16` where vehicle count is stored.

**Return**

* `TRDP_NO_ERR` — count retrieved
* `TRDP_PARAM_ERR` — parameter error

---

### 6.2 `tau_getVehInfo`

**Prototype**

```c
EXT_DECL TRDP_ERR_T tau_getVehInfo(
    TRDP_APP_SESSION_T    appHandle,
    TRDP_VEHICLE_INFO_T **ppVehInfo,
    const TRDP_LABEL_T    pVehLabel,
    const TRDP_LABEL_T    pCstLabel);
```

**Description**

Allocates memory and returns **vehicle information** for the specified vehicle in a consist.

* If `pVehLabel == NULL` and `pCstLabel` refers to own consist, **own vehicle** is assumed.
* Memory pointed to by `*ppVehInfo` must be freed by caller via `vos_memFree()`.

**Parameters**

* `appHandle`
  TRDP application session handle.

* `ppVehInfo`
  Pointer to pointer that receives a newly allocated `TRDP_VEHICLE_INFO_T`.

* `pVehLabel`
  Vehicle label, or `NULL` to use own vehicle (if `pCstLabel` is own consist).

* `pCstLabel`
  Consist label, or `NULL` for own consist.

**Return**

* `TRDP_NO_ERR` — info retrieved and allocated
* `TRDP_PARAM_ERR` — parameter error

---

### 6.3 `tau_getVehOrient`

**Prototype**

```c
EXT_DECL TRDP_ERR_T tau_getVehOrient(
    TRDP_APP_SESSION_T   appHandle,
    UINT8               *pVehOrient,
    UINT8               *pCstOrient,
    TRDP_LABEL_T         pVehLabel,
    TRDP_LABEL_T         pCstLabel);
```

**Description**

Retrieves **vehicle and consist orientation** relative to the **operational train direction**.

Orientation values:

* `'00'B` — not known
* `'01'B` — same as operational train direction
* `'10'B` — inverse to operational train direction

Label semantics:

* `pVehLabel == NULL` ⇒ own vehicle if `pCstLabel == NULL`
* `pCstLabel == NULL` ⇒ own consist

**Parameters**

* `appHandle`
  TRDP application session handle.

* `pVehOrient`
  Pointer to `UINT8` to receive vehicle orientation.

* `pCstOrient`
  Pointer to `UINT8` to receive consist orientation.

* `pVehLabel`
  Vehicle label (may be `NULL` as described above).

* `pCstLabel`
  Consist label (may be `NULL` as described above).

**Return**

* `TRDP_NO_ERR` — orientation data returned
* `TRDP_PARAM_ERR` — parameter error

---

## 7. Own Identity Helpers

### 7.1 `tau_getOwnIds`

**Prototype**

```c
EXT_DECL TRDP_ERR_T tau_getOwnIds(
    TRDP_APP_SESSION_T  appHandle,
    TRDP_LABEL_T        *pDevId,
    TRDP_LABEL_T        *pVehId,
    TRDP_LABEL_T        *pCstId);
```

**Description**

Returns the **own device, vehicle, and consist identifiers** (labels).
Implements a "Who am I?" concept for the TRDP node.

If information is not yet available, the function may return `TRDP_NODATA_ERR` and should be called again later.

**Parameters**

* `appHandle`
  TRDP application session handle.

* `pDevId`
  Pointer to `TRDP_LABEL_T` returning the device label (host name).

* `pVehId`
  Pointer to `TRDP_LABEL_T` returning the vehicle label.

* `pCstId`
  Pointer to `TRDP_LABEL_T` returning the consist label.

**Return**

* `TRDP_NO_ERR` — IDs successfully returned
* `TRDP_PARAM_ERR` — parameter error
* `TRDP_NODATA_ERR` — data not yet available (retry later)

---

### 7.2 `tau_getOwnOpCstNo`

**Prototype**

```c
EXT_DECL UINT8 tau_getOwnOpCstNo(
    TRDP_APP_SESSION_T appHandle);
```

**Description**

Returns the **own operational consist number**.

**Parameters**

* `appHandle`
  TRDP application session handle.

**Return**

* `> 0` — own operational consist number
* `0`   — on error

---

### 7.3 `tau_getOwnTrnCstNo`

**Prototype**

```c
EXT_DECL UINT8 tau_getOwnTrnCstNo(
    TRDP_APP_SESSION_T appHandle);
```

**Description**

Returns the **own train consist number** (index into the train directory / train network directory).

**Parameters**

* `appHandle`
  TRDP application session handle.

**Return**

* `> 0` — train consist number
* `0`   — on error

---

## 8. Typical Usage Flow (High-Level)

1. **Start TRDP session** via `tlc_init()` and `tlc_openSession()`.
2. **Initialize TTI** with `tau_initTTIaccess()`:

   * Provide ECSP IP address and optional hosts file.
   * Wait for `userAction` semaphore to be signalled.
3. After inauguration:

   * Use `tau_getTTI()` or `tau_getOpTrnDirectory()` + `tau_getTrnDirectory()` to load all topology data.
   * Use `tau_getOwnIds()`, `tau_getOwnTrnCstNo()`, `tau_getOwnOpCstNo()` to determine own location in the train.
   * Use `tau_getCstInfo()`, `tau_getVehInfo()`, `tau_getVehOrient()` for detailed topology queries.
4. Before shutdown of the TRDP session:

   * Call `tau_deInitTTI()` to release all TTI-specific resources.

---

```
```
