# Repository Architecture

## Purpose

The Repository Architecture document defines how the PSP DevLink repository is organized and how its contents are intended to evolve throughout development.

Its objectives are to:

* describe the purpose of each top-level directory
* establish repository organization guidelines
* maintain a predictable project structure
* support long-term maintainability
* ensure future contributors can navigate the repository without relying on external documentation

This document describes the current repository structure. Future repository changes should be documented only after they have been implemented and verified.

---

## Repository Structure

The repository is organized into a small number of top-level directories.

Each directory has a single, clearly defined responsibility.

Current structure:

```text
PSP-DevLink/
│
├── apps/
│   ├── desktop/
│   └── psp/
├── docs/
│   └── architecture/
├── shared/
├── scripts/
├── .gitignore
└── README.md
```

As the project evolves, additional directories may be introduced. Structural changes must be documented only after they have been implemented and verified.

---

## Top-Level Directories

### apps/

The `apps/` directory contains the project's executable applications.

Current applications include:

* `desktop/` — Desktop Companion
* `psp/` — PSP application

Each application owns its own source code, build configuration, and entry point.

Applications remain independently buildable using their respective build systems.

---

### docs/

The `docs/` directory contains all project documentation.

This includes:

* project governance documents
* architecture documentation
* engineering decisions
* milestone handoff documents
* project state information

Architecture documentation is maintained under `docs/architecture/`.

Documentation evolves alongside implementation and serves as the primary engineering reference for the project.

---

### shared/

The `shared/` directory contains components intended to be shared between multiple applications.

Examples include:

* protocol definitions
* shared interfaces
* common utilities
* shared constants

Shared components should remain platform-independent wherever practical.

---

### scripts/

The `scripts/` directory contains development and automation scripts.

Typical responsibilities include:

* developer utilities
* build automation
* release automation
* verification scripts

Scripts should support development workflows without containing application logic.

---

## Build Organization

Each application owns its own build system.

Current build systems are:

* `apps/psp/` — GNU Make with PSPDEV / PSPSDK
* `apps/desktop/` — CMake

This separation keeps applications independently buildable while allowing each platform to use the tooling most appropriate for its environment.

Build configuration should remain separate from application implementation.

---

## Repository Files

### README.md

The `README.md` file serves as the primary entry point for the repository.

It should provide:

* a high-level project introduction
* build prerequisites
* setup instructions
* links to important documentation
* project status

The README should remain concise and direct readers to detailed documentation where appropriate.

---

### .gitignore

The `.gitignore` file defines files and directories that must not be committed to version control.

Typical ignored content includes:

* build artifacts
* temporary files
* editor-specific configuration
* generated binaries
* operating system metadata

Repository-specific source code and documentation must never be ignored unintentionally.

---

## Future Expansion

As PSP DevLink grows, the repository structure may evolve to better separate concerns.

Potential additions include:

* automated test directories
* development tools
* continuous integration configuration
* release packaging
* additional shared libraries

Repository changes should be introduced only when justified by project requirements and documented after they have been implemented and verified.

---

## Repository Rules

The following principles govern repository organization:

* Each top-level directory must have a clearly defined purpose.
* Documentation should evolve alongside implementation.
* Applications should remain independently buildable.
* Shared code should remain isolated from application-specific implementations.
* Build artifacts must not be committed.
* New top-level directories require corresponding documentation updates.
* Repository organization changes require corresponding documentation updates.
* Documentation must accurately reflect the implemented repository structure.

---

End of Document
