````markdown
# TRDP Utility Types — Train Topology (`tau_tti_types.h`)

## 1. Purpose

The header `tau_tti_types.h` defines **type structures** for **Train Topology Information (TTI)** according to **IEC 61375-2-3 / -2-5**, used together with the TTI API in `tau_tti.h`.

These types cover:

- Static consist and vehicle information  
- Operational train directory and state  
- Train directory and train network directory  
- TCN ETB control safety/trailer structures  

They are used by the TTI utilities to interpret and expose train topology information over TRDP.

---

## 2. Includes and Basic Limits

```c
#include "trdp_types.h"
````

### 2.1 Constants

```c
#define TRDP_MAX_CST_CNT   63u      /* Max number of consists per train        */
#define TRDP_MAX_VEH_CNT   63u      /* Max number of vehicles per train        */
#define TRDP_MAX_PROP_LEN  32768u   /* Max length of property information (#378) */
```

---

## 3. Common Version Types

### 3.1 `TRDP_SHORT_VERSION_T`

**Purpose**
Compact version information (major/minor) for communication buffers and topology structures.

**Definition**

```c
typedef struct
{
    UINT8 ver;   /* Major version: incremented for incompatible changes   */
    UINT8 rel;   /* Minor release: incremented for compatible changes     */
} TRDP_SHORT_VERSION_T;
```

**Used in**

* `TRDP_PROP_T`
* `TRDP_CONSIST_INFO_T`
* `TRDP_CONSIST_INFO_LIST_T`
* `TRDP_ETB_CTRL_VDP_T`
* `TRDP_CSTINFOCTRL_T`
* `TRDP_TRAIN_DIR_T`
* `TRDP_OP_TRAIN_DIR_STATE_T`
* `TRDP_OP_TRAIN_DIR_T`

---

## 4. Static Train / Consist / Vehicle Information

### 4.1 ETB Information — `TRDP_ETB_INFO_T`

**Purpose**
Describes how many Consist Network (CN) nodes of a consist are connected to a specific ETB (Train Backbone).

**Definition**

```c
typedef struct
{
    UINT8  etbId;     /* ETB ID: 0..3                                     */
    UINT8  cnCnt;     /* Number of CNs on this ETB (1..16)                */
    UINT16 reserved01;/* Reserved (= 0)                                    */
} TRDP_ETB_INFO_T;
```

---

### 4.2 Closed Train Consist Info — `TRDP_CLTR_CST_INFO_T`

**Purpose**
Contains information for **closed train consists** within a closed train configuration.

**Definition**

```c
typedef struct
{
    TRDP_UUID_T cltrCstUUID;  /* Closed train consist UUID                         */
    UINT8       cltrCstOrient;/* Orientation: 01B same as closed train, 10B inverse */
    UINT8       cltrCstNo;    /* Consist sequence number in closed train (1..32)   */
    UINT16      reserved01;   /* Reserved (= 0)                                     */
} TRDP_CLTR_CST_INFO_T;
```

---

### 4.3 Application Properties — `TRDP_PROP_T`

**Purpose**
Application-defined **property blob** used for consist and vehicle properties. Length and internal layout are application-specific.

**Definition**

```c
typedef struct
{
    TRDP_SHORT_VERSION_T ver;   /* Properties version (application-defined)        */
    UINT16               len;   /* Length in octets (0..32768), multiple of 4      */
    UINT8                prop[];/* Property data (application-defined payload)     */
} TRDP_PROP_T;
```

* `prop[]` is a **flexible array** (Ticket #378); actual size is `len` bytes.

---

### 4.4 Function / Device Info — `TRDP_FUNCTION_INFO_T`

**Purpose**
Describes a **function device** or **function group** in a consist (logical function, host, or group of hosts).

**Definition**

```c
typedef struct
{
    TRDP_NET_LABEL_T fctName;   /* Function device/group label                     */
    UINT16           fctId;     /* Function ID (1..16383, device or group)        */
    BOOL8            grp;       /* TRUE: function group (IP multicast group)      */
    UINT8            reserved01;
    UINT8            cstVehNo;  /* Vehicle number in consist (1..16, 0=undefined) */
    UINT8            etbId;     /* ETB ID (0..3)                                   */
    UINT8            cnId;      /* CN ID (0..31) in that consist/ETB              */
    UINT8            reserved02;
} TRDP_FUNCTION_INFO_T;
```

---

### 4.5 Vehicle Info — `TRDP_VEHICLE_INFO_T`

**Purpose**
Represents **static vehicle information** within a consist.

**Definition**

```c
typedef struct
{
    TRDP_NET_LABEL_T vehId;     /* Vehicle identifier (e.g. UIC vehicle ID)       */
    TRDP_NET_LABEL_T vehType;   /* Vehicle type (application-defined)             */
    UINT8            vehOrient; /* Vehicle orientation:
                                   01B = same as consist direction
                                   10B = inverse to consist direction             */
    UINT8            cstVehNo;  /* Sequence number within consist (1..16)         */
    ANTIVALENT8      tractVeh;  /* Traction vehicle flag:
                                   01B = not traction
                                   10B = traction vehicle                         */
    UINT8            reserved01;
    TRDP_PROP_T     *pVehProp;  /* Pointer to static vehicle properties (#378)    */
} TRDP_VEHICLE_INFO_T;
```

---

### 4.6 Consist Info — `TRDP_CONSIST_INFO_T`

**Purpose**
Holds **complete static information about a consist**, including ETB info, vehicle list, function list, and closed train relations.

**Definition**

```c
typedef struct
{
    TRDP_SHORT_VERSION_T  version;        /* Version of ConsistInfo structure                  */
    UINT8                 cstClass;       /* Consist classification:
                                             1 = single consist
                                             2 = closed train
                                             3 = closed train consist                         */
    UINT8                 reserved01;
    TRDP_NET_LABEL_T      cstId;          /* Consist identifier (e.g. UIC ID)                 */
    TRDP_NET_LABEL_T      cstType;        /* Consist type (application-defined)               */
    TRDP_NET_LABEL_T      cstOwner;       /* Consist owner (e.g. "db.de")                     */
    TRDP_UUID_T           cstUUID;        /* Consist UUID                                     */
    UINT32                reserved02;
    TRDP_PROP_T          *pCstProp;       /* Static consist properties (#378)                 */
    UINT16                reserved03;
    UINT16                etbCnt;         /* Number of ETBs (1..4)                            */
    TRDP_ETB_INFO_T      *pEtbInfoList;   /* Ordered ETB info list                            */
    UINT16                reserved04;
    UINT16                vehCnt;         /* Vehicles in consist (1..32)                      */
    TRDP_VEHICLE_INFO_T  *pVehInfoList;   /* Ordered vehicle info list (cstVehNo==1 first)    */
    UINT16                reserved05;
    UINT16                fctCnt;         /* Number of functions (0..1024)                    */
    TRDP_FUNCTION_INFO_T *pFctInfoList;   /* Function info list (sorted by fctName)           */
    UINT16                reserved06;
    UINT16                cltrCstCnt;     /* Number of original consists in closed train (0..32) */
    TRDP_CLTR_CST_INFO_T *pCltrCstInfoList;/* Closed-train composition list                  */
    UINT32                cstTopoCnt;     /* Consist topology counter (CRC-like)              */
} TRDP_CONSIST_INFO_T;
```

---

### 4.7 Consist Info List — `TRDP_CONSIST_INFO_LIST_T`

**Purpose**
Represents a static **collection of consist info structures** for a train.

**Definition**

```c
typedef struct
{
    TRDP_SHORT_VERSION_T  version;         /* Version of ConsistInfoList (mainVersion = 1)     */
    UINT16                cstInfoCnt;      /* Number of consists in train (1..63)              */
    TRDP_CONSIST_INFO_T   cstInfoList[TRDP_MAX_CST_CNT];
                                           /* Consist info array (up to 63 entries)           */
} TRDP_CONSIST_INFO_LIST_T;
```

---

## 5. ETB Control & Consist Topology Control

### 5.1 ETB Control Trailer — `TRDP_ETB_CTRL_VDP_T`

**Purpose**
Safety trailer for ETB control telegrams (vital part).

**Definition**

```c
typedef struct
{
    UINT32               reserved01;      /* Reserved (=0)                                    */
    UINT16               reserved02;      /* Reserved (=0)                                    */
    TRDP_SHORT_VERSION_T userDataVersion; /* Vital ETBCTRL version (main=1, sub=0)           */
    UINT32               safeSeqCount;    /* Safe sequence counter                            */
    UINT32               safetyCode;      /* Safety checksum                                  */
} TRDP_ETB_CTRL_VDP_T;
```

---

### 5.2 Consist Entry — `TRDP_CONSIST_T`

**Purpose**
Represents **one consist** in the train / CSTINFO control / train directory context.

**Definition**

```c
typedef struct
{
    TRDP_UUID_T cstUUID;     /* Consist UUID (0 if not available)                 */
    UINT32      cstTopoCnt;  /* Consist topology counter (from CSTINFO, 0 if none)*/
    UINT8       trnCstNo;    /* Train consist sequence number (1..63)             */
    UINT8       cstOrient;   /* Consist orientation relative to train direction:
                                01B = same
                                10B = inverse                                     */
    UINT16      reserved01;
} TRDP_CONSIST_T;
```

---

### 5.3 CSTINFO Control Telegram — `TRDP_CSTINFOCTRL_T`

**Purpose**
Control telegram structure for CSTINFO, describing which consists should send CSTINFO and their relation to the train.

**Definition**

```c
typedef struct
{
    TRDP_SHORT_VERSION_T version;   /* Version of CSTINFO control (main=1)           */
    UINT8                trnCstNo;  /* Train consist number / control type:
                                      0 = without trnTopoCnt tracking
                                      1 = with trnTopoCnt tracking                   */
    UINT8                cstCnt;    /* Number of consists in train (1..63)          */
    TRDP_CONSIST_T       cstList[TRDP_MAX_CST_CNT];
                                    /* Consist list (ordered if trnCstNo > 0)       */
    UINT32               trnTopoCnt;/* Train topology counter (0 if no tracking)    */
    TRDP_ETB_CTRL_VDP_T  safetyTrail;
} TRDP_CSTINFOCTRL_T;
```

---

## 6. Train Directory (Static)

### 6.1 Train Directory — `TRDP_TRAIN_DIR_T`

**Purpose**
Represents the **static train directory** (TRAIN_DIRECTORY), describing the composition of the train.

**Definition**

```c
typedef struct
{
    TRDP_SHORT_VERSION_T version;  /* TrainDirectory version (main=1)                */
    UINT8                etbId;    /* ETB ID bitmask:
                                     bit0: ETB0 (operational)
                                     bit1: ETB1 (multimedia)
                                     bit2: ETB2
                                     bit3: ETB3                                       */
    UINT8                cstCnt;   /* Number of consists (1..63)                     */
    TRDP_CONSIST_T       cstList[TRDP_MAX_CST_CNT];
                                   /* Consist list ordered by trnCstNo==1..          */
    UINT32               trnTopoCnt;/* Train topology counter                         */
} TRDP_TRAIN_DIR_T;
```

---

## 7. Operational Train Directory

### 7.1 Operational Vehicle — `TRDP_OP_VEHICLE_T`

**Purpose**
Represents a **vehicle in the operational view** of the train (direction, leading status, mapping to consist).

**Definition**

```c
typedef struct
{
    TRDP_NET_LABEL_T vehId;     /* Unique vehicle identifier (e.g. UIC)            */
    UINT8            opVehNo;   /* Operational vehicle number in train (1..63)     */
    ANTIVALENT8      isLead;    /* Leading vehicle flag                            */
    UINT8            leadDir;   /* Leading direction:
                                   0 = not relevant
                                   1 = direction 1
                                   2 = direction 2                                  */
    UINT8            trnVehNo;  /* Vehicle index in ETB reference direction (1..63, 0=corrected) */
    UINT8            vehOrient; /* Vehicle orientation relative to op train dir:
                                   00B = unknown
                                   01B = same
                                   10B = inverse                                    */
    UINT8            ownOpCstNo;/* Operational consist number this vehicle belongs to */
    UINT8            reserved01;
    UINT8            reserved02;
} TRDP_OP_VEHICLE_T;
```

---

### 7.2 Operational Consist — `TRDP_OP_CONSIST_T`

**Purpose**
Represents a **consist in the operational view**.

**Definition**

```c
typedef struct
{
    TRDP_UUID_T cstUUID;      /* Reference to static consist info (0 if none)     */
    UINT8       opCstNo;      /* Operational consist number (1..63)               */
    UINT8       opCstOrient;  /* Orientation relative to op train direction:
                                 00B = unknown (corrected)
                                 01B = same
                                 10B = inverse                                     */
    UINT8       trnCstNo;     /* Train consist number (1..63, 0=corrected)        */
    UINT8       reserved01;
} TRDP_OP_CONSIST_T;
```

---

### 7.3 Operational Train Directory State — `TRDP_OP_TRAIN_DIR_STATE_T`

**Purpose**
Contains **state and status information** of the operational train directory.

**Definition**

```c
typedef struct
{
    TRDP_SHORT_VERSION_T version;       /* TrainDirectoryState version (main=1)        */
    UINT8                reserved01;
    UINT8                reserved02;
    UINT8                etbId;         /* ETB ID (0..3 for ETB0..ETB3)                */
    UINT8                trnDirState;   /* TTDB status:
                                           01B = unconfirmed
                                           10B = confirmed                             */
    UINT8                opTrnDirState; /* Operational train directory status:
                                           01B = invalid
                                           10B = valid
                                           100B = shared                               */
    UINT8                reserved03;
    TRDP_NET_LABEL_T     trnId;         /* Train identifier (e.g. "ICE75")             */
    TRDP_NET_LABEL_T     trnOperator;   /* Train operator (e.g. "db.de")              */
    UINT32               opTrnTopoCnt;  /* Operational train topology counter (0 if invalid) */
    UINT32               crc;           /* CRC over record (seed FFFFFFFFh)           */
} TRDP_OP_TRAIN_DIR_STATE_T;
```

---

### 7.4 Operational Train Directory — `TRDP_OP_TRAIN_DIR_T`

**Purpose**
Describes the **operational view of the entire train** (consists + vehicles, orientation, and topology).

**Definition**

```c
typedef struct
{
    TRDP_SHORT_VERSION_T version;       /* Version of Train info structure             */
    UINT8                etbId;         /* ETB ID (0..3 for ETB0..ETB3)                */
    UINT8                opTrnOrient;   /* Operational train orientation:
                                           00B = unknown
                                           01B = same as train direction
                                           10B = inverse                                */
    UINT8                reserved01;
    UINT8                reserved02;
    UINT8                reserved03;
    UINT8                opCstCnt;      /* Number of operational consists (1..63)      */
    TRDP_OP_CONSIST_T    opCstList[TRDP_MAX_CST_CNT];
                                        /* Operational consist list (size opCstCnt)    */
    UINT8                reserved04;
    UINT8                reserved05;
    UINT8                reserved06;
    UINT8                opVehCnt;      /* Number of operational vehicles (1..63)      */
    TRDP_OP_VEHICLE_T    opVehList[TRDP_MAX_VEH_CNT];
                                        /* Operational vehicle list (size opVehCnt)    */
    UINT32               opTrnTopoCnt;  /* Operational train topology counter          */
} TRDP_OP_TRAIN_DIR_T;
```

---

### 7.5 Operational Train Directory Status Info — `TRDP_OP_TRAIN_DIR_STATUS_INFO_T`

**Purpose**
Convenience structure combining the **operational directory state**, relevant counters, and safety trailer, as used in PD 100.

**Definition**

```c
typedef struct
{
    TRDP_OP_TRAIN_DIR_STATE_T state;     /* Operational state (as above)           */
    UINT32                    etbTopoCnt;/* ETB topology counter                   */
    UINT8                     ownOpCstNo;/* Own operational consist number         */
    UINT8                     ownTrnCstNo;/* Own train consist number              */
    UINT16                    reserved02;
    TRDP_ETB_CTRL_VDP_T       safetyTrail;
} TRDP_OP_TRAIN_DIR_STATUS_INFO_T;
```

---

## 8. Train Network Directory

### 8.1 Train Network Directory Entry — `TRDP_TRAIN_NET_DIR_ENTRY_T`

**Purpose**
Maps a consist UUID to **network properties** (orientation, ETBN, subnet, CN).

**Definition**

```c
typedef struct
{
    TRDP_UUID_T cstUUID;   /* Unique consist identifier                         */
    UINT32      cstNetProp;/* Network properties (bitfield):
                              bit 0..1: consist orientation
                              bit 8..13: ETBN Id
                              bit 16..21: subnet Id
                              bit 24..29: CN Id                                  */
} TRDP_TRAIN_NET_DIR_ENTRY_T;
```

---

### 8.2 Train Network Directory — `TRDP_TRAIN_NET_DIR_T`

**Purpose**
Represents the **train network directory**, listing all consist network mappings and including a topology CRC.

**Definition**

```c
typedef struct
{
    UINT16                    reserved01;
    UINT16                    entryCnt;   /* Number of entries                         */
    TRDP_TRAIN_NET_DIR_ENTRY_T trnNetDir[TRDP_MAX_CST_CNT];
                                          /* Directory entries                          */
    UINT32                    etbTopoCnt; /* Train network directory CRC / topology     */
} TRDP_TRAIN_NET_DIR_T;
```

---

## 9. Complete TTDB Representation

### 9.1 Full TTDB Snapshot — `TRDP_READ_COMPLETE_REPLY_T`

**Purpose**
Represents a **complete TTDB snapshot**, combining all major TTI structures (state, operational, static, and network directories).

**Definition**

```c
typedef struct
{
    TRDP_OP_TRAIN_DIR_STATE_T state;    /* Operational train state                    */
    TRDP_OP_TRAIN_DIR_T       opTrnDir; /* Operational train directory                */
    TRDP_TRAIN_DIR_T          trnDir;   /* Static train directory                     */
    TRDP_TRAIN_NET_DIR_T      trnNetDir;/* Train network directory                    */
} TRDP_READ_COMPLETE_REPLY_T;
```

This structure is often used as a convenient “all-in-one” response type for TTDB queries.

---

```
::contentReference[oaicite:0]{index=0}
```
