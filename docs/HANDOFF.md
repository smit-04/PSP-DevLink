# HANDOFF.md

Milestone: Milestone 4 — USB Transport Implementation

Status: Completed (USB Transport and Emulator Mock Fallback Implemented)

---

# Summary

Milestone 4 has been fully completed. We transitioned from placeholder transport APIs to full USB transport backend implementations on both the Desktop Companion and the PSP client, while maintaining strict transport abstraction from the protocol layer.

1. **Desktop Companion**: Integrated `libusb-1.0` to discover, claim, and communicate with the PSP over Bulk OUT (`0x02`) and Bulk IN (`0x81`) endpoints.
2. **PSP Client**: Switched the application to Kernel Mode (`0x1000`) and implemented the hardware-level USB driver using the SCE bus driver APIs, with 64-byte alignment, cache coherence, and non-blocking asynchronous event flags.
3. **Emulator Fallback**: Implemented a graceful Mock loopback mode that activates if real hardware registration fails, ensuring the application runs correctly on PPSSPP for testing.

---

# Deliverables Completed

* **Desktop CMake Updates**: Integratedpkg-config checks for `libusb-1.0` and linked the library target to the Desktop Companion executable.
* **Desktop libusb-1.0 Transport**: Wrote the C++ code to open the device, claim interface 0, handle active kernel drivers, and perform bulk transfers.
* **PSP Kernel Mode Configuration**: Updated `main.c` and `Makefile` to run the EBOOT in kernel mode and link to `pspusbbus_driver` and `pspusb_driver` libs.
* **PSP USB Driver**: Wrote C code to register the USB driver, configure descriptor tables, manage MIPS DMA cache lines (writeback/invalidation), and queue Bulk transfers.
* **Asynchronous Check Loop**: Polled timed event flags in `transport_receive()` to avoid blocking the frame rendering loop.
* **Emulator Mock Fallback**: Added loopback fallback logic when hardware USB is unavailable (e.g. under PPSSPP).

---

# Verification Summary

* **Desktop Compilation**: Verified that the Desktop Companion compiles under WSL with zero warnings/errors.
* **PSP Compilation**: Verified that the PSP EBOOT compiles and packages successfully under WSL.
* **PPSSPP Emulator Validation**: Loaded the EBOOT on PPSSPP; it successfully initialized the display and controller, printed the fallback warning on screen, and responded to START button exit cleanly.

---

# Remaining Work

* Protocol handshake implementation.
* Packet serialization and deserialization.
* Runtime message routing and processing.
* User interface and state machine management.

---

# Recommended Next Milestone

**Milestone 5 — Handshake and Packet Serialization**

Focus on establishing a robust communication session:
1. Implement a handshake protocol where the PSP client and Desktop Companion exchange version info to establish a link.
2. Implement packet serialization and validation structures based on the `PSPDL_PacketHeader` to frame inter-device messages.

---

End of Document
