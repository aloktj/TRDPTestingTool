# TRDP API Documentation — `trdp_types.h`

## 1. Overview

`trdp_types.h` defines **all fundamental data types, error codes, configuration structures, flags, and statistics structures** used by the TRDP Light stack.

This header is the **base dependency** for almost every TRDP API header (such as `trdp_if_light.h`, `tau_xml.h`, `tau_dnr.h`, etc.).  

Include it indirectly via the public TRDP headers (for example `trdp_if_light.h`), not usually on its own.

---

## 2. Included Headers

```c
#include "vos_types.h"
#include "vos_mem.h"
#include "vos_sock.h"
#include "iec61375-2-3.h"

#ifdef TSN_SUPPORT
#include "trdp_tsn_def.h"
#endif
These provide:

Basic integer and platform types (VOS_*)

Memory statistics and configuration (vos_mem.h)

Socket types and descriptors (vos_sock.h)

IEC 61375-2-3 specific types and constants (iec61375-2-3.h)

Optional TSN/TRDP v2 extensions (trdp_tsn_def.h)

3. DLL Export Macro
c
Copy code
#if (defined (WIN32) || defined (WIN64))
    #ifdef DLL_EXPORT
        #define EXT_DECL    __declspec(dllexport)
    #elif DLL_IMPORT
        #define EXT_DECL    __declspec(dllimport)
    #else
        #define EXT_DECL
    #endif
#else
    #define EXT_DECL
#endif
All public TRDP functions are declared as EXT_DECL to support Windows DLL import/export and remain empty on other platforms.

4. Core Basic Types
4.1 Address and Label Types
c
Copy code
typedef VOS_IP4_ADDR_T TRDP_IP_ADDR_T;
typedef CHAR8 TRDP_LABEL_T[TRDP_MAX_LABEL_LEN + 1];
typedef CHAR8 TRDP_EXTRA_LABEL_T[TRDP_EXTRA_LABEL_LEN + 1];
typedef CHAR8 TRDP_NET_LABEL_T[TRDP_MAX_LABEL_LEN];
typedef CHAR8 TRDP_URI_T[TRDP_MAX_URI_LEN + 1];
typedef CHAR8 TRDP_URI_HOST_T[TRDP_MAX_URI_HOST_LEN + 1];
typedef CHAR8 TRDP_URI_USER_T[TRDP_MAX_URI_USER_LEN + 1];
typedef CHAR8 TRDP_FILE_NAME_T[TRDP_MAX_FILE_NAME_LEN + 1];
TRDP_IP_ADDR_T: IPv4 address in host-endian UINT32.

TRDP_LABEL_T: Generic textual label (null-terminated).

TRDP_NET_LABEL_T: Same size as label but not guaranteed to be null-terminated (for on-wire usage).

TRDP_URI_*: Split URI into host part and user part for TRDP addressing.

TRDP_FILE_NAME_T: File path string, typically for debug/logging.

4.2 Version, Time, FD, Socket
c
Copy code
typedef VOS_VERSION_T TRDP_VERSION_T;
typedef VOS_TIMEVAL_T TRDP_TIME_T;
typedef VOS_FDS_T     TRDP_FDS_T;
typedef VOS_SOCK_T    TRDP_SOCK_T;

#define TRDP_INVALID_SOCKET  VOS_INVALID_SOCKET
TRDP_VERSION_T: Version triple (major, minor, patch), identical to VOS_VERSION_T.

TRDP_TIME_T: Compatible with struct timeval (seconds, microseconds).

TRDP_FDS_T: File descriptor set, compatible with fd_set.

TRDP_SOCK_T: Abstract socket descriptor; TRDP_INVALID_SOCKET is an invalid handle sentinel.

4.3 UUID
c
Copy code
typedef VOS_UUID_T TRDP_UUID_T;
Used for MD sessions, unique transaction/interaction IDs.

5. Error Handling
5.1 TRDP_ERR_T — Global Return Codes
c
Copy code
typedef enum
{
    TRDP_NO_ERR             = 0,
    TRDP_PARAM_ERR          = -1,
    TRDP_INIT_ERR           = -2,
    TRDP_NOINIT_ERR         = -3,
    TRDP_TIMEOUT_ERR        = -4,
    TRDP_NODATA_ERR         = -5,
    TRDP_SOCK_ERR           = -6,
    TRDP_IO_ERR             = -7,
    TRDP_MEM_ERR            = -8,
    TRDP_SEMA_ERR           = -9,
    TRDP_QUEUE_ERR          = -10,
    TRDP_QUEUE_FULL_ERR     = -11,
    TRDP_MUTEX_ERR          = -12,
    TRDP_THREAD_ERR         = -13,
    TRDP_BLOCK_ERR          = -14,
    TRDP_INTEGRATION_ERR    = -15,
    TRDP_NOCONN_ERR         = -16,
    TRDP_NOSESSION_ERR      = -30,
    TRDP_SESSION_ABORT_ERR  = -31,
    TRDP_NOSUB_ERR          = -32,
    TRDP_NOPUB_ERR          = -33,
    TRDP_NOLIST_ERR         = -34,
    TRDP_CRC_ERR            = -35,
    TRDP_WIRE_ERR           = -36,
    TRDP_TOPO_ERR           = -37,
    TRDP_COMID_ERR          = -38,
    TRDP_STATE_ERR          = -39,
    TRDP_APP_TIMEOUT_ERR    = -40,
    TRDP_APP_REPLYTO_ERR    = -41,
    TRDP_APP_CONFIRMTO_ERR  = -42,
    TRDP_REPLYTO_ERR        = -43,
    TRDP_CONFIRMTO_ERR      = -44,
    TRDP_REQCONFIRMTO_ERR   = -45,
    TRDP_PACKET_ERR         = -46,
    TRDP_UNRESOLVED_ERR     = -47,
    TRDP_XML_PARSER_ERR     = -48,
    TRDP_INUSE_ERR          = -49,
    TRDP_MARSHALLING_ERR    = -50,
    TRDP_UNKNOWN_ERR        = -99
} TRDP_ERR_T;
Constant	Meaning
TRDP_NO_ERR	No error
TRDP_PARAM_ERR	Parameter missing / out of range
TRDP_INIT_ERR	Library not correctly initialised
TRDP_NOINIT_ERR	Invalid or uninitialised handle
TRDP_TIMEOUT_ERR	Timeout occurred
TRDP_NODATA_ERR	Non-blocking mode: no data available
TRDP_SOCK_ERR	Socket error / option not supported
TRDP_IO_ERR	I/O error on socket
TRDP_MEM_ERR	Out of memory
TRDP_SEMA_ERR	Semaphore error
TRDP_QUEUE_ERR	Queue empty
TRDP_QUEUE_FULL_ERR	Queue full
TRDP_MUTEX_ERR	Mutex error
TRDP_THREAD_ERR	Thread creation/join error
TRDP_BLOCK_ERR	Would block in blocking mode
TRDP_INTEGRATION_ERR	Misalignment / endian configuration error
TRDP_NOCONN_ERR	No TCP connection
TRDP_NOSESSION_ERR	MD session not found
TRDP_SESSION_ABORT_ERR	Session aborted
TRDP_NOSUB_ERR	No subscriber found
TRDP_NOPUB_ERR	No publisher found
TRDP_NOLIST_ERR	No listener found
TRDP_CRC_ERR	CRC mismatch
TRDP_WIRE_ERR	Wire/protocol error
TRDP_TOPO_ERR	Invalid topology count
TRDP_COMID_ERR	Unknown ComId
TRDP_STATE_ERR	API called in invalid state
TRDP_APP_TIMEOUT_ERR	Application-level timeout
TRDP_APP_REPLYTO_ERR	Application reply timeout
TRDP_APP_CONFIRMTO_ERR	Application confirm timeout
TRDP_REPLYTO_ERR	Protocol reply timeout
TRDP_CONFIRMTO_ERR	Protocol confirm timeout
TRDP_REQCONFIRMTO_ERR	Confirm timeout (request sender)
TRDP_PACKET_ERR	Incomplete MD packet
TRDP_UNRESOLVED_ERR	DNR: address could not be resolved
TRDP_XML_PARSER_ERR	XML parsing error (tau_xml subsystem)
TRDP_INUSE_ERR	Resource still in use
TRDP_MARSHALLING_ERR	Source size exceeded / dataset mismatch
TRDP_UNKNOWN_ERR	Unknown/unclassified error

6. Reply Status — TRDP_REPLY_STATUS_T
c
Copy code
typedef enum
{
    TRDP_REPLY_OK                   = 0,
    TRDP_REPLY_RESERVED01           = -1,
    TRDP_REPLY_SESSION_ABORT        = -2,
    TRDP_REPLY_NO_REPLIER_INST      = -3,
    TRDP_REPLY_NO_MEM_REPL          = -4,
    TRDP_REPLY_NO_MEM_LOCAL         = -5,
    TRDP_REPLY_NO_REPLY             = -6,
    TRDP_REPLY_NOT_ALL_REPLIES      = -7,
    TRDP_REPLY_NO_CONFIRM           = -8,
    TRDP_REPLY_RESERVED02           = -9,
    TRDP_REPLY_SENDING_FAILED       = -10,
    TRDP_REPLY_UNSPECIFIED_ERROR    = -99
} TRDP_REPLY_STATUS_T;
Used in MD callbacks to report reply state (missing replies, no confirm, send failed, etc.).

7. Packet Flags and Timeouts
7.1 TRDP_FLAGS_T and Flags
c
Copy code
typedef UINT8 TRDP_FLAGS_T;
typedef UINT16 TRDP_MSG_T;
Packet flags:

c
Copy code
#define TRDP_FLAGS_DEFAULT      0u
#define TRDP_FLAGS_NONE         0x01u
#define TRDP_FLAGS_MARSHALL     0x02u
#define TRDP_FLAGS_CALLBACK     0x04u
#define TRDP_FLAGS_TCP          0x08u
#define TRDP_FLAGS_FORCE_CB     0x10u

#define TRDP_FLAGS_TSN          0x20u
#define TRDP_FLAGS_TSN_SDT      0x40u
#define TRDP_FLAGS_TSN_MSDT     0x80u
TRDP_FLAGS_MARSHALL: Use automatic marshalling/unmarshalling.

TRDP_FLAGS_CALLBACK: Deliver packet via callback instead of polled API.

TRDP_FLAGS_TCP: Use TCP for MD instead of UDP.

TRDP_FLAGS_FORCE_CB: Always invoke callback even if no data change.

TRDP_FLAGS_TSN*: TSN/SDT specific PD options.

7.2 Default Timeout and Timer Granularity
c
Copy code
#define TRDP_INFINITE_TIMEOUT       0xffffffffu
#define TRDP_DEFAULT_PD_TIMEOUT     100000u   /* 100 ms */
Timer granularity (µs):

c
Copy code
#ifdef HIGH_PERF_INDEXED
#   define TRDP_TIMER_GRANULARITY   500u      /* 0.5 ms */
#else
#   define TRDP_TIMER_GRANULARITY   5000u     /* 5 ms   */
#endif
8. Redundancy and Timeout Behaviour
8.1 Redundancy State
c
Copy code
typedef enum
{
    TRDP_RED_FOLLOWER   = 0u,
    TRDP_RED_LEADER     = 1u
} TRDP_RED_STATE_T;
8.2 Timeout Behaviour for PD
c
Copy code
typedef enum
{
    TRDP_TO_DEFAULT         = 0u,
    TRDP_TO_SET_TO_ZERO     = 1u,
    TRDP_TO_KEEP_LAST_VALUE = 2u
} TRDP_TO_BEHAVIOR_T;
9. PD and MD Information Structures
9.1 TRDP_PD_INFO_T — PD Telegram Info
c
Copy code
typedef struct
{
    TRDP_IP_ADDR_T      srcIpAddr;
    TRDP_IP_ADDR_T      destIpAddr;
    UINT32              seqCount;
    UINT16              protVersion;
    TRDP_MSG_T          msgType;
    UINT32              comId;
    UINT32              etbTopoCnt;
    UINT32              opTrnTopoCnt;
    UINT32              replyComId;
    TRDP_IP_ADDR_T      replyIpAddr;
    void                *pUserRef;
    TRDP_ERR_T          resultCode;
    TRDP_URI_HOST_T     srcHostURI;
    TRDP_URI_HOST_T     destHostURI;
    TRDP_TO_BEHAVIOR_T  toBehavior;
    UINT32              serviceId;
} TRDP_PD_INFO_T;
Used mainly in PD callback function to describe the incoming PD telegram.

9.2 TRDP_MD_INFO_T — MD Telegram Info
c
Copy code
typedef struct
{
    TRDP_IP_ADDR_T      srcIpAddr;
    TRDP_IP_ADDR_T      destIpAddr;
    UINT32              seqCount;
    UINT16              protVersion;
    TRDP_MSG_T          msgType;
    UINT32              comId;
    UINT32              etbTopoCnt;
    UINT32              opTrnTopoCnt;
    BOOL8               aboutToDie;
    UINT32              numRepliesQuery;
    UINT32              numConfirmSent;
    UINT32              numConfirmTimeout;
    UINT16              userStatus;
    TRDP_REPLY_STATUS_T replyStatus;
    TRDP_UUID_T         sessionId;
    UINT32              replyTimeout;
    TRDP_URI_USER_T     srcUserURI;
    TRDP_URI_HOST_T     srcHostURI;
    TRDP_URI_USER_T     destUserURI;
    TRDP_URI_HOST_T     destHostURI;
    UINT32              numExpReplies;
    UINT32              numReplies;
    void                *pUserRef;
    TRDP_ERR_T          resultCode;
} TRDP_MD_INFO_T;
Provided to MD callbacks (TRDP_MD_CALLBACK_T) so the application can distinguish requests, responses, timeouts, etc.

10. Communication Parameters
10.1 Send/Com Parameters
c
Copy code
typedef struct
{
    UINT8   qos;      /* Quality of service (2 recommended) */
    UINT8   ttl;      /* Time to live (64 recommended)      */
    UINT8   retries;  /* MD retries                         */
    BOOL8   tsn;      /* TRUE: TSN socket                   */
    UINT16  vlan;     /* VLAN Id                            */
} TRDP_COM_PARAM_T, TRDP_SEND_PARAM_T;
Used for both:

PD send parameters (QoS, TTL, TSN, VLAN)

MD send parameters (QoS, TTL, retries, VLAN)

11. Dataset Description Types
11.1 Data Type Enum
c
Copy code
typedef enum
{
    TRDP_INVALID    = 0u,
    TRDP_BITSET8    = 1u,
    TRDP_CHAR8      = 2u,
    TRDP_UTF16      = 3u,
    TRDP_INT8       = 4u,
    TRDP_INT16      = 5u,
    TRDP_INT32      = 6u,
    TRDP_INT64      = 7u,
    TRDP_UINT8      = 8u,
    TRDP_UINT16     = 9u,
    TRDP_UINT32     = 10u,
    TRDP_UINT64     = 11u,
    TRDP_REAL32     = 12u,
    TRDP_REAL64     = 13u,
    TRDP_TIMEDATE32 = 14u,
    TRDP_TIMEDATE48 = 15u,
    TRDP_TIMEDATE64 = 16u,
    TRDP_TYPE_MAX   = 30u
} TRDP_DATA_TYPE_T;
Convenience aliases:

c
Copy code
#define TRDP_BOOL8       TRDP_BITSET8
#define TRDP_ANTIVALENT8 TRDP_BITSET8
11.2 Dataset Element & Dataset
c
Copy code
struct TRDP_DATASET;

typedef struct
{
    UINT32              type;
    UINT32              size;
    CHAR8               *name;
    CHAR8               *unit;
    REAL32              scale;
    INT32               offset;
    struct TRDP_DATASET *pCachedDS;
} TRDP_DATASET_ELEMENT_T;

typedef struct TRDP_DATASET
{
    UINT32                  id;
    UINT16                  reserved1;
    UINT16                  numElement;
    TRDP_EXTRA_LABEL_T      name;
    TRDP_DATASET_ELEMENT_T  pElement[];
} TRDP_DATASET_T;
A dataset is a typed, possibly nested structure used for PD/MD payloads.

pCachedDS is used internally to speed up marshalling.

11.3 ComID to Dataset Mapping
c
Copy code
typedef struct
{
    UINT32  comId;
    UINT32  datasetId;
} TRDP_COMID_DSID_MAP_T;

typedef TRDP_DATASET_T *pTRDP_DATASET_T;
typedef pTRDP_DATASET_T *apTRDP_DATASET_T;
typedef apTRDP_DATASET_T *papTRDP_DATASET_T;
Mapping is provided by XML configuration and used by marshalling utilities.

12. Statistics Types
Used by tlc_getStatistics(), tlc_getSubsStatistics(), tlc_getPubStatistics(), etc.

12.1 Request Wrapper
c
Copy code
typedef struct
{
    UINT32 comId;
} GNU_PACKED TRDP_STATISTICS_REQUEST_T;
12.2 Memory Statistics
c
Copy code
typedef VOS_MEM_STATISTICS_T TRDP_MEM_STATISTICS_T;
12.3 PD and MD Statistics
c
Copy code
typedef struct
{
    UINT32  defQos;
    UINT32  defTtl;
    UINT32  defTimeout;
    UINT32  numSubs;
    UINT32  numPub;
    UINT32  numRcv;
    UINT32  numCrcErr;
    UINT32  numProtErr;
    UINT32  numTopoErr;
    UINT32  numNoSubs;
    UINT32  numNoPub;
    UINT32  numTimeout;
    UINT32  numSend;
    UINT32  numMissed;
} GNU_PACKED TRDP_PD_STATISTICS_T;

typedef struct
{
    UINT32  defQos;
    UINT32  defTtl;
    UINT32  defReplyTimeout;
    UINT32  defConfirmTimeout;
    UINT32  numList;
    UINT32  numRcv;
    UINT32  numCrcErr;
    UINT32  numProtErr;
    UINT32  numTopoErr;
    UINT32  numNoListener;
    UINT32  numReplyTimeout;
    UINT32  numConfirmTimeout;
    UINT32  numSend;
} GNU_PACKED TRDP_MD_STATISTICS_T;
12.4 Global Statistics
c
Copy code
typedef struct
{
    UINT32                  version;
    UINT64                  timeStamp;
    UINT32                  upTime;
    UINT32                  statisticTime;
    TRDP_NET_LABEL_T        hostName;
    TRDP_NET_LABEL_T        leaderName;
    TRDP_IP_ADDR_T          ownIpAddr;
    TRDP_IP_ADDR_T          leaderIpAddr;
    UINT32                  processPrio;
    UINT32                  processCycle;
    UINT32                  numJoin;
    UINT32                  numRed;
    TRDP_MEM_STATISTICS_T   mem;
    TRDP_PD_STATISTICS_T    pd;
    TRDP_MD_STATISTICS_T    udpMd;
    TRDP_MD_STATISTICS_T    tcpMd;
} GNU_PACKED TRDP_STATISTICS_T;
12.5 Per-Subscription / Per-Publisher / Listener / Redundancy
c
Copy code
typedef struct
{
    UINT32          comId;
    TRDP_IP_ADDR_T  joinedAddr;
    TRDP_IP_ADDR_T  filterAddr;
    UINT32          callBack;
    UINT32          userRef;
    UINT32          timeout;
    UINT32          status;   /* TRDP_ERR_T */
    UINT32          toBehav;
    UINT32          numRecv;
    UINT32          numMissed;
} GNU_PACKED TRDP_SUBS_STATISTICS_T;

typedef struct
{
    UINT32          comId;
    TRDP_IP_ADDR_T  destAddr;
    UINT32          cycle;
    UINT32          redId;
    UINT32          redState;
    UINT32          numPut;
    UINT32          numSend;
} GNU_PACKED TRDP_PUB_STATISTICS_T;

typedef struct
{
    UINT32          comId;
    CHAR8           uri[32];
    TRDP_IP_ADDR_T  joinedAddr;
    UINT32          callBack;
    UINT32          queue;
    UINT32          userRef;
    UINT32          numRecv;
} GNU_PACKED TRDP_LIST_STATISTICS_T;

typedef struct
{
    UINT32  id;
    UINT32  state;
} GNU_PACKED TRDP_RED_STATISTICS_T;
13. Session and Handle Types
c
Copy code
typedef struct TRDP_SESSION *TRDP_APP_SESSION_T;
typedef struct PD_ELE       *TRDP_PUB_T;
typedef struct PD_ELE       *TRDP_SUB_T;
typedef struct MD_LIS_ELE   *TRDP_LIS_T;
Opaque handles used by the TRDP API (tlc_*, tlp_*, tlm_*).

14. Logging and Debug
c
Copy code
typedef VOS_PRINT_DBG_T TRDP_PRINT_DBG_T;
typedef VOS_LOG_T       TRDP_LOG_T;
TRDP uses VOS logging abstractions. The actual print callback is provided to tlc_init().

15. Marshalling Configuration and Function Types
15.1 Function Pointer Types
c
Copy code
typedef TRDP_ERR_T (*TRDP_MARSHALL_T)(
    void            *pRefCon,
    UINT32          comId,
    const UINT8     *pSrc,
    UINT32          srcSize,
    UINT8           *pDst,
    UINT32          *pDstSize,
    TRDP_DATASET_T  **ppCachedDS);

typedef TRDP_ERR_T (*TRDP_UNMARSHALL_T)(
    void            *pRefCon,
    UINT32          comId,
    UINT8           *pSrc,
    UINT32          srcSize,
    UINT8           *pDst,
    UINT32          *pDstSize,
    TRDP_DATASET_T  **ppCachedDS);
15.2 Configuration Struct
c
Copy code
typedef struct
{
    TRDP_MARSHALL_T     pfCbMarshall;
    TRDP_UNMARSHALL_T   pfCbUnmarshall;
    void                *pRefCon;
} TRDP_MARSHALL_CONFIG_T;
Passed into tlc_openSession() or tlc_configSession() to plug in marshaling logic (e.g. tau_marshall).

16. Callback Types and Default Configs
16.1 PD Callback
c
Copy code
typedef void (*TRDP_PD_CALLBACK_T)(
    void                    *pRefCon,
    TRDP_APP_SESSION_T      appHandle,
    const TRDP_PD_INFO_T    *pMsg,
    UINT8                   *pData,
    UINT32                  dataSize);
16.2 PD Default Configuration
c
Copy code
typedef struct
{
    TRDP_PD_CALLBACK_T  pfCbFunction;
    void                *pRefCon;
    TRDP_SEND_PARAM_T   sendParam;
    TRDP_FLAGS_T        flags;
    UINT32              timeout;
    TRDP_TO_BEHAVIOR_T  toBehavior;
    UINT16              port;     /* Default: 17224 */
} TRDP_PD_CONFIG_T;
16.3 MD Callback
c
Copy code
typedef void (*TRDP_MD_CALLBACK_T)(
    void                    *pRefCon,
    TRDP_APP_SESSION_T      appHandle,
    const TRDP_MD_INFO_T    *pMsg,
    UINT8                   *pData,
    UINT32                  dataSize);
16.4 MD Default Configuration
c
Copy code
typedef struct
{
    TRDP_MD_CALLBACK_T  pfCbFunction;
    void                *pRefCon;
    TRDP_SEND_PARAM_T   sendParam;
    TRDP_FLAGS_T        flags;
    UINT32              replyTimeout;
    UINT32              confirmTimeout;
    UINT32              connectTimeout;
    UINT32              sendingTimeout;
    UINT16              udpPort;        /* default 17225 */
    UINT16              tcpPort;        /* default 17225 */
    UINT32              maxNumSessions;
} TRDP_MD_CONFIG_T;
17. Memory Configuration
c
Copy code
typedef struct
{
    UINT8   *p;
    UINT32  size;
    UINT32  prealloc[VOS_MEM_NBLOCKSIZES];
} TRDP_MEM_CONFIG_T;
Used at tlc_init() to configure memory pools and pre-fragmentation strategies.

18. Process Options and Configuration
18.1 TRDP_OPTION_T and Flags
c
Copy code
#define TRDP_OPTION_NONE                0u
#define TRDP_OPTION_BLOCK               0x01u
#define TRDP_OPTION_TRAFFIC_SHAPING     0x02u
#define TRDP_OPTION_NO_REUSE_ADDR       0x04u
#define TRDP_OPTION_NO_MC_LOOP_BACK     0x08u
#define TRDP_OPTION_NO_UDP_CHK          0x10u
#define TRDP_OPTION_WAIT_FOR_DNR        0x20u
#define TRDP_OPTION_NO_PD_STATS         0x40u
#define TRDP_OPTION_DEFAULT_CONFIG      0x80u

typedef UINT8 TRDP_OPTION_T;
18.2 Process Configuration
c
Copy code
typedef struct
{
    TRDP_LABEL_T        hostName;
    TRDP_LABEL_T        leaderName;
    TRDP_LABEL_T        type;
    UINT32              cycleTime;   /* main loop period in µs */
    UINT32              priority;   /* 0..255 (0 = default)   */
    TRDP_OPTION_T       options;
} TRDP_PROCESS_CONFIG_T;
Passed to tlc_openSession() or tau_readXmlInterfaceConfig().

19. Index Table Configuration
c
Copy code
typedef struct
{
    UINT32  maxNoOfLowCatSubscriptions;
    UINT32  maxNoOfMidCatSubscriptions;
    UINT32  maxNoOfHighCatSubscriptions;
    UINT32  maxNoOfLowCatPublishers;
    UINT32  maxDepthOfLowCatPublishers;
    UINT32  maxNoOfMidCatPublishers;
    UINT32  maxDepthOfMidCatPublishers;
    UINT32  maxNoOfHighCatPublishers;
    UINT32  maxDepthOfHighCatPublishers;
    UINT32  maxNoOfExtPublishers;
} TRDP_IDX_TABLE_T;
Used in tlc_presetIndexSession() to pre-allocate time-slot tables for publishers and subscribers.

20. Usage Notes
trdp_types.h is a foundational header: types defined here are used in almost every TRDP API function.

Application code typically does not include this directly; it is included indirectly through public interface headers like trdp_if_light.h.

For MISRA-C/C++ compatibility, types are fixed-width and all enums are fully defined with explicit values.

When writing new APIs in your simulator, prefer using:

TRDP_ERR_T for return codes

TRDP_IP_ADDR_T for IP addresses

TRDP_*_CONFIG_T for configuration structures

TRDP_PD_INFO_T / TRDP_MD_INFO_T for representing telegram metadata

