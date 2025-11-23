
# TRDP Light API Reference – `trdp_if_light.h`

This document summarizes the **public API** exposed by the TCNOpen TRDP Light interface header:

```c
#include "trdp_if_light.h"
```

All fundamental types (e.g. `TRDP_ERR_T`, `TRDP_APP_SESSION_T`, `TRDP_IP_ADDR_T`) are defined in:

```c
#include "trdp_types.h"
```

Return codes are of type `TRDP_ERR_T`. Unless stated otherwise:

- `TRDP_NO_ERR` means success.
- Any negative value indicates an error (see `trdp_types.h` for full list).

---

## 1. Session Management (`tlc_*`)

### 1.1 Library Initialization & Termination

#### `tlc_init`

```c
EXT_DECL TRDP_ERR_T tlc_init(
    const TRDP_PRINT_DBG_T  pPrintDebugString,
    void                    *pRefCon,
    const TRDP_MEM_CONFIG_T *pMemConfig);
```

Initializes the TRDP library (VOS, memory management, debug callback). Must be called **once** before any other TRDP API.

- `pPrintDebugString` – Pointer to debug print callback (may be `NULL` to use default or none).
- `pRefCon` – User reference passed back to debug callback.
- `pMemConfig` – Memory configuration (static or heap based).

Returns `TRDP_NO_ERR` on success.

---

#### `tlc_terminate`

```c
EXT_DECL TRDP_ERR_T tlc_terminate(void);
```

Terminates the TRDP library and releases global resources.  
All sessions should be closed before calling this.

Returns `TRDP_NO_ERR` on success.

---

### 1.2 Session Lifecycle

#### `tlc_openSession`

```c
EXT_DECL TRDP_ERR_T tlc_openSession(
    TRDP_APP_SESSION_T              *pAppHandle,
    TRDP_IP_ADDR_T                  ownIpAddr,
    TRDP_IP_ADDR_T                  leaderIpAddr,
    const TRDP_MARSHALL_CONFIG_T    *pMarshall,
    const TRDP_PD_CONFIG_T          *pPdDefault,
    const TRDP_MD_CONFIG_T          *pMdDefault,
    const TRDP_PROCESS_CONFIG_T     *pProcessConfig);
```

Opens a TRDP application session, binds sockets, and initializes PD/MD processing.

- `pAppHandle` – Receives created session handle.
- `ownIpAddr` – Local IP address (host).
- `leaderIpAddr` – Redundancy leader IP (if used, otherwise typical 0).
- `pMarshall` – Optional marshalling/unmarshalling callbacks (`NULL` to disable marshalling).
- `pPdDefault` – Default PD configuration (`TRDP_PD_CONFIG_T`).
- `pMdDefault` – Default MD configuration (`TRDP_MD_CONFIG_T`, may be `NULL` if `MD_SUPPORT == 0`).
- `pProcessConfig` – Process-level configuration (`TRDP_PROCESS_CONFIG_T`).

---

#### `tlc_closeSession`

```c
EXT_DECL TRDP_ERR_T tlc_closeSession(
    TRDP_APP_SESSION_T appHandle);
```

Closes an existing TRDP application session, releasing associated sockets and internal structures.

- `appHandle` – Session handle returned by `tlc_openSession`.

---

#### `tlc_reinitSession`

```c
EXT_DECL TRDP_ERR_T tlc_reinitSession(
    TRDP_APP_SESSION_T appHandle);
```

Reinitializes an existing session (e.g. after network start/stop or interface changes) without recreating the handle.

---

#### `tlc_configSession`

```c
EXT_DECL TRDP_ERR_T tlc_configSession(
    TRDP_APP_SESSION_T              appHandle,
    const TRDP_MARSHALL_CONFIG_T    *pMarshall,
    const TRDP_PD_CONFIG_T          *pPdDefault,
    const TRDP_MD_CONFIG_T          *pMdDefault,
    const TRDP_PROCESS_CONFIG_T     *pProcessConfig);
```

Re-configures an already open session (e.g. adjust defaults, callbacks, priorities).

---

#### `tlc_updateSession`

```c
EXT_DECL TRDP_ERR_T tlc_updateSession(
    TRDP_APP_SESSION_T appHandle);
```

Updates internal session state (e.g. after changing options, topology). Typically used in advanced setups.

---

#### `tlc_presetIndexSession`

```c
EXT_DECL TRDP_ERR_T tlc_presetIndexSession(
    TRDP_APP_SESSION_T  appHandle,
    TRDP_IDX_TABLE_T    *pIndexTableSizes);
```

Pre-allocates index tables for subscriptions and publishers to reduce runtime fragmentation and improve performance.

- `pIndexTableSizes` – Index table sizing hints.

---

### 1.3 Topology Counters

#### `tlc_setETBTopoCount` / `tlc_getETBTopoCount`

```c
EXT_DECL TRDP_ERR_T tlc_setETBTopoCount(
    TRDP_APP_SESSION_T  appHandle,
    UINT32              etbTopoCnt);

EXT_DECL UINT32 tlc_getETBTopoCount(
    TRDP_APP_SESSION_T appHandle);
```

Sets/gets the current **ETB topology counter** for the session, used in TRDP headers.

---

#### `tlc_setOpTrainTopoCount` / `tlc_getOpTrainTopoCount`

```c
EXT_DECL TRDP_ERR_T tlc_setOpTrainTopoCount(
    TRDP_APP_SESSION_T  appHandle,
    UINT32              opTrnTopoCnt);

EXT_DECL UINT32 tlc_getOpTrainTopoCount(
    TRDP_APP_SESSION_T appHandle);
```

Sets/gets the current **Operational Train topology counter** used by the session.

---

### 1.4 Event Loop Integration (Single-Threaded)

#### `tlc_getInterval`

```c
EXT_DECL TRDP_ERR_T tlc_getInterval(
    TRDP_APP_SESSION_T  appHandle,
    TRDP_TIME_T         *pInterval,
    TRDP_FDS_T          *pFileDesc,
    TRDP_SOCK_T         *pNoDesc);
```

Determines the next timeout interval and file descriptors to monitor for PD/MD activity in **single-threaded** mode.

- `pInterval` – Output: time until next internal action.
- `pFileDesc` – Output: FD set to be used with `select()`/`poll()`.
- `pNoDesc` – Output: number of descriptors in the FD set.

---

#### `tlc_process`

```c
EXT_DECL TRDP_ERR_T tlc_process(
    TRDP_APP_SESSION_T  appHandle,
    TRDP_FDS_T          *pRfds,
    INT32               *pCount);
```

Processes incoming/outgoing PD and MD traffic when using a single-threaded, `select()`-based event loop.

- `pRfds` – Ready FD set from `select()`.
- `pCount` – Number of ready descriptors (in/out).

---

### 1.5 Utility

#### `tlc_getOwnIpAddress`

```c
EXT_DECL TRDP_IP_ADDR_T tlc_getOwnIpAddress(
    TRDP_APP_SESSION_T appHandle);
```

Returns the session’s **own IP address** as `TRDP_IP_ADDR_T`.

---

## 2. Process Data (PD) API (`tlp_*`)

These functions manage PD cyclic publishers and subscribers.

### 2.1 PD Event Loop (Multi-Threaded Split)

#### `tlp_getInterval`

```c
EXT_DECL TRDP_ERR_T tlp_getInterval(
    TRDP_APP_SESSION_T  appHandle,
    TRDP_TIME_T         *pInterval,
    TRDP_FDS_T          *pFileDesc,
    TRDP_SOCK_T         *pNoDesc);
```

Like `tlc_getInterval` but **PD-specific** when PD processing is handled in a dedicated thread.

---

#### `tlp_processSend`

```c
EXT_DECL TRDP_ERR_T tlp_processSend(
    TRDP_APP_SESSION_T appHandle);
```

Processes pending PD transmissions (publishers) in a cyclic transmit thread.

---

#### `tlp_processReceive`

```c
EXT_DECL TRDP_ERR_T tlp_processReceive(
    TRDP_APP_SESSION_T  appHandle,
    TRDP_FDS_T          *pRfds,
    INT32               *pCount);
```

Processes received PD packets in a PD receive thread using a `select()` FD set.

---

### 2.2 PD Publishing

#### `tlp_publish`

```c
EXT_DECL TRDP_ERR_T tlp_publish(
    TRDP_APP_SESSION_T      appHandle,
    TRDP_PUB_T              *pPubHandle,
    void                    *pUserRef,
    TRDP_PD_CALLBACK_T      pfCbFunction,
    UINT32                  serviceId,
    UINT32                  comId,
    UINT32                  etbTopoCnt,
    UINT32                  opTrnTopoCnt,
    TRDP_IP_ADDR_T          srcIpAddr,
    TRDP_IP_ADDR_T          destIpAddr,
    UINT32                  interval,
    UINT32                  redId,
    TRDP_FLAGS_T            pktFlags,
    const TRDP_SEND_PARAM_T *pSendParam,
    const UINT8             *pData,
    UINT32                  dataSize);
```

Creates a **PD publisher** for `comId` with given cycle time, redundancy, and initial payload.

- `pPubHandle` – Output: handle for later updates/unpublish.
- `pUserRef` – User reference returned in callbacks (optional).
- `pfCbFunction` – Optional PD send callback (can be `NULL`).
- `serviceId` – Optional service ID field in header.
- `comId` – TRDP ComId of PD telegram.
- `interval` – Cycle time in microseconds.
- `redId` – Redundancy group id.
- `pktFlags` – Flags (e.g. `TRDP_FLAGS_MARSHALL`).
- `pSendParam` – Send parameters (QoS, TTL, VLAN, TSN).
- `pData` / `dataSize` – Initial payload buffer and size.

---

#### `tlp_republish`

```c
EXT_DECL TRDP_ERR_T tlp_republish(
    TRDP_APP_SESSION_T  appHandle,
    TRDP_PUB_T          pubHandle,
    UINT32              etbTopoCnt,
    UINT32              opTrnTopoCnt,
    TRDP_IP_ADDR_T      srcIpAddr,
    TRDP_IP_ADDR_T      destIpAddr);
```

Updates addressing and topology fields for an existing publisher (e.g. after topology changes).

---

#### `tlp_republishService` *(if `SOA_SUPPORT`)*

```c
EXT_DECL TRDP_ERR_T tlp_republishService(
    TRDP_APP_SESSION_T  appHandle,
    TRDP_PUB_T          pubHandle,
    UINT32              etbTopoCnt,
    UINT32              opTrnTopoCnt,
    TRDP_IP_ADDR_T      srcIpAddr,
    TRDP_IP_ADDR_T      destIpAddr,
    UINT32              serviceId);
```

Like `tlp_republish`, but also updates the **serviceId** (service oriented architecture).

---

#### `tlp_unpublish`

```c
EXT_DECL TRDP_ERR_T tlp_unpublish(
    TRDP_APP_SESSION_T  appHandle,
    TRDP_PUB_T          pubHandle);
```

Stops publishing and removes the publisher identified by `pubHandle`.

---

#### `tlp_put`

```c
EXT_DECL TRDP_ERR_T tlp_put(
    TRDP_APP_SESSION_T  appHandle,
    TRDP_PUB_T          pubHandle,
    const UINT8         *pData,
    UINT32              dataSize);
```

Updates the payload of an existing publisher. Data is sent at the next scheduled cycle.

---

#### `tlp_putImmediate`

```c
EXT_DECL TRDP_ERR_T tlp_putImmediate(
    TRDP_APP_SESSION_T  appHandle,
    TRDP_PUB_T          pubHandle,
    const UINT8         *pData,
    UINT32              dataSize,
    VOS_TIMEVAL_T       *pTxTime);
```

Sends a PD packet **immediately**, independent of the cyclic schedule.

- `pTxTime` – Optional pointer receiving actual transmit time.

---

#### `tlp_setRedundant` / `tlp_getRedundant`

```c
EXT_DECL TRDP_ERR_T tlp_setRedundant(
    TRDP_APP_SESSION_T  appHandle,
    UINT32              redId,
    BOOL8               leader);

EXT_DECL TRDP_ERR_T tlp_getRedundant(
    TRDP_APP_SESSION_T  appHandle,
    UINT32              redId,
    BOOL8               *pLeader);
```

Configures and queries redundancy state (leader/follower) for redundancy group `redId`.

---

### 2.3 PD Pull / Request

#### `tlp_request`

```c
EXT_DECL TRDP_ERR_T tlp_request(
    TRDP_APP_SESSION_T      appHandle,
    TRDP_SUB_T              subHandle,
    UINT32                  serviceId,
    UINT32                  comId,
    UINT32                  etbTopoCnt,
    UINT32                  opTrnTopoCnt,
    TRDP_IP_ADDR_T          srcIpAddr,
    TRDP_IP_ADDR_T          destIpAddr,
    UINT32                  redId,
    TRDP_FLAGS_T            pktFlags,
    const TRDP_SEND_PARAM_T *pSendParam,
    const UINT8             *pData,
    UINT32                  dataSize,
    UINT32                  replyComId,
    TRDP_IP_ADDR_T          replyIpAddr);
```

Sends a **PD pull request** (one-shot PD telegram) and expects a reply via `subHandle` subscription.

---

### 2.4 PD Subscription

#### `tlp_subscribe`

```c
EXT_DECL TRDP_ERR_T tlp_subscribe(
    TRDP_APP_SESSION_T      appHandle,
    TRDP_SUB_T              *pSubHandle,
    void                    *pUserRef,
    TRDP_PD_CALLBACK_T      pfCbFunction,
    UINT32                  serviceId,
    UINT32                  comId,
    UINT32                  etbTopoCnt,
    UINT32                  opTrnTopoCnt,
    TRDP_IP_ADDR_T          srcIpAddr1,
    TRDP_IP_ADDR_T          srcIpAddr2,
    TRDP_IP_ADDR_T          destIpAddr,
    TRDP_FLAGS_T            pktFlags,
    const TRDP_COM_PARAM_T  *pRecParams,
    UINT32                  timeout,
    TRDP_TO_BEHAVIOR_T      toBehavior);
```

Creates a **PD subscription** for a given `comId` and (optional) source/destination IP filters.

- `pSubHandle` – Output handle for later get/unsubscribe.
- `pUserRef` – User reference passed in callback.
- `pfCbFunction` – PD receive callback (`NULL` if using polling with `tlp_get`).
- `timeout` – PD timeout in microseconds (0 = no supervision).
- `toBehavior` – Timeout behavior (zero/keep last).

---

#### `tlp_resubscribe`

```c
EXT_DECL TRDP_ERR_T tlp_resubscribe(
    TRDP_APP_SESSION_T  appHandle,
    TRDP_SUB_T          subHandle,
    UINT32              etbTopoCnt,
    UINT32              opTrnTopoCnt,
    TRDP_IP_ADDR_T      srcIpAddr1,
    TRDP_IP_ADDR_T      srcIpAddr2,
    TRDP_IP_ADDR_T      destIpAddr);
```

Updates addressing/topology for an existing subscription.

---

#### `tlp_unsubscribe`

```c
EXT_DECL TRDP_ERR_T tlp_unsubscribe(
    TRDP_APP_SESSION_T  appHandle,
    TRDP_SUB_T          subHandle);
```

Removes a PD subscription.

---

#### `tlp_get`

```c
EXT_DECL TRDP_ERR_T tlp_get(
    TRDP_APP_SESSION_T  appHandle,
    TRDP_SUB_T          subHandle,
    TRDP_PD_INFO_T      *pPdInfo,
    UINT8               *pData,
    UINT32              *pDataSize);
```

Retrieves the **latest received PD packet** for a subscription (polling style).

- `pPdInfo` – Output: PD meta information (comId, topo, result, etc.).
- `pData` / `pDataSize` – Payload buffer and size (in/out).

---

## 3. Message Data (MD) API (`tlm_*`)

> Available only if `MD_SUPPORT == 1`.

### 3.1 MD Event Loop (Multi-Threaded Split)

#### `tlm_process`

```c
EXT_DECL TRDP_ERR_T tlm_process(
    TRDP_APP_SESSION_T  appHandle,
    TRDP_FDS_T          *pRfds,
    INT32               *pCount);
```

Processes MD traffic in a dedicated MD thread using `select()`.

---

#### `tlm_getInterval`

```c
EXT_DECL TRDP_ERR_T tlm_getInterval(
    TRDP_APP_SESSION_T  appHandle,
    TRDP_TIME_T         *pInterval,
    TRDP_FDS_T          *pFileDesc,
    TRDP_SOCK_T         *pNoDesc);
```

Determines MD-related interval and FD set for a separate MD processing thread.

---

### 3.2 MD Notification (one-way)

#### `tlm_notify`

```c
EXT_DECL TRDP_ERR_T tlm_notify(
    TRDP_APP_SESSION_T      appHandle,
    void                    *pUserRef,
    TRDP_MD_CALLBACK_T      pfCbFunction,
    UINT32                  comId,
    UINT32                  etbTopoCnt,
    UINT32                  opTrnTopoCnt,
    TRDP_IP_ADDR_T          srcIpAddr,
    TRDP_IP_ADDR_T          destIpAddr,
    TRDP_FLAGS_T            pktFlags,
    const TRDP_SEND_PARAM_T *pSendParam,
    const UINT8             *pData,
    UINT32                  dataSize,
    const TRDP_URI_USER_T   srcURI,
    const TRDP_URI_USER_T   destURI);
```

Sends an **MD Notification** (`Mn`) without expecting a reply.

---

### 3.3 MD Request / Reply / Confirm

#### `tlm_request`

```c
EXT_DECL TRDP_ERR_T tlm_request(
    TRDP_APP_SESSION_T      appHandle,
    void                    *pUserRef,
    TRDP_MD_CALLBACK_T      pfCbFunction,
    TRDP_UUID_T             *pSessionId,
    UINT32                  comId,
    UINT32                  etbTopoCnt,
    UINT32                  opTrnTopoCnt,
    TRDP_IP_ADDR_T          srcIpAddr,
    TRDP_IP_ADDR_T          destIpAddr,
    TRDP_FLAGS_T            pktFlags,
    UINT32                  numReplies,
    UINT32                  replyTimeout,
    const TRDP_SEND_PARAM_T *pSendParam,
    const UINT8             *pData,
    UINT32                  dataSize,
    const TRDP_URI_USER_T   srcURI,
    const TRDP_URI_USER_T   destURI);
```

Sends an **MD Request** (`Mr`) and optionally expects one or more replies.

- `pSessionId` – Output: session UUID to identify conversation.
- `numReplies` – Expected number of replies (0 if unknown).
- `replyTimeout` – Timeout per reply in microseconds.

---

#### `tlm_confirm`

```c
EXT_DECL TRDP_ERR_T tlm_confirm(
    TRDP_APP_SESSION_T      appHandle,
    const TRDP_UUID_T       *pSessionId,
    UINT16                  userStatus,
    const TRDP_SEND_PARAM_T *pSendParam);
```

Sends an **MD Confirm** (`Mc`) for a reply that requested confirmation.

---

#### `tlm_abortSession`

```c
EXT_DECL TRDP_ERR_T tlm_abortSession(
    TRDP_APP_SESSION_T  appHandle,
    const TRDP_UUID_T   *pSessionId);
```

Aborts an active MD session (e.g. timeout or application decision).

---

### 3.4 MD Listeners

#### `tlm_addListener`

```c
EXT_DECL TRDP_ERR_T tlm_addListener(
    TRDP_APP_SESSION_T      appHandle,
    TRDP_LIS_T              *pListenHandle,
    void                    *pUserRef,
    TRDP_MD_CALLBACK_T      pfCbFunction,
    BOOL8                   comIdListener,
    UINT32                  comId,
    UINT32                  etbTopoCnt,
    UINT32                  opTrnTopoCnt,
    TRDP_IP_ADDR_T          srcIpAddr1,
    TRDP_IP_ADDR_T          srcIpAddr2,
    TRDP_IP_ADDR_T          mcDestIpAddr,
    TRDP_FLAGS_T            pktFlags,
    const TRDP_URI_USER_T   srcURI,
    const TRDP_URI_USER_T   destURI);
```

Creates an **MD listener** for certain `comId` and/or URI filter. Incoming MDs trigger `pfCbFunction`.

- `comIdListener` – If `TRUE`, filter by `comId`, otherwise more generic URI matching.

---

#### `tlm_readdListener`

```c
EXT_DECL TRDP_ERR_T tlm_readdListener(
    TRDP_APP_SESSION_T  appHandle,
    TRDP_LIS_T          listenHandle,
    UINT32              etbTopoCnt,
    UINT32              opTrnTopoCnt,
    TRDP_IP_ADDR_T      srcIpAddr,
    TRDP_IP_ADDR_T      srcIpAddr2,
    TRDP_IP_ADDR_T      mcDestIpAddr);
```

Reconfigures an existing MD listener (e.g. new topology or multicast address).

---

#### `tlm_delListener`

```c
EXT_DECL TRDP_ERR_T tlm_delListener(
    TRDP_APP_SESSION_T  appHandle,
    TRDP_LIS_T          listenHandle);
```

Deletes an MD listener.

---

### 3.5 MD Replies

#### `tlm_reply`

```c
EXT_DECL TRDP_ERR_T tlm_reply(
    TRDP_APP_SESSION_T      appHandle,
    const TRDP_UUID_T       *pSessionId,
    UINT32                  comId,
    UINT32                  userStatus,
    const TRDP_SEND_PARAM_T *pSendParam,
    const UINT8             *pData,
    UINT32                  dataSize,
    const CHAR8             *srcURI);
```

Sends an **MD Reply** (`Mp` or `Mq`) to a received MD Request, without requesting a follow-up confirm.

- `userStatus` – Application-specific status code.

---

#### `tlm_replyQuery`

```c
EXT_DECL TRDP_ERR_T tlm_replyQuery(
    TRDP_APP_SESSION_T      appHandle,
    const TRDP_UUID_T       *pSessionId,
    UINT32                  comId,
    UINT32                  userStatus,
    UINT32                  confirmTimeout,
    const TRDP_SEND_PARAM_T *pSendParam,
    const UINT8             *pData,
    UINT32                  dataSize,
    const CHAR8             *srcURI);
```

Sends an **MD Reply with Confirm required** (`Mq`). A confirm (`Mc`) is expected within `confirmTimeout` microseconds.

---

## 4. Version & Statistics (`tlc_*`)

### 4.1 Version Information

#### `tlc_getVersionString`

```c
EXT_DECL const CHAR8 *tlc_getVersionString(void);
```

Returns a human-readable, null-terminated string with the TRDP library version.

---

#### `tlc_getVersion`

```c
EXT_DECL const TRDP_VERSION_T *tlc_getVersion(void);
```

Returns a pointer to a `TRDP_VERSION_T` structure with version components.

---

### 4.2 Global Statistics

#### `tlc_getStatistics`

```c
EXT_DECL TRDP_ERR_T tlc_getStatistics(
    TRDP_APP_SESSION_T  appHandle,
    TRDP_STATISTICS_T   *pStatistics);
```

Retrieves aggregated statistics for the session (memory, PD, MD, uptime, etc.).

---

### 4.3 Detailed Statistics

#### `tlc_getSubsStatistics`

```c
EXT_DECL TRDP_ERR_T tlc_getSubsStatistics(
    TRDP_APP_SESSION_T      appHandle,
    UINT16                  *pNumSubs,
    TRDP_SUBS_STATISTICS_T  *pStatistics);
```

Retrieves detailed PD subscription statistics.

- `pNumSubs` – In: max entries available; Out: actual number returned.
- `pStatistics` – Array of `TRDP_SUBS_STATISTICS_T` entries.

---

#### `tlc_getPubStatistics`

```c
EXT_DECL TRDP_ERR_T tlc_getPubStatistics(
    TRDP_APP_SESSION_T      appHandle,
    UINT16                  *pNumPub,
    TRDP_PUB_STATISTICS_T   *pStatistics);
```

Retrieves PD publisher statistics.

---

#### `tlc_getUdpListStatistics` / `tlc_getTcpListStatistics` *(MD)*

```c
EXT_DECL TRDP_ERR_T tlc_getUdpListStatistics(
    TRDP_APP_SESSION_T      appHandle,
    UINT16                  *pNumList,
    TRDP_LIST_STATISTICS_T  *pStatistics);

EXT_DECL TRDP_ERR_T tlc_getTcpListStatistics(
    TRDP_APP_SESSION_T      appHandle,
    UINT16                  *pNumList,
    TRDP_LIST_STATISTICS_T  *pStatistics);
```

Retrieve MD listener statistics (UDP/TCP).

---

#### `tlc_getRedStatistics`

```c
EXT_DECL TRDP_ERR_T tlc_getRedStatistics(
    TRDP_APP_SESSION_T      appHandle,
    UINT16                  *pNumRed,
    TRDP_RED_STATISTICS_T   *pStatistics);
```

Retrieves statistics for redundancy groups.

---

#### `tlc_getJoinStatistics`

```c
EXT_DECL TRDP_ERR_T tlc_getJoinStatistics(
    TRDP_APP_SESSION_T  appHandle,
    UINT16              *pNumJoin,
    UINT32              *pIpAddr);
```

Returns multicast join statistics:

- `pNumJoin` – In: max entries; Out: number filled.
- `pIpAddr` – Array of joined multicast IP addresses.

---

#### `tlc_resetStatistics`

```c
EXT_DECL TRDP_ERR_T tlc_resetStatistics(
    TRDP_APP_SESSION_T appHandle);
```

Resets all statistics counters for the given session.

---

## 5. Notes for Usage in the Simulator

- Always call `tlc_init()` once at process start, and `tlc_terminate()` at shutdown.
- For a **single-threaded** design, use `tlc_getInterval()` + `tlc_process()`.
- For **multi-threaded** (recommended for low jitter), use:
  - PD: `tlp_getInterval()`, `tlp_processSend()`, `tlp_processReceive()`
  - MD: `tlm_getInterval()`, `tlm_process()`
- Use `tlp_subscribe()` + callback for live PD monitoring, and `tlp_get()` for polling modes.
- Use MD listeners (`tlm_addListener()`) to react to requests/notifications, and `tlm_reply()` / `tlm_replyQuery()` to respond.
- Dataset marshalling can be fully delegated to TRDP by enabling `TRDP_FLAGS_MARSHALL` and providing dataset definitions via XML.

This document is intended to live at the project root as:

```text
TRDP_API_trdp_if_light.md
```

for easy reference by Codex and human developers.
