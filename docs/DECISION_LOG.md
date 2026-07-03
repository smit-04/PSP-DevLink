# DECISION_LOG.md

Version: 1.0

Status: ACTIVE

---

# ADR-001

Title

Primary Development Environment

Decision

WSL2 is the official development environment.

Reason

Provides a stable Linux environment compatible with PSPDEV and PSPSDK.

---

# ADR-002

Title

Editor

Decision

Visual Studio Code with Remote - WSL is the official editor configuration.

Reason

Avoids Windows filesystem permission issues and provides Linux-native tooling.

---

# ADR-003

Title

Version Control

Decision

GitHub is the canonical remote repository.

Authentication uses SSH.

Reason

Reliable authentication for long-term development.

---

# ADR-004

Title

Runtime Verification

Decision

Every executable milestone must be verified inside PPSSPP.

Reason

A successful build alone is insufficient proof of correctness.

---

# ADR-005

Title

Smoke Test

Decision

Maintain a permanent smoke-test application.

Purpose

Verify:

- Build
- Rendering
- Input
- Runtime
- Clean shutdown

This application serves as the first verification after cloning the repository.

---

# ADR-006

Title

Repository Philosophy

Decision

The repository is the source of truth.

Chat history must never be required to understand the project.

All engineering decisions and project state must be documented inside the repository.

---

# ADR-007

Title

Desktop Application Architecture

Decision

The Desktop Companion shall follow a layered architecture consisting of:

- UI
- Application Layer
- Services Layer
- Transport & Protocol Layer
- Platform Integration Layer

Reason

A layered architecture separates responsibilities, minimizes coupling, improves maintainability, and allows platform-specific functionality to remain isolated from application logic.

---

# ADR-008

Title

PSP Application Architecture

Decision

The PSP application shall follow a single-threaded, frame-driven architecture centered around a deterministic main loop.

Reason

A deterministic execution model simplifies debugging, preserves predictable rendering performance, and aligns with the resource constraints of the PSP platform.

---

# ADR-009

Title

Communication Architecture

Decision

PSP DevLink shall use a client-server communication model.

The Desktop Companion is the authoritative source of host system information.

The PSP acts as a presentation-oriented client.

Reason

This architecture establishes clear ownership of application data, minimizes synchronization complexity, and isolates communication from presentation logic.

---

# ADR-010

Title

Repository Architecture

Decision

Repository organization shall evolve alongside implementation.

Documentation must accurately reflect the implemented repository structure and serve as the primary engineering reference.

Reason

Keeping repository documentation synchronized with implementation ensures that the repository remains understandable without relying on external context or chat history.

---

# ADR-011

Title

Architecture-First Development

Decision

Major architectural decisions shall be documented and reviewed before feature implementation begins.

Reason

Establishing architecture before implementation reduces redesign effort, provides consistent engineering direction, and improves long-term maintainability.

---

End of Document
