# PROJECT_STATE.md

Version: 4.0

Status: ACTIVE

---

# Project

PSP DevLink

---

# Current Milestone

Milestone 14 — Physical Hardware Deployment (PSP E-1003) and Kernel PRX Stabilization

Status: COMPLETED

---

# Objective

Stabilize the PSP Kernel USB driver on physical hardware (specifically the PSP E-1003 "Street" model), resolve "corrupted data" loading errors caused by implicit network dependencies, and eliminate kernel panics during module startup.

---

# Milestone Summary

Completed during this milestone:

* Discovered and pruned default linker network dependencies (`sceNetInet`) which caused ELF loading failures on non-WiFi PSP models.
* Replaced fragile dynamic NID resolution in the USB PRX driver with stable static imports from `libpspusb_driver.a` and `libpspusbbus_driver.a`.
* Identified and resolved binary corruption of EBOOT.PBP and PRX files caused by Git CRLF conversions on Windows (added `.gitattributes`).
* Implemented pre-loading of system `flash0:/kd/usb.prx` and `usbbd.prx` modules to satisfy kernel dependencies safely.

Not yet implemented:

* Packaging and installation scripts

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

## Emulator & Hardware

* PPSSPP (Windows)
* PSP E-1003 "Street" (PRO-B9 Custom Firmware)

Environment verification remains valid.

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
* Milestone 11 Desktop Web GUI Dashboard completed
* Milestone 14 Physical Hardware Stabilization completed
* Config file reading/writing parsing implemented
* termios non-blocking terminal console UI implemented
* Remote power-off and reboot command routines integrated on PSP client
* Control shortcuts mapped to buttons in emulator mock loop

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
│       ├── driver/
│       │   └── pspdl_driver_kernel.c
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
* Real USB transport backend via libusb (Desktop) and Usbbd/Usbd (PSP Kernel PRX)
* Shared packet handshake and watchdog timers
* System stats telemetry and Git status monitoring services
* Flicker-free PSP graphical UI dashboard (vsync synchronized)
* Desktop notification collection service (D-Bus listener + simulation)
* Client-side notification ticker display
* Companion settings TUI and INI configuration parsing
* Remote console reboot and shutdown routines
* Visual popup overlay cards and auto-hide display timers
* Scrolling notification history cache and drawer interface
* Local TCP Web GUI Dashboard server and REST API endpoints
* Hardware-compatible USB Kernel PRX Driver (statically linked, network-independent)
* Git configuration for binary asset protection (`.gitattributes`)

Not Yet Implemented:

* Packaging and installation scripts

---

# Build Verification

Verified:

* Desktop Companion builds successfully using CMake.
* Shared protocol interface integrates successfully with the Desktop build.
* PSP application builds successfully using PSPDEV and PSPSDK.
* PSP EBOOT generation verified.
* Platform-specific transport backends compile successfully on both targets.
* Binary generation verified safe against CRLF line-ending corruption.

---

# Known Limitations

Real notifications require an active session D-Bus in the WSL environment. In headless WSL instances, the companion falls back automatically to simulated notifications.

Real USB transport communication between Windows/WSL and PPSSPP is not emulated; local control simulation keys are mapped on the client. Hardware USB testing requires a physical PSP running CFW.

---

# Next Task

Milestone 15 — System stability testing and final user testing.

---

End of Document
