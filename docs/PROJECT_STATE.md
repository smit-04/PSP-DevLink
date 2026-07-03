# PROJECT_STATE.md

Version: 4.0

Status: ACTIVE

---

# Project

PSP DevLink

---

# Current Milestone

Milestone 3 — Communication Foundation

Status: IMPLEMENTATION COMPLETE (Documentation & Git Verification Pending)

---

# Objective

Design and implement the shared communication foundation used by both the Desktop Companion and the PSP application.

This milestone establishes the protocol architecture and shared interfaces without implementing runtime communication or user-facing functionality.

---

# Milestone Summary

Implementation completed during this milestone includes:

* shared protocol module
* protocol architecture documentation
* protocol versioning framework
* message definitions
* packet header definition
* transport abstraction
* Desktop protocol library integration
* Desktop stub implementation
* PSP stub implementation
* Desktop build verification
* PSP build verification

No runtime communication has been implemented.

No USB functionality has been implemented.

No user interface functionality has been implemented.

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
* Milestone 3 communication foundation implemented
* Desktop protocol library integrated
* Shared protocol module established
* Desktop build verified
* PSP build verified
* Documentation update in progress
* Pending final commit and push

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
│   └── psp/
├── shared/
│   └── protocol/
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
* Shared protocol module
* Protocol public interfaces
* Protocol versioning framework
* Packet definitions
* Message definitions
* Transport abstraction
* Desktop protocol library
* Desktop stub transport
* PSP stub transport
* Verified Desktop build
* Verified PSP build

Not Yet Implemented:

* USB communication
* Runtime protocol exchange
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
* Shared protocol library builds successfully.
* PSP application builds successfully using PSPDEV and PSPSDK.
* PSP EBOOT generation verified.

---

# Known Limitations

Current PSP build configuration places generated object files for shared sources within the shared source tree.

This is acceptable for Milestone 3 and should be revisited during a future build-system improvement milestone.

---

# Next Milestone

Milestone 4 — USB Transport Layer

Objective:

Implement the first transport implementation using USB while preserving the protocol abstraction established during Milestone 3.

---

End of Document