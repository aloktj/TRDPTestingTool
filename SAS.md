System Architecture Specification (SAS)
TRDP Simulator – Terminal-Based TRDP Test & Diagnostic Tool

Version: 1.0
Author: Alok T J / Codex assisted development
Target: Linux / Raspberry Pi
UI: Terminal UI (FTXUI)
Protocol: TRDP PD & MD (IEC-61375-2-3)

1. System Overview

The TRDP Simulator is a modular, event-driven application that simulates, monitors, and interacts with TRDP PD (Process Data) and MD (Message Data) communications for railway and industrial embedded environments.

The system loads IEC-compliant XML configurations to instantiate publishers, subscribers, listeners, and datasets, and provides a live editable view of actual telegram content over a terminal UI.

2. Architectural Goals
Goal	Description
G1	Modular, scalable, and maintainable architecture
G2	Event-driven (no polling) for optimal performance
G3	Multiple simultaneous PD/MD sessions
G4	Real-time data updating and visualization
G5	Clear separation of UI, business logic, and communications
G6	MISRA-C++ aligned design
G7	Fully portable to Linux & Raspberry Pi
3. High-Level Architecture Diagram
┌─────────────────────────────────────────────────────────────┐
│                      Terminal UI (FTXUI)                     │
│  - Dashboard | PD View | MD View | Dataset Editor | Logs     │
└──────────────────────────────┬──────────────────────────────┘
                               │
                               v
┌─────────────────────────────────────────────────────────────┐
│                    TRDP Controller (Core Engine)             │
│   - Event routing  - Session lifecycle  - UI event callbacks │
└──────────────┬──────────────┬──────────────┬────────────────┘
               │              │              │
               v              v              v
      XML Config Loader   Dataset Manager   Logger & Error Mapper
               │
               v
┌────────────────────────────────────────────────────────────────┐
│              TRDP Stack Layer (TCNOpen TRDP Light)             │
│  PD Publisher | PD Subscriber | MD Manager | Marshalling Layer │
└────────────────────────────────────────────────────────────────┘
                               │
                               v
                       Linux Network Stack

4. Layer Responsibilities
4.1 Presentation Layer (TUI Layer)
Component	Responsibilities
TUIManager	Render screens, input handling, navigation
Panels	Dashboard / PD View / MD View / Dataset Editor / Log Viewer
Event Bus	Dispatch UI commands to core

Uses FTXUI reactive components (container / component model).

4.2 Application Core Layer
Module	Responsibility
TRDPController	Central orchestrator; owns TRDP sessions
XMLConfigLoader	Parses IEC XML & maps devices, telegrams, datasets
PDPubManager	Creates PD publishers, manages update cycles
PDSubManager	Subscriptions & callbacks
MDManager	Request / notify / reply / confirm handler
MDFileTransfer	Chunk streaming with ACK handling
DatasetManager	Dataset lookup, marshalling, edit evaluation
Logger & ErrorMapper	Standard logging with TRDP error decoding

Communicates via internal event mechanism:
UIEvent → TRDPController → Subsystem Action → Async Callback → UI Update

4.3 Communication / Protocol Layer
Component	Responsibility
TRDP Stack (TCNOpen)	Socket communication handling
tlc_openSession()	Setup main TRDP session environment
tlp_put()/tlp_subscribe()	PD publishing and subscribing
tlm_request()/tlm_reply()	MD session operations
Marshall/Unmarshall	Dataset binary conversions

No polling: stack uses callback-driven processing.

5. Module Dependency Diagram
+---------------+      +-------------------+
|  UI Manager   | ---> |  TRDP Controller  | ---> TRDP Stack
+---------------+      +-------------------+
         ^                       ^
         |                       |
+-------------------+   +---------------------+
| Dataset Manager   |   | XMLConfigLoader     |
+-------------------+   +---------------------+

6. Runtime Workflow
6.1 Startup Sequence
main.cpp
  └ initialize Logger
  └ launch TRDPController
      └ load configuration (optional)
      └ initialize TRDP stack (tlc_openSession)
      └ start TUIManager (FTXUI loop)

6.2 PD Transmission Cycle
TUI → Start Publisher(com-id)
   → TRDPController → PDPubManager
      → schedule cyclic transmission (timer thread)
          → tlp_put() at cycle time

6.3 PD Subscribe Callback
Receive PD packet
   → TRDP callback
      → DatasetManager unmarshall
         → Update dataset value table
            → UI repaint diff only

6.4 MD Request / Stream
User → MD Send(file)
   → MDManager → MDFileTransfer chunker
      → tlm_request()
         → tlm_reply() listener

7. Concurrency Model
Thread	Purpose
UI main thread	Draw TUI, process input
TRDP callback receiving thread	PD & MD receive handlers
PD transmit thread	Cyclic telegram generation
File transfer worker thread	Chunked streaming

Communication via:

Thread-safe queue

Mutex-protected dataset values

Event dispatcher

8. Data Model Overview
Dataset Structure
Dataset
└─ elements[]
      name
      type (INT8/UINT16/REAL64/UTF8…)
      array-size
      value-buffer
      scale
      offset

PD Instance
telegram-config
  com-id
  dataset-id
  cycle-time
  publisher/subscriber
  UDP multicast address

MD Instance
MD session
  session-id
  type (request/reply/notify)
  payload buffer
  status info

9. Error & Logging Strategy
Source	Handling
TRDP_ERR_T	Mapped to readable string via ErrorMapper
Network fault	UI notification + log
Dataset mismatch	Exception + UI alert
XML parse error	Line/column error message

Log storage levels:

ERROR | WARNING | INFO | DEBUG | TRACE

10. Configuration Management
Component	Description
config/*.xml	IEC-standard TRDP definitions
config/default.yaml	UI + runtime preferences
logs/*.log	History logs
replay/*.trdp	recording for replay
11. Security Considerations
item	description
Unsafe control commands blocked	No real TCMS command execution
User must approve file transfer	Prevents unintended flooding
No elevated privileges needed	Runs as unprivileged process
XML files treated as untrusted	Validation required
12. Build & Deployment
mkdir build && cd build
cmake ..
make -j4
./trdp-simulator

13. Future Extensions
Feature	Reason
MQTT remote control	Multi-station simulation
Graphical optional UI	Engineering version
Train topology viewer	Visual network mapping
Prometheus exporter	Monitoring dashboards
