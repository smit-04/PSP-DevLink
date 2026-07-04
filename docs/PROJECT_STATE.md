# PROJECT_STATE.md

Version: 4.0

Status: ACTIVE

---

# Project

PSP DevLink

---

# Current Milestone

Milestone 4 — USB Transport Implementation

Status: COMPLETED (VERIFIED IN EMULATOR FALLBACK)

---

# Objective

Implement the first transport backend that enables reliable raw byte communication between the Desktop Companion and the PSP application while preserving the transport abstraction established in the previous milestone.

This milestone focuses exclusively on transport implementation and does not introduce protocol processing or user-facing functionality.

---

# Milestone Summary

Completed during this milestone:

* platform-specific transport architecture
* shared transport interface
* Desktop transport backend integration
* PSP transport backend integration
* Desktop build verification
* PSP build verification
* transport implementation refactor
* USB transport communication (hardware drivers & custom descriptors)
* Graceful emulator mock loopback fallback for PPSSPP
* libusb-1.0 integration on Desktop Companion

Current implementation has fully implemented the USB Transport APIs. Real hardware calls MIPS-aligned DMA and driver registrations, while emulator environment falls back gracefully to Mock loopback mode.

Not yet implemented:

* protocol handshake
* packet serialization
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
* Shared protocol module converted to an INTERFACE library
* Platform-specific transport backends established
* Desktop build verified
* PSP build verified
* USB transport communication (hardware/mock fallback) verified
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
