# TRDPTestingTool

content = """
# TRDP Simulator (Terminal UI, Linux / Raspberry Pi)

## Overview
This project implements a comprehensive TRDP (Train Real-time Data Protocol) simulator supporting:
- **PD (Process Data)** publishing and subscribing
- **MD (Message Data)** request, reply, notification
- **IEC-61375-2-3 compliant** XML configuration input
- **Dataset browsing and live editing**
- **File transfer through MD (chunked streaming)**
- **Terminal-based user interface using FTXUI**
- **Event-driven architecture (no polling)**

The simulator is intended for rail/industrial TRDP system testing and supports multiple devices and telegrams defined in configuration XML.

---

## High-Level Architecture

