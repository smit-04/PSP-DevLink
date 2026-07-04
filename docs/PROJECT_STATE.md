# PROJECT_STATE.md

Version: 4.0

Status: ACTIVE

---

# Project

PSP DevLink

---

# Current Milestone

Milestone 11 — PSP Notification Card Overlay Popups

Status: COMPLETED (VERIFIED IN EMULATOR)

---

# Objective

Implement visual popup overlay cards on the PSP Client UI to alert users on incoming notifications, and support debounced controller bindings for close actions and full list drawers.

---

# Milestone Summary

Completed during this milestone:

* Implement static history queue caching last 5 notification payloads in `ui.c`
* Render visually centered modal boxes mapping app, title, and body elements
* Setup 5-second automatic close ticks decrementing frame-by-frame
* Map CIRCLE inputs to dismiss modal dialogs and clear history listings
* Map SELECT inputs to toggle history drawer overlay panels on the dashboard

Current implementation supports dynamically alerting the user about notifications with countdown dismissals.

Not yet implemented:

* Desktop GUI configurations settings page
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
* Milestone 10 Desktop Companion Settings Page & Session Manager completed
* Config file reading/writing parsing implemented
* termios non-blocking terminal console UI implemented
* Remote power-off and reboot command routines integrated on PSP client
* Control shortcuts mapped to buttons in emulator mock loop
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
│   ├── .gitignore
│   └── README.md
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
* Companion settings TUI and INI configuration parsing
* Remote console reboot and shutdown routines
* Visual popup overlay cards and auto-hide display timers
* Scrolling notification history cache and drawer interface

Not Yet Implemented:

* Desktop GUI configurations settings page

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

Real USB transport communication between Windows/WSL and PPSSPP is not emulated; local control simulation keys are mapped on the client.

---

# Next Task

Milestone 12 — Desktop Companion GUI App.

---

End of Document
