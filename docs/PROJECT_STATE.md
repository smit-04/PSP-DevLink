# PROJECT_STATE.md

Version: 4.0

Status: ACTIVE

---

# Project

PSP DevLink

---

# Current Milestone

Milestone 5 — Handshake and Packet Serialization

Status: COMPLETED (VERIFIED IN EMULATOR FALLBACK)

---

# Objective

Establish a communication session protocol between the Desktop Companion and the PSP. Implement a byte-level serialization layer for the `PSPDL_PacketHeader` and define the handshake state machine logic on both targets to ensure connection reliability.

---

# Milestone Summary

Completed during this milestone:

* Convert `pspdevlink_protocol` to a CMake STATIC library
* Implement Little-Endian explicit packet header serialization/deserialization (`packet.c`)
* Declare serialization functions with `extern "C"` linkage in `packet.h`
* Update PSP Makefile to compile and link shared protocol files
* Implement Desktop Companion handshake loop and heartbeat broadcast (`main.cpp`)
* Implement PSP Client handshake loop and connection status rendering (`main.c`)
* Implement 5-second inactivity watchdog timer on PSP
* Develop interactive visual mock packet simulation for PPSSPP verification

Current implementation provides a fully operational handshake and packet serialization protocol. Both applications transition connection states dynamically and handle timeouts gracefully.

Not yet implemented:

* message processing (routing payloads to specific handlers)
* Desktop services (CPU/Git monitoring)
* PSP graphics rendering engine (visual dashboard)
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
* Shared protocol module converted to a STATIC library with packet serialization
* Platform-specific transport backends established and linked
* Handshake loops and watchdogs implemented on both targets
* Interactive mock handshake simulation verified on PPSSPP
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
