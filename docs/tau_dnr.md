# TRDP API Reference – `tau_dnr.h`

This document summarises the public API provided by **`tau_dnr.h`** from the TCNOpen TRDP stack.  
It is intended as a quick reference for Codex and developers integrating the TRDP name‑resolution utilities.

---

## 1. Header Overview

```c
#include "tau_dnr.h"
```

This header provides the **DNR (DNS / Name Resolution) helper API** for TRDP applications:

- Initialisation and shutdown of the DNR helper.
- Status monitoring of the DNR subsystem.
- Conversion between **TCN URIs** and **IPv4 addresses**.
- Access to the configured **own IP address**.

> All types such as `TRDP_APP_SESSION_T`, `TRDP_IP_ADDR_T`, `TRDP_ERR_T`, `TRDP_URI_HOST_T` etc. are defined in `trdp_types.h`.

---

## 2. Constants and Limits

### 2.1 Maximum Cache Entries

```c
#define TAU_MAX_NO_CACHE_ENTRY      50u
```

- Maximum number of DNR cache entries internally stored by the helper.

---

## 3. Enumerations

### 3.1 `TRDP_DNR_STATE_T` – DNR State

```c
typedef enum TRDP_DNR_STATE {
    TRDP_DNR_UNKNOWN        = 0,
    TRDP_DNR_NOT_AVAILABLE  = 1,
    TRDP_DNR_ACTIVE         = 2,
    TRDP_DNR_HOSTSFILE      = 3
} TRDP_DNR_STATE_T;
```

Represents the current state of the DNR subsystem for a given TRDP session:

- `TRDP_DNR_UNKNOWN` – DNR enabled but no information / cache yet.
- `TRDP_DNR_NOT_AVAILABLE` – DNR support not available.
- `TRDP_DNR_ACTIVE` – DNR enabled and active (cache contains entries).
- `TRDP_DNR_HOSTSFILE` – DNR operating in static host‑file mode.

---

### 3.2 `TRDP_DNR_OPTS_T` – DNR Options

```c
typedef enum TRDP_DNR_OPTS {
    TRDP_DNR_COMMON_THREAD  = 0,
    TRDP_DNR_OWN_THREAD     = 1,
    TRDP_DNR_STANDARD_DNS   = 2
} TRDP_DNR_OPTS_T;
```

Defines how the DNR client integrates with the application:

- `TRDP_DNR_COMMON_THREAD` – Use the existing TRDP processing loop (`tlc_process`) for DNR (recommended for typical multi‑threaded setups).
- `TRDP_DNR_OWN_THREAD` – Run DNR in its own internal thread (intended for single‑threaded systems; internally calls `tlc_process()`).
- `TRDP_DNR_STANDARD_DNS` – Use standard DNS instead of TCN‑DNS for name resolution.

---

## 4. Data Structures

### 4.1 `TAU_DNR_ENTRY_T` – DNR Cache Entry

```c
typedef struct tau_dnr_cache
{
    CHAR8           uri[TRDP_MAX_URI_HOST_LEN];
    TRDP_IP_ADDR_T  ipAddr;
    UINT32          etbTopoCnt;
    UINT32          opTrnTopoCnt;
    BOOL8           fixedEntry;
} TAU_DNR_ENTRY_T;
```

Represents a single address mapping entry in the DNR cache:

- `uri` – Host part of the URI associated with this entry.
- `ipAddr` – Resolved IP address.
- `etbTopoCnt` – ETB topological counter used with this entry.
- `opTrnTopoCnt` – Operational train topology counter for this entry.
- `fixedEntry` – If non‑zero, the entry is static and should not be overwritten by dynamic resolution.

---

### 4.2 `TAU_DNR_DATA_T` – DNR Configuration and Cache

```c
typedef struct tau_dnr_data
{
    TRDP_IP_ADDR_T  dnsIpAddr;                      /* IP address of the resolver                 */
    UINT16          dnsPort;                        /* 53 for standard DNS or 17225 for TCN-DNS   */
    UINT8           timeout;                        /* timeout for requests (in seconds)          */
    TRDP_DNR_OPTS_T useTCN_DNS;                     /* how to use TCN DNR                         */
    UINT32          noOfCachedEntries;              /* no of items currently in the cache         */
    TAU_DNR_ENTRY_T cache[TAU_MAX_NO_CACHE_ENTRY];  /* cache array                                */
} TAU_DNR_DATA_T;
```

Internal state object describing one DNR resolver instance:

- `dnsIpAddr` – DNS or ECSP server IP address.
- `dnsPort` – Port number of the resolver (e.g. 53 for DNS, 17225 for TCN‑DNS).
- `timeout` – Request timeout in seconds.
- `useTCN_DNS` – Selected DNR operating mode (`TRDP_DNR_OPTS_T`).
- `noOfCachedEntries` – Number of valid entries currently present in `cache`.
- `cache` – Fixed‑size array of cached URI/IP mappings.

> This structure is typically managed internally by the DNR helper; application code usually interacts via the API functions rather than accessing this directly.

---

## 5. Functions

This section lists all public functions declared in `tau_dnr.h` that are not wrapped in `#if 0`.  
Functions inside the `#if 0` block are **disabled** and therefore omitted here.

### 5.1 DNR Initialisation and Shutdown

#### 5.1.1 `tau_initDnr` – Initialise DNR

```c
EXT_DECL TRDP_ERR_T tau_initDnr(
    TRDP_APP_SESSION_T  appHandle,
    TRDP_IP_ADDR_T      dnsIpAddr,
    UINT16              dnsPort,
    const CHAR8         *pHostsFileName,
    TRDP_DNR_OPTS_T     dnsOptions,
    BOOL8               waitForDnr);
```

Initialises the DNR subsystem for a given TRDP application session.

**Parameters**

- `appHandle` – Application session handle obtained from `tlc_openSession()`.
- `dnsIpAddr` – IP address of the DNS/ECSP server to be used.
- `dnsPort` – Port number for the resolver (e.g. 53 for standard DNS, 17225 for TCN‑DNS).
- `pHostsFileName` – Optional path to a hosts file that may supplement or replace ECSP/DNS; can be `NULL` if unused.
- `dnsOptions` – DNR operating mode (`TRDP_DNR_OPTS_T`), controls whether to use a common thread, own thread, or standard DNS.
- `waitForDnr` – If `TRUE`, call blocks until DNR is ready (recommended); if `FALSE`, initialisation returns without waiting (useful for tests).

**Return Values**

- `TRDP_NO_ERR` – Initialisation succeeded.
- `TRDP_INIT_ERR` – Initialisation failed.

---

#### 5.1.2 `tau_deInitDnr` – De‑initialise DNR

```c
EXT_DECL void tau_deInitDnr(
    TRDP_APP_SESSION_T appHandle);
```

Releases all resources allocated by the DNR subsystem for the given application session.

**Parameters**

- `appHandle` – Application session handle returned by `tlc_openSession()`.

**Return Value**

- None.

---

### 5.2 DNR Status

#### 5.2.1 `tau_DNRstatus` – Query DNR State

```c
EXT_DECL TRDP_DNR_STATE_T tau_DNRstatus(
    TRDP_APP_SESSION_T  appHandle);
```

Returns the current DNR state for the given TRDP session.

**Parameters**

- `appHandle` – Application session handle.

**Return Value**

A value of type `TRDP_DNR_STATE_T`:

- `TRDP_DNR_NOT_AVAILABLE` – DNR is not available.
- `TRDP_DNR_UNKNOWN` – DNR enabled but cache is empty / not initialised.
- `TRDP_DNR_ACTIVE` – DNR enabled and cache holds entries.
- `TRDP_DNR_HOSTSFILE` – DNR uses a hosts file in static mode.

---

### 5.3 Address and URI Conversions

#### 5.3.1 `tau_getOwnAddr` – Get Own IP Address

```c
EXT_DECL TRDP_IP_ADDR_T tau_getOwnAddr(
    TRDP_APP_SESSION_T   appHandle);
```

Returns the IP address associated with the given TRDP application session.

**Parameters**

- `appHandle` – Application session handle returned by `tlc_openSession()`.

**Return Value**

- The local `TRDP_IP_ADDR_T` address for the session.

---

#### 5.3.2 `tau_uri2Addr` – Convert URI to IP Address

```c
EXT_DECL TRDP_ERR_T tau_uri2Addr(
    TRDP_APP_SESSION_T   appHandle,
    TRDP_IP_ADDR_T      *pAddr,
    const CHAR8         *pUri);
```

Converts a TCN URI or string representation of an IP address into a `TRDP_IP_ADDR_T`.  
The URI may denote unicast or multicast destinations. A `NULL` `pUri` typically means “own URI”.

**Parameters**

- `appHandle` – Application session handle returned by `tlc_openSession()`.
- `pAddr` – Pointer to location where the resulting IP address will be stored.
- `pUri` – Pointer to URI string or IP address string; `NULL` may indicate own URI depending on configuration.

**Return Values**

- `TRDP_NO_ERR` – Conversion succeeded and `*pAddr` has been set.
- `TRDP_PARAM_ERR` – Invalid parameters or unresolved URI.

---

#### 5.3.3 `tau_ipFromURI` – Convenience URI → IP Helper

```c
EXT_DECL TRDP_IP_ADDR_T tau_ipFromURI(
    TRDP_APP_SESSION_T  appHandle,
    const CHAR8         *uri);
```

Convenience function to obtain an IP address directly from a URI string, returning the address by value.

**Parameters**

- `appHandle` – Application session handle.
- `uri` – URI string to resolve to an IP address.

**Return Value**

- Resolved `TRDP_IP_ADDR_T` for the given URI. If resolution fails, the value may indicate an invalid/zero address depending on implementation; check for errors with `tau_uri2Addr` if you need explicit error codes.

---

#### 5.3.4 `tau_addr2Uri` – Convert IP Address to URI Host Part

```c
EXT_DECL TRDP_ERR_T tau_addr2Uri(
    TRDP_APP_SESSION_T   appHandle,
    TRDP_URI_HOST_T      pUri,
    TRDP_IP_ADDR_T       addr);
```

Converts an IP address to the corresponding host part of a TCN URI.  
Both unicast and multicast IP addresses are accepted. Passing `addr = 0` usually refers to the local device address.

**Parameters**

- `appHandle` – Application session handle.
- `pUri` – Output buffer for the resulting URI host string (`TRDP_URI_HOST_T`).
- `addr` – IP address to be mapped; `0` refers to the own IP address.

**Return Values**

- `TRDP_NO_ERR` – Conversion succeeded and `pUri` contains the host part.
- `TRDP_PARAM_ERR` – Invalid arguments or mapping not available.

---

## 6. Disabled / Optional APIs

The header contains additional helper functions within a `#if 0` block. These cover conversions between labels, vehicle/consist IDs, TCN vehicle numbers, operational vehicle numbers, and IP addresses.

Since they are wrapped in `#if 0`, they are **not part of the compiled public API** in this configuration and are intentionally omitted from this reference. If they are ever enabled, this document should be extended accordingly.

---

## 7. Usage Notes

- **Session dependency:** All APIs require a valid `TRDP_APP_SESSION_T` created by `tlc_openSession()` from `trdp_if_light.h`.
- **Error handling:** Functions returning `TRDP_ERR_T` use the standard TRDP error codes defined in `trdp_types.h` (e.g. `TRDP_NO_ERR`, `TRDP_PARAM_ERR`, `TRDP_INIT_ERR`).
- **Threading model:** When using `TRDP_DNR_OWN_THREAD`, ensure the application design is compatible with the internal call behaviour of `tlc_process()`. For most applications `TRDP_DNR_COMMON_THREAD` is simpler.
- **Topology awareness:** For systems that use train topology counters (`etbTopoCnt`, `opTrnTopoCnt`), pay attention to how DNR cache entries are tied to those values via `TAU_DNR_ENTRY_T`.

This file should live in the repository root as:

```text
TRDP_API_tau_dnr.md
```

and be kept in sync with the version of `tau_dnr.h` vendored into the project.
