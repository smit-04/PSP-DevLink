# PSP DevLink

PSP DevLink is a custom software engineering project that transforms a Sony PlayStation Portable (PSP) into a USB-connected developer companion for a desktop computer.

The project establishes a real-time, bi-directional USB bulk transfer link between a modern desktop host and a PSP running custom firmware. It consists of three primary components:

* **Desktop Companion** — A cross-platform C/C++ application (using `libusb-1.0`) that collects host system data (or arbitrary payloads) and sends it over USB to the PSP.
* **PSP User-Mode Application (EBOOT)** — Receives data from the desktop and presents it via a lightweight hardware-accelerated user interface.
* **PSP Kernel-Mode Driver (PRX)** — A custom USB bus driver that interfaces directly with the PSP's internal `sceUsbBusDriver` subsystem to handle raw bulk endpoint transfers, bypassing standard networking limitations.

## Features & Capabilities

* **Custom USB Vendor/Product IDs:** Identifies the PSP specifically as a DevLink device.
* **Direct USB Bulk Transfers:** Low latency data exchange without relying on WiFi or network modules (fully compatible with PSP Street E-1003 models).
* **Kernel/User Separation:** A clean architecture where the kernel PRX handles hardware interrupts and data DMA, while the user-mode EBOOT handles UI and logic.
* **Custom Binary Protocol:** A structured packet protocol with checksum validation for reliable data transmission.
* **Cross-Platform Host:** The desktop client uses standard `libusb` and CMake, making it portable.

## Repository Structure

```text
PSP-DevLink/
├── apps/
│   ├── desktop/      # CMake-based host application
│   └── psp/          # PSP EBOOT and Kernel PRX driver
│       └── driver/   # pspdl_driver.prx source
├── docs/             # Project documentation and decision logs
├── shared/           # Protocol definitions shared between host and PSP
└── scripts/          # Build and utility scripts
```

## Build Instructions

### Prerequisites
* **Desktop:** CMake, a C compiler (GCC/Clang/MSVC), and `libusb-1.0` development headers.
* **PSP:** A configured PSPDEV toolchain (PSPSDK) and a PSP running Custom Firmware (e.g., 6.60/6.61 PRO-C).

### Building the Desktop Companion
```bash
cd apps/desktop
mkdir build && cd build
cmake ..
cmake --build .
```

### Building the PSP Application
```bash
# Requires PSPDEV environment variables to be set
cd apps/psp
make clean
make
```
This builds both the `pspdl_driver.prx` kernel module and the `EBOOT.PBP` executable.

## Installation on PSP

Copy the following files to your PSP's memory stick into `PSP/GAME/PSPDevLink/`:
1. `EBOOT.PBP` (The main application)
2. `pspdl_driver.prx` (The USB kernel driver)

## Technical Notes

* **Git Binary Corruption:** When cloning this repository on Windows, ensure that `.gitattributes` is respected so that Git does not corrupt binary files (`*.PBP`, `*.prx`) via CRLF line-ending conversion.
* **Network Independence:** This project deliberately avoids using `sceNet` modules, making it functional on PSP models without WiFi hardware (like the E-1000 "Street" series).
* **Static Kernel Imports:** The kernel driver links statically against `libpspusb_driver.a` and `libpspusbbus_driver.a` and relies on the user-mode EBOOT to pre-load `flash0:/kd/usb.prx` and `usbbd.prx` to avoid `0x8002013C` kernel panics.

## Current Status

**Active Development** - USB communication and kernel driver stability have been achieved. The host can successfully connect to the PSP and transmit structured payloads.
