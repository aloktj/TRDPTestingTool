# TRDPTestingTool

TRDP Simulator – Terminal Based Application
Linux / Raspberry Pi – Real-Time TRDP PD & MD Simulator with Dataset Editor

This project implements a terminal-based TRDP Simulator that allows users to load an IEC-61375-2-3 compliant XML configuration, manage TRDP PD/MD communications, monitor runtime behavior, edit datasets in real time, and simulate advanced train network use-cases including TSN & file transfers using MD.

The UI is based on FTXUI (no polling, reactive event-driven design).
Backend uses TCNOpen TRDP Light Stack and tau_xml for configuration parsing.

1. High-Level Architecture Overview
+-----------------------------+
|        TUI (FTXUI)         |
|  User interaction layer     |
+--------------+--------------+
               |
               v
+-----------------------------+
|        App Controller       |
|   Owns TRDP runtime state   |
|   Coordinates UI & Engine   |
+--------------+--------------+
               |
               v
+-----------------------------+
|        TRDP Engine          |
|   PD / MD runtime handler   |
|   File transfer system      |
+--------------+--------------+
               |
               v
+-----------------------------+
|   Config / XML Model Layer  |
|   Load XML → internal model |
|   Dataset definitions        |
+--------------+--------------+
               |
               v
+-----------------------------+
|   TRDP Stack & XML parser   |
|   (TRDP Light & tau_xml)    |
+-----------------------------+

2. Module Responsibilities
Module	Description
ui/	All FTXUI screen components. Event-driven user interaction.
trdp/	PD/MD runtime layer wrapping TRDP stack APIs.
config/	XML parsing using tau_xml, conversion to internal model.
model/	Pure C++ configuration & dataset structures.
util/	Logging, error helpers, TRDP string conversion utilities.
docs/	TRDP API reference Markdown files (tau_*.md, trdp_*.md).
3. Directory Structure
src/
  main.cpp
  model/
    sim_config.h
    dataset_model.h
  config/
    xml_loader.h
    xml_loader.cpp
  trdp/
    trdp_session.h
    trdp_session.cpp
    pd_endpoint.h
    pd_endpoint.cpp
    md_endpoint.h
    md_endpoint.cpp
    file_transfer.h
    file_transfer.cpp
  ui/
    tui_app.h
    tui_app.cpp
    screens/
      screen_main_menu.h
      screen_config_summary.h
      screen_endpoint_list.h
      screen_endpoint_detail.h
      screen_dataset_editor.h
      screen_logs.h
      screen_stats.h
  util/
    logging.h
    logging.cpp
    trdp_error_strings.h
    trdp_error_strings.cpp

4. Coding Standards & Naming Guidelines
MISRA-C++ style discipline applied
Area	Rule
Naming	PascalCase for classes, camelCase for variables & methods, SCREAMING_SNAKE_CASE for constants.
Functions	Single responsibility, no more than 50 lines.
Global vars	Forbidden.
Memory	RAII, no raw new/delete.
Thread safety	No shared mutable state without guards.
Error handling	No exceptions, pure return code checking using enum results.
Logging	Mandatory for all TRDP API failures.
Example naming conventions
class PdEndpointRuntime;
TRDP_ERR_T startPdEndpoint();
uint32_t cycleTimeUsec;
const char *trdpErrToString(TRDP_ERR_T error);

5. Build Requirements
Dependency	Notes
C++17	Required
TRDP Light Stack	Must be pre-built and linked
tau_xml	Must be included
FTXUI	UI framework
pkg-config & cmake	Build system
gcc / clang	MISRA-friendly compilers
6. XML Input Loading Flow
main.cpp
  --> loadSimulatorConfigFromXml(path)
  --> SimulatorConfig structure populated
  --> create TRDP Session
  --> build runtime PD/MD endpoints
  --> start TUI

XML Loader API
XmlLoadResult loadSimulatorConfigFromXml(
    const std::string &path,
    SimulatorConfig   &outConfig,
    std::string       &outErrorMessage);

7. Runtime PD / MD Operation Flow
User selects endpoint from UI
    |
    v
PdEndpointRuntime.start(session)
    |
    v
payloadBuffer edited via dataset editor
    |
    v
sendCurrentPayload()  --> tlp_put()


MD file transfer workflow:

ui -> FileTransferSession.start()
while (!completed)
    FileTransferSession.process()

8. Key Classes and Their Purpose
Class	Responsibility
TrdpSession	Init, process, terminate TRDP session
PdEndpointRuntime	Publish/subscribe PD endpoints, manage payload
MdEndpointRuntime	Request/notify/reply logic
FileTransferSession	MD chunk-based file send/receive
TuiApp	Manages UI states
SimulatorConfig	Device config mapped from XML
9. Logging
logMessage(LogLevel::Error, "Failed to publish PD endpoint");


Used with macro:

#define LOG_TRDP_CALL(expr, ctx) \
do { TRDP_ERR_T e = (expr); \
if (e != TRDP_NO_ERR) { \
   logMessage(LogLevel::Error, std::string(ctx) + trdpErrToString(e)); \
}} while (0)

10. Development Phases
Phase 1

Implement data model (model/)

Implement XML loader

Display parsed config in TUI

Phase 2

Implement TRDP session wrapper

PD endpoint publish/subscribe

Phase 3

MD request/reply

Dataset editor

Phase 4

File transfer & statistics

Advanced debug views

11. Testing Strategy
Test Type	Description
Unit tests	XML parser correctness, PD payload buffer operations
Integration tests	PD/MD exchange with real TRDP node
Performance	1ms PD cycle under load
Boundary tests	TRDP_MAX_MD_DATA_SIZE
12. Final Goal Checklist
Feature	Status
Load XML IEC config	☐
Display endpoints & edit	☐
PD publish / subscribe	☐
MD file transfer	☐
TSN support	☐
Statistics realtime view	☐
No polling (select/event driven)	☐
13. Build & Run

### Quick start (Linux)

After cloning the repository, you can build and launch the simulator with a single command from the repo root:

```
scripts/build_and_run.sh
```

The script will:

* Initialize/update the bundled submodules (FTXUI, TCNopen)
* Configure the CMake project (defaults to Release build in `./build`)
* Build the default target (`trdp_simulator`) and launch it

Environment overrides:

* `BUILD_DIR` – custom build directory (default: `./build`)
* `BUILD_TYPE` – CMake build type (default: `Release`)
* `TARGET` – CMake target to build (default: `trdp_simulator`)
* `SKIP_RUN` – set to skip launching after a successful build

### Manual build

Clone with submodules to ensure the bundled FTXUI and TCNopen stacks are available:

```
git clone --recurse-submodules <repo-url>
```

Then configure and build:

```
mkdir -p build
cd build
cmake ..
cmake --build .
```

Run the simulator (provide a TRDP XML configuration path if different from `config.xml`):

```
./trdp_simulator external/TCNopen/trdp/example/example.xml
```
13. Future expansion

MQTT-based remote control option

Capture/replay PCAP

Integrated Wireshark exporter
