# Repository Architecture

## Purpose

The Repository Architecture document defines how the PSP DevLink repository is organized and how its contents are intended to evolve throughout development.

Its objectives are to:

- describe the purpose of each top-level directory
- establish repository organization guidelines
- maintain a predictable project structure
- support long-term maintainability
- ensure future contributors can navigate the repository without relying on external documentation

This document describes the current repository structure. Future repository changes should be documented only after they have been implemented and verified.

---

## Repository Structure

The repository is organized into a small number of top-level directories.

Each directory has a single, clearly defined responsibility.

Current structure:

```text
PSP-DevLink/
│
├── docs/
├── src/
├── .gitignore
├── Makefile
└── README.md
```

As the project evolves, additional directories may be introduced. Structural changes must be documented only after they have been implemented and verified.

---

## Top-Level Directories

### docs/

The `docs/` directory contains all project documentation.

This includes:

- project governance documents
- architecture documentation
- engineering decisions
- milestone handoff documents
- project state information

Architecture documentation is maintained under `docs/architecture/`.

Documentation evolves alongside implementation and serves as the primary engineering reference for the project.

---

### src/

The `src/` directory contains the current PSP application source code.

During the early milestones, all PSP source files are located here.

If the repository is reorganized in a future milestone, this document will be updated after the new structure has been implemented and verified.

---

## Build Files

### Makefile

The repository uses a top-level `Makefile` as the primary build entry point for the PSP application.

The Makefile coordinates the PSP build process through the PSP SDK and PSPSDK toolchain. Its responsibilities include:

- compiling source files
- linking the application
- generating the final `EBOOT.PBP`
- providing a consistent build interface for developers

The Makefile should contain build configuration only. Application logic, project configuration, and feature implementation must remain within the source code.

---

## Repository Files

### README.md

The `README.md` file serves as the primary entry point for the repository.

It should provide:

- a high-level project introduction
- build prerequisites
- setup instructions
- links to important documentation
- project status

The README should remain concise and direct readers to detailed documentation where appropriate.

---

### .gitignore

The `.gitignore` file defines files and directories that must not be committed to version control.

Typical ignored content includes:

- build artifacts
- temporary files
- editor-specific configuration
- generated binaries
- operating system metadata

Repository-specific source code and documentation must never be ignored unintentionally.

---

## Future Expansion

As PSP DevLink grows, the repository structure may evolve to better separate concerns.

Potential additions include:

- dedicated desktop application directories
- shared protocol definitions
- automated test directories
- development tools
- project scripts

Repository changes should be introduced only when justified by project requirements and documented after they have been implemented and verified.

---

## Repository Rules

The following principles govern repository organization:

- Each top-level directory must have a clearly defined purpose.
- Documentation should evolve alongside implementation.
- Source code should remain separated from project documentation.
- Build artifacts must not be committed.
- New top-level directories require corresponding documentation updates.
- Repository organization changes require corresponding documentation updates.
- Documentation must accurately reflect the implemented repository structure.

---

End of Document
