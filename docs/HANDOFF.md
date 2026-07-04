# HANDOFF.md

Milestone: Milestone 6 — Message Processing and Routing

Status: Completed (Message Payloads, Serialization, Routing, and Visual Telemetry Verified in Emulator)

---

# Summary

Milestone 6 has been fully completed. We extended the shared protocol and communication layers to support transmission, verification, deserialization, and routing of structured payloads (System Stats and Git Status) carrying host data to the PSP client state cache.

1. **Structured Telemetry Payloads**: Defined exact payload structures for system performance metrics (`cpu_usage`, `ram_usage`, `cpu_temp`, `ram_total`, `ram_free`) and Git repository updates (`modified_files`, `untracked_files`, `branch_name`).
2. **Byte-Level Payload Serialization**: Implemented warning-free serialization and deserialization routines in `payload.c` using shifts to guarantee Little-Endian packing, resolving MIPS (client) vs Host (companion) architecture differences.
3. **Desktop Telemetry Stream**: Configured the companion application to stream system metrics every 1s and Git status updates every 3s.
4. **PSP Router Dispatcher**: Created the `message_router` component on the PSP client to decode packets, cache updates in global telemetry state stores, and render formatted outputs to the screen.
5. **Interactive Telemetry Simulation**: Upgraded the emulator mock transport on the PSP to inject simulated System Stats and Git Status updates, confirming that the entire payload framing and routing pipeline runs flawlessly on PPSSPP.

---

# Deliverables Completed

* **Payload Definitions**: Message types added to `message.h` and structures declared in `payload.h`.
* **Payload Serialization API**: Custom byte-level Little-Endian marshalling functions built in `payload.c`.
* **Desktop companion Telemetry Loop**: Broadcast scheduler in `main.cpp`.
* **PSP Client Dispatcher**: `message_router.c` to parse message types and cache/output updates.
* **PSP Client receive loop integration**: Payload assembly logic in `main.c`.
* **Emulator mock simulation**: Custom payload injection sequence for visual telemetry verification.

---

# Verification Summary

* **Desktop Companion Build**: Compiles and links successfully under WSL.
* **PSP Client Build**: Compiles, links all static libraries/objects, and builds `EBOOT.PBP` cleanly under WSL.
* **PPSSPP Emulator Validation**: Loaded the EBOOT on PPSSPP; observed the simulation running on screen:
  1. Displays `Waiting for Host Connection...`
  2. Handshake succeeds and prints `Connected to Host.`
  3. Receives and displays stats: `[STATS] CPU:35% RAM:55% Temp:42.5C`
  4. Receives and displays git updates: `[GIT] Branch:dev-branch Mod:5 Untracked:2`
  5. Disconnects due to watchdog timeout when heartbeats stop.

---

# Remaining Work

* Desktop system service integration (gathering actual system stats and Git repository logs via host operating system APIs).
* PSP graphics rendering engine (converting debug printouts to a graphical HUD interface using the PSP GU).
* Desktop notifications service.

---

# Recommended Next Milestone

**Milestone 7 — Desktop System Services Integration**

Focus on acquiring real host metrics on the Desktop Companion:
1. Replace the hardcoded simulation timers in `apps/desktop/src/main.cpp` with actual background system monitor threads.
2. Query system APIs on the host (e.g. `/proc/stat` on Linux/WSL or registry keys) to acquire CPU load and memory usage.
3. Execute local git commands (e.g. `git symbolic-ref --short HEAD` and `git status --porcelain`) to read real git metrics.
4. Package this information into protocol payloads and stream them to the PSP client.

---

End of Document
