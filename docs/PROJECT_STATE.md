# PROJECT_STATE.md

Version: 4.0

Status: ACTIVE

---

# Project

PSP DevLink

---

# Current Milestone

Milestone 6 — Message Processing and Routing

Status: COMPLETED (VERIFIED IN EMULATOR FALLBACK)

---

# Objective

Implement structured message payload formats, platform-independent serialization, and a client-side routing dispatcher that parses message types, deserializes fields, and updates local state.

---

# Milestone Summary

Completed during this milestone:

* Add `PSPDL_MESSAGE_SYSTEM_STATS` and `PSPDL_MESSAGE_GIT_STATUS` message IDs
* Define payload structs and serialization APIs in `payload.h`
* Implement Little-Endian explicit serialization/deserialization for stats/git (`payload.c`)
* Configure `CMakeLists.txt` to include `src/payload.c`
* Implement simulated telemetry broadcasts in Desktop Companion (`main.cpp`)
* Compile and link shared payload object in PSP Client (`Makefile`)
* Implement state stores and dispatcher on PSP client (`message_router.c`)
* Integrate dispatcher and payload readers inside PSP main loop (`main.c`)
* Upgrade mock simulation in `transport_usb.c` to inject system stats and git status payloads

Current implementation has fully implemented message payload serialization and client routing. The client parses headers, extracts variable-sized payloads, and updates local telemetry state caches dynamically.

Not yet implemented:

* Desktop system services (reading real CPU/Git metrics via system APIs)
* PSP graphics rendering engine (visual dashboard panel)
* notifications collection and display
* message processing
* session management
* runtime communication

---

# Verified Environment

## Host OS

* Windows

## Linux Environment

* WSL2
* Ubuntu 24.04.4 LTS

## Development Tools

* Git
* GNU Make
* PSPDEV Toolchain
* PSPSDK
* psp-gcc
* CMake
* GNU C++

## Editor

* Visual Studio Code
* Remote - WSL Extension

## Emulator

* PPSSPP (Windows)

Environment verification remains valid from Milestone 0.

---

# Repository

Repository Name

PSP-DevLink

Branch

main

Remote

GitHub (SSH)

Repository Status

* Milestone 0 completed
* Milestone 1 architecture completed
* Milestone 2 repository restructuring completed
* Milestone 3 communication foundation completed
* Milestone 4 USB Transport Implementation completed
* Milestone 5 Handshake and Packet Serialization completed
* Milestone 6 Message Processing and Routing completed
* Shared static library compiles packet and payload serialization
* Desktop streams simulated system and git telemetry
* PSP client routes payloads to state managers
* Visual mock telemetry verification loop verified in PPSSPP
* Pending push to remote repository

---

# Architecture Documents

The repository currently contains:

```text
docs/architecture/
├── overview.md
├── repository.md
├── desktop.md
├── psp.md
├── communication.md
├── protocol.md
├── system_lifecycle.md
├── build.md
└── coding_standards.md
```

These documents define the architectural direction of the project and remain the primary engineering reference.

---

# Repository Structure

```text
PSP-DevLink/
├── apps/
│   ├── desktop/
│   │   └── src/
│   │       └── transport_usb.cpp
│   └── psp/
│       └── src/
│           └── transport_usb.c
├── shared/
│   └── protocol/
│       ├── include/
│       └── CMakeLists.txt
├── docs/
│   └── architecture/
├── scripts/
├── .gitignore
└── README.md
```

---

# Current Implementation Status

Implemented:

* Production repository structure
* Independent Desktop and PSP build systems
* Shared protocol interfaces
* Transport abstraction
* Platform-specific Desktop transport backend
* Platform-specific PSP transport backend
* Shared protocol INTERFACE library
* Verified Desktop build
* Verified PSP build

Not Yet Implemented:

* USB communication
* Runtime protocol exchange
* Protocol handshake
* Packet serialization
* Message processing
* Desktop services
* PSP graphics
* Rendering engine
* Notification system
* Plugin architecture
* Runtime features

---

# Build Verification

Verified:

* Desktop Companion builds successfully using CMake.
* Shared protocol interface integrates successfully with the Desktop build.
* PSP application builds successfully using PSPDEV and PSPSDK.
* PSP EBOOT generation verified.
* Platform-specific transport backends compile successfully on both targets.

---

# Known Limitations

The transport backends currently contain placeholder implementations.

USB initialization, byte transmission, byte reception, and transport shutdown will be implemented during the remainder of Milestone 4.

---

# Next Task

Complete the USB transport backend implementation for both Desktop and PSP while preserving transport independence from the protocol layer.

---

End of Document
