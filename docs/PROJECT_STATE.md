# PROJECT_STATE.md

Version: 4.0

Status: ACTIVE

---

# Project

PSP DevLink

---

# Current Milestone

Milestone 9 — Desktop Notification Collection Service

Status: COMPLETED (VERIFIED IN EMULATOR)

---

# Objective

Capture host desktop notifications (using D-Bus on Linux/WSL) and stream them dynamically to the PSP Client to render a clean, color-coded notification ticker at the bottom of the dashboard screen.

---

# Milestone Summary

Completed during this milestone:

* Register `PSPDL_MESSAGE_NOTIFICATION = 5` and define `PSPDL_NotificationPayload` structure (128 bytes)
* Implement D-Bus monitoring `NotificationService` daemon with thread-safe queue and simulated fallback support
* Integrate notification packet polling and network serialization into companion main loop
* Add router decoding and global cache management for notifications in the client
* Implement row-aligned ticker rendering with auto-truncation and residue space-clearing in the client UI

Current implementation displays a beautiful terminal-based telemetry layout with real-time stats updates, progress indicators, and host notification ticker.

Not yet implemented:

* Desktop GUI configurations settings page
* PSP notifications overlay popup card
* message processing
* session management

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
* Milestone 7 Desktop System Services Integration completed
* Milestone 8 PSP Graphics Engine and UI Dashboard completed
* Milestone 9 Desktop Notification Collection Service completed
* D-Bus notification parsing library implemented on Desktop
* Notification ticker drawing and residue clean-up logic coded in client GUI
* Real and mock notification pipeline validated under PPSSPP emulation
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
* Shared protocol interfaces and transport abstraction
* Real USB transport backend via libusb (Desktop) and Usbbd/Usbd (PSP)
* Shared packet handshake and watchdog timers
* System stats telemetry and Git status monitoring services
* Flicker-free PSP graphical UI dashboard (vsync synchronized)
* Desktop notification collection service (D-Bus listener + simulation)
* Client-side notification ticker display

Not Yet Implemented:

* Desktop GUI configurations settings page
* PSP notification card overlay popups

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

Real notifications require an active session D-Bus in the WSL environment. In headless WSL instances, the companion falls back automatically to simulated notifications.

---

# Next Task

Milestone 10 — Desktop Companion Settings Page & Session Manager.

---

End of Document
