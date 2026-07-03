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