# PSP DevLink

PSP DevLink is a portfolio-quality software engineering project that transforms a Sony PSP into a USB-connected developer companion for a desktop computer.

The project consists of two independent applications:

* **Desktop Companion** — Collects host system information and communicates with the PSP.
* **PSP Application** — Receives data from the Desktop Companion and presents it through a lightweight user interface.

The project emphasizes:

* Clean software architecture
* Maintainable engineering practices
* Documentation-driven development
* Independent build systems
* Long-term extensibility

## Repository Structure

```text
PSP-DevLink/
├── apps/
│   ├── desktop/
│   └── psp/
├── docs/
├── shared/
├── scripts/
└── README.md
```

## Build Systems

* **Desktop Companion:** CMake
* **PSP Application:** PSPDEV / PSPSDK (GNU Make)

Each application is built independently.

## Documentation

Project documentation is located in `docs/`.

Architecture documentation is located in `docs/architecture/`.

## Current Status

**Milestone 2 – Project Foundation Implementation**

Repository restructuring and build system foundation are complete.

End-user runtime features will be introduced in subsequent milestones.
