Sure — here is the **pure Markdown textual content** you can directly copy-paste into a file and save as **`TRDP_API_tau_dnr_types.md`**.

---

````markdown
# TRDP API Documentation — tau_dnr_types.h

## Overview
`tau_dnr_types.h` provides type definitions and structured data models required by the **DNR (Dynamic Name Resolution)** and **TCN-DNS** components of the TRDP stack.  
These types enable URI–IP address translation, DNR request/response processing, and store train topology resolution information.

---

## Included Header Files
| Header | Description |
|--------|-------------|
| `trdp_types.h` | Core TRDP type definitions |
| `trdp_serviceRegistry.h` *(optional, only with SOA_SUPPORT)* | Service registry interface for service-oriented architecture |

---

## Typedef Structures

---

### `TCN_URI_T`
Represents a single TCN-URI entry in DNR request/response messages.

```c
typedef struct TCN_URI
{
    CHAR8   tcnUriStr[80];
    INT16   reserved01;
    INT16   resolvState;
    UINT32  tcnUriIpAddr;
    UINT32  tcnUriIpAddr2;
} GNU_PACKED TCN_URI_T;
````

#### Field Description

| Field           | Type      | Description             |
| --------------- | --------- | ----------------------- |
| `tcnUriStr[80]` | `CHAR8[]` | URI string              |
| `resolvState`   | `INT16`   | -1 = unknown, 0 = ok    |
| `tcnUriIpAddr`  | `UINT32`  | resolved IP address     |
| `tcnUriIpAddr2` | `UINT32`  | end of range (optional) |

---

### `TRDP_DNS_REQUEST_T`

Defines a DNR request telegram.

```c
typedef struct TRDP_DNS_REQUEST
{
    TRDP_SHORT_VERSION_T    version;
    INT16                   reserved01;
    TRDP_NET_LABEL_T        deviceName;
    UINT32                  etbTopoCnt;
    UINT32                  opTrnTopoCnt;
    UINT8                   etbId;
    UINT8                   reserved02;
    UINT8                   reserved03;
    UINT8                   tcnUriCnt;
    TCN_URI_T               tcnUriList[255];
    TRDP_ETB_CTRL_VDP_T     safetyTrail;
} GNU_PACKED TRDP_DNS_REQUEST_T;
```

#### Field Description

| Field                         | Type                      | Description |
| ----------------------------- | ------------------------- | ----------- |
| `version`                     | protocol version          |             |
| `deviceName`                  | request sender device     |             |
| `etbTopoCnt` / `opTrnTopoCnt` | topology counters         |             |
| `etbId`                       | ETB identifier            |             |
| `tcnUriCnt`                   | number of URIs to resolve |             |
| `tcnUriList[]`                | list of URI entries       |             |
| `safetyTrail`                 | SDT safety trailer        |             |

---

### `TRDP_DNS_REPLY_T`

Defines a DNR reply telegram.

```c
typedef struct TRDP_DNS_REPLY
{
    TRDP_SHORT_VERSION_T    version;
    INT16                   reserved01;
    TRDP_NET_LABEL_T        deviceName;
    UINT32                  etbTopoCnt;
    UINT32                  opTrnTopoCnt;
    UINT8                   etbId;
    INT8                    dnsStatus;
    UINT8                   reserved02;
    UINT8                   tcnUriCnt;
    TCN_URI_T               tcnUriList[255];
    TRDP_ETB_CTRL_VDP_T     safetyTrail;
} GNU_PACKED TRDP_DNS_REPLY_T;
```

#### Field Description

| Field          | Type                                         | Description |
| -------------- | -------------------------------------------- | ----------- |
| `dnsStatus`    | 0 = OK, -1 server not ready, -2 inauguration |             |
| `tcnUriCnt`    | number of entries                            |             |
| `tcnUriList[]` | resolved entries                             |             |

---

## Notes

* Structures use `GNU_PACKED` for byte alignment due to TRDP wire protocol compatibility.
* Maximum of **255 URI entries** in request/response messages.
* Used by `tau_dnr.h` runtime API functions.

---

## Related APIs

| Function          | Description          |
| ----------------- | -------------------- |
| `tau_initDnr()`   | Setup DNS/DNR system |
| `tau_uri2Addr()`  | Convert URI → IP     |
| `tau_addr2Uri()`  | Convert IP → URI     |
| `tau_DNRstatus()` | Get status of DNR    |
| `tau_deInitDnr()` | Cleanup resources    |

---

## Usage Example

```c
TRDP_DNS_REQUEST_T request;
memset(&request, 0, sizeof(request));

request.tcnUriCnt = 1;
strncpy(request.tcnUriList[0].tcnUriStr, "dev01.car01.cst01.lTrain", 80);
```

---

## Revision Information

| Version        | Summary                                     |
| -------------- | ------------------------------------------- |
| **2017-11-13** | TRDP_LABEL_T replaced with TRDP_NET_LABEL_T |
| **2017-07-25** | Initial DNR client implementation           |

---

## Copyright

© 2017–2024 NewTec GmbH, Bombardier Transportation & Contributors
Licensed under MPL 2.0

```

---

🙂
```
