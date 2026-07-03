# Build Architecture

## Purpose

The Build Architecture defines how PSP DevLink components are built, organized, and verified throughout the project's lifecycle.

Its objectives are to:

- provide reproducible builds
- maintain independent build processes for project components
- separate build configuration from application logic
- support future repository expansion without disrupting existing workflows

This document defines build architecture rather than build implementation details.

---

## Design Goals

The build system should be:

- reproducible
- deterministic
- modular
- easy to understand
- easy to extend
- independently buildable

---

## Current Build Architecture

The current repository builds the PSP application using:

- PSP SDK
- PSPSDK
- GNU Make

The repository currently exposes a single top-level `Makefile` that serves as the primary build entry point.

The build process produces a PSP executable package (`EBOOT.PBP`) suitable for deployment to the target device.

---

## Build Responsibilities

The build system is responsible for:

- compiling source code
- resolving project dependencies
- linking application binaries
- generating build artifacts
- reporting build failures

The build system must not contain application logic or feature-specific behavior.

---

## Build Principles

The build architecture follows these principles:

- build configuration remains separate from application code
- each application should remain independently buildable
- builds should be reproducible on a clean development environment
- warnings should be investigated before a build is considered successful
- build outputs should remain deterministic for identical source revisions

---

## Future Evolution

As PSP DevLink expands, the build architecture may include:

- independent desktop application builds
- shared build configuration
- automated verification
- continuous integration
- release packaging

Future build changes should preserve reproducibility and be documented only after they have been implemented and verified.

---

## Architectural Constraints

The build architecture follows these constraints:

- Build scripts contain build logic only.
- Application behavior must never be implemented in build scripts.
- Build systems should remain independent wherever practical.
- Build artifacts must not be committed to version control.

These constraints preserve maintainability and simplify future expansion.

---

## Out of Scope

This document does not define:

- compiler flags
- optimization levels
- CI implementation
- release workflow
- packaging implementation

These topics belong to implementation documentation.

---

End of Document
