System Requirements Specification (SRS)
TRDP Simulator – Terminal-Based TRDP Test & Diagnostic Software

Version: 1.0
Target Platforms: Linux, Raspberry Pi (ARM32/ARM64)
Interfaces: Ethernet, TRDP (PD & MD), XML IEC-61375-2-3
User Interface: Terminal UI (FTXUI)
Author: Alok T J / Codex automated development

1. Introduction
1.1 Purpose

The purpose of the TRDP Simulator software is to provide a comprehensive interactive tool for testing railway and industrial embedded devices that use TRDP communication per IEC-61375-2-3. Users should be able to:

Load an IEC-compliant TRDP XML configuration file

Simulate and observe Process Data (PD) telegrams

Simulate and interact with Message Data (MD) sessions

Edit dataset values dynamically

Send large files using MD chunking

View topology, statistics, and communication logs

Perform diagnostic and validation tasks for end devices

1.2 Intended Users
User Role	Description
TRDP developers & testers	Engineers validating TRDP systems
Integration engineers	Commissioning trains or testbeds
QA / Software validation teams	Automated regression testing
Researchers / students	Learning and experimenting with TRDP
1.3 Scope

The software provides simulation, monitoring, debugging, and visualization of TRDP communication in a single application running locally on Linux or Raspberry Pi. It does not modify real train control commands or safety systems.

2. System Overview

The system uses:

Event-driven reactor model using TRDP stack callbacks (No polling)

FTXUI based terminal interface

TRDP Light stack (TCNOpen) with PD & MD support

XML-based configuration to dynamically construct communication sessions

3. Functional Requirements
3.1 XML Configuration Management
ID	Requirement	Priority
FR-XML-01	System shall load system configuration from IEC-61375-2-3 XML file	High
FR-XML-02	Support reading telegram, interface, dataset, mapped devices	High
FR-XML-03	Validate XML and report errors meaningfully	High
FR-XML-04	Allow reloading configuration without restarting application	Medium
3.2 TRDP Communication
ID	Requirement	Priority
FR-PD-01	Support PD publish according to cycle time in XML	High
FR-PD-02	Support PD subscribe and live display values	High
FR-PD-03	Allow runtime dataset editing	High
FR-MD-01	Support MD request, reply, confirm, notify	High
FR-MD-02	Support MD TCP and UDP modes	High
FR-MD-03	MD large-payload chunked file transfer	Medium
FR-MD-04	Allow user to craft custom MD payload strings	High
3.3 Dataset Editor
ID	Requirement
FR-DS-01	Display dataset elements with name, type, size, value
FR-DS-02	Allow value editing with type validation
FR-DS-03	Support variable-length / array type values
FR-DS-04	Calculate marshalling automatically
3.4 Terminal UI
ID	Requirement
FR-UI-01	Dashboard showing publishers, subscribers, datasets, MD sessions
FR-UI-02	Real-time view of incoming PD & MD
FR-UI-03	Editor panel for datasets
FR-UI-04	View log panel with filtering
FR-UI-05	Menu navigation using keyboard keys only
3.5 Logging & Debugging
ID	Requirement
FR-LOG-01	System shall log TRDP errors by mapping TRDP_ERR_T to human description
FR-LOG-02	Provide per-session and global log viewer
FR-LOG-03	Allow enabling/disabling logs in UI
FR-STAT-01	View PD & MD statistics as received from TRDP stack
3.6 Simulation Control
ID	Requirement
FR-SIM-01	Start/stop PD publish per telegram
FR-SIM-02	Pause/resume all communication
FR-SIM-03	Load replay log files for PD streams
FR-SIM-04	Export communication log to file
4. Non-functional Requirements
ID	Description
NFR-01	Software shall run on Linux and Raspberry Pi OS
NFR-02	Event-driven system (no polling, no unnecessary CPU load)
NFR-03	PD transmission up to 1ms cycle time
NFR-04	Must use C++17, MISRA-C++ compliant coding
NFR-05	Memory safe: no leaks, no undefined behavior
NFR-06	Latency for packet handling ≤ 2 ms
NFR-07	Build using CMake ≥ 3.16
NFR-08	UI interaction latency ≤ 100ms
5. System Architecture Requirements
ID	Requirement
ARCH-01	Use layered architecture separating UI, core modules & TRDP stack
ARCH-02	Message passing using thread-safe queues
ARCH-03	Provide clear API interfaces between layers
ARCH-04	XML parsing isolated from runtime messaging
ARCH-05	No globals except where unavoidable by TRDP stack
6. Hardware & OS Requirements
OS	Version	Arch
Ubuntu / Debian Linux	18+	x86_64
Raspberry Pi OS	2023+	ARM32/ARM64
Resource	Minimum
RAM	256MB
CPU	700MHz single core
Network	Ethernet required for TRDP
7. Constraints

Licensed under Mozilla Public License 2.0

Must not operate safety-critical functionality without supervision

No kernel modifications required

8. Risks & Mitigations
Risk	Mitigation
XML misconfiguration produces undefined PD behavior	XML validator + error messages
Incorrect dataset type conversion	Strong type validation and exception reports
9. Acceptance Criteria

XML file loads with resource mapping shown in UI

PD and MD telegrams can be sent and received

Dataset elements editable live

UI remains stable during heavy communication

File transfer test completes successfully

10. Future Enhancements

MQTT remote simulation control

Data recording and statistical plotting

GUI visualization (optional separate build)

TRDP topology renderer
