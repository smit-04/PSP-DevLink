# PROJECT_STATE.md

Version: 4.0

Status: ACTIVE

---

# Project

PSP DevLink

---

# Current Milestone

Milestone 10 — Desktop Companion Settings Page & Session Manager

Status: COMPLETED (VERIFIED IN EMULATOR)

---

# Objective

Build an interactive, non-blocking Terminal User Interface (TUI) settings dashboard and configuration storage for the Desktop Companion, and support remote console actions (Exit to XMB, Reboot Console) from the companion to the PSP Client.

---

# Milestone Summary

Completed during this milestone:

* Register `PSPDL_MESSAGE_CONTROL = 6` and define `PSPDL_ControlPayload` structure (1 byte)
* Implement `ConfigService` reading/writing configurations to `config.ini` files
* Implement `TuiService` setting raw termios console parameters and rendering cursor-aligned options pages
* Integrate configuration variables into the main companion telemetry intervals
* Process remote exit and reboot triggers client-side, incorporating `<psppower.h>` cold resets
* Map controller mock button bindings (`SELECT + SQUARE`/`TRIANGLE`) to verify handlers inside emulators

Current implementation displays an interactive TUI configurations console on the host, and supports remote client power-off and system reboot signals.

Not yet implemented:

* PSP notification card overlay popups
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

Not Yet Implemented:

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

Real USB transport communication between Windows/WSL and PPSSPP is not emulated; local control simulation keys are mapped on the client.

---

# Next Task

Milestone 11 — PSP Notification Card Overlay Popups.

---

End of Document
