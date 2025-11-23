Below is the clean **Markdown text** version for `tau_so_if.h` that you can **copy, paste, and save as**:

### **`TRDP_API_tau_so_if.md`**

---

````markdown
# TRDP API Documentation — tau_so_if.h

## Overview
`tau_so_if.h` contains the TRDP Service-Oriented Architecture (SOA) interface API.  
It enables applications to **register, update, delete, and enumerate TRDP service definitions** stored in the Service Registry.

This is an extension proposed to the Train Topology Database (TTDB) defined in **IEC 61375-2-3:2017** and is used in TRDP FDF/DbD communication environments.

---

## Included Header Files
| Header | Purpose |
|--------|---------|
| `iec61375-2-3.h` | SOA & TTDB related structures and definitions |
| `trdp_serviceRegistry.h` | Service Registry Manager types and API |

---

## API Functions

### `tau_addService()`
Registers a new service in the shared service registry.

```c
EXT_DECL TRDP_ERR_T tau_addService(
    TRDP_APP_SESSION_T  appHandle,
    SRM_SERVICE_INFO_T  *pServiceToAdd,
    BOOL8               waitForCompletion);
````

| Parameter           | Direction | Description                                        |
| ------------------- | --------- | -------------------------------------------------- |
| `appHandle`         | IN        | TRDP application session handle                    |
| `pServiceToAdd`     | IN        | Service description to add                         |
| `waitForCompletion` | IN        | TRUE = synchronous, FALSE = asynchronous operation |

| Returns            | Meaning                        |
| ------------------ | ------------------------------ |
| `TRDP_NO_ERR`      | success                        |
| `TRDP_PARAM_ERR`   | invalid parameters             |
| `TRDP_TIMEOUT_ERR` | timeout waiting for completion |
| `TRDP_STATE_ERR`   | registry not available         |

---

### `tau_delService()`

Deletes a previously registered service.

```c
EXT_DECL TRDP_ERR_T tau_delService(
    TRDP_APP_SESSION_T  appHandle,
    SRM_SERVICE_INFO_T  *pServiceToAdd,
    BOOL8               waitForCompletion);
```

---

### `tau_updService()`

Updates a service entry.

```c
EXT_DECL TRDP_ERR_T tau_updService(
    TRDP_APP_SESSION_T  appHandle,
    SRM_SERVICE_INFO_T  *pServiceToAdd,
    BOOL8               waitForCompletion);
```

| Typical use case                                                              |
| ----------------------------------------------------------------------------- |
| Updating TTL, instance count, event list, usage visibility, redundancy status |

---

### `tau_getServicesList()`

Fetches list of registered services. Supports filtering.

```c
EXT_DECL TRDP_ERR_T tau_getServicesList(
    TRDP_APP_SESSION_T      appHandle,
    SRM_SERVICE_ENTRIES_T   **ppServicesToAdd,
    UINT32                  *pNoOfServices,
    SRM_SERVICE_ENTRIES_T   *pFilterEntry);
```

| Parameter         | Direction | Description                                |
| ----------------- | --------- | ------------------------------------------ |
| `ppServicesToAdd` | OUT       | Pointer to buffer containing services list |
| `pNoOfServices`   | OUT       | Number of services found                   |
| `pFilterEntry`    | IN        | Filter object (NULL = no filter)           |

Call `tau_freeServicesList()` to free returned list.

---

### `tau_freeServicesList()`

```c
EXT_DECL void tau_freeServicesList(
    SRM_SERVICE_ENTRIES_T *pServicesListBuffer);
```

| Purpose                                           |
| ------------------------------------------------- |
| Releases memory allocated for service enumeration |

---

## Typical Workflow Example

```c
SRM_SERVICE_INFO_T service = { /* fill service structure */ };
TRDP_ERR_T err;

err = tau_addService(appHandle, &service, TRUE);

SRM_SERVICE_ENTRIES_T *list = NULL;
UINT32 count = 0;
err = tau_getServicesList(appHandle, &list, &count, NULL);

// Print services...

tau_freeServicesList(list);
```

---

## Usage in TRDP Simulator

| Capability Enabled                    | Explanation                               |
| ------------------------------------- | ----------------------------------------- |
| Register simulator as TRDP SOA device | Expose supported PD/MD datasets           |
| Discover TRDP services from endpoints | Testing distributed communication         |
| Modify behavior dynamically           | Change MD confirm policy, TTL, redundancy |
| Service Browser                       | UI view listing available services        |

---

## Return Values

| Enum               | Meaning                          |
| ------------------ | -------------------------------- |
| `TRDP_NO_ERR`      | operation completed successfully |
| `TRDP_TIMEOUT_ERR` | operation not completed in time  |
| `TRDP_PARAM_ERR`   | invalid or NULL pointer          |
| `TRDP_STATE_ERR`   | service registry unavailable     |
| `TRDP_MEM_ERR`     | allocation failure               |

---

## Notes

* Must be used within an active TRDP session created via `tlc_openSession()`
* Calls may block if `waitForCompletion = TRUE`
* Used heavily for TRDP SOA discovery and management

---

## License

MPL 2.0
© NewTec GmbH, 2019

```

---

### Next header file?
Would you like documentation next for:

1. `trdp_serviceRegistry.h`
2. `iec61375-2-3.h`
3. `tau_evt.h`
4. **Batch all remaining**

Reply with option number or: **"batch generate"** 🚀
```
