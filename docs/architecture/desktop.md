# Desktop Application Architecture

## Purpose

The Desktop Companion is responsible for collecting information from the host computer and communicating it to the PSP.

It performs all platform-specific operations while remaining independent of the PSP user interface.

The Desktop Companion is the authoritative source of host system information within PSP DevLink.

---

## Architectural Style

The Desktop Companion follows a layered architecture.

Each layer has a clearly defined responsibility and communicates only with adjacent layers.

This separation improves maintainability, testing, and future extensibility.

---

## High-Level Architecture

```text
+------------------------------+
|             UI               |
+------------------------------+
|         Application          |
+------------------------------+
|          Services            |
+------------------------------+
| Transport & Protocol Layer   |
+------------------------------+
| Platform Integration Layer   |
+------------------------------+
```

---

## Layer Responsibilities

### UI

Responsible for presenting information to the user and handling user interactions.

The UI should remain independent of business logic and communicate only with the Application Layer.

Typical responsibilities include:

- application settings
- connection status
- debugging tools
- diagnostics

---

### Application Layer

Coordinates the overall application lifecycle.

Responsibilities include:

- startup
- shutdown
- configuration loading
- service initialization
- application coordination

The Application Layer orchestrates communication between the UI and backend services without implementing service-specific behavior.

---

### Services Layer

Contains independent services responsible for gathering information from the host system.

Examples include:

- system monitoring
- Git integration
- notification collection
- clipboard monitoring
- media controls

Services should remain independent and communicate through well-defined interfaces coordinated by the Application Layer whenever possible.

---

### Transport & Protocol Layer

Responsible for communication with the PSP.

Responsibilities include:

- packet serialization
- packet transmission
- protocol version management
- transport abstraction

This layer hides transport-specific implementation details from higher layers.

Application services communicate with the PSP exclusively through this layer.

---

### Platform Integration Layer

Provides controlled access to operating system functionality.

Examples include:

- process information
- hardware information
- operating system APIs
- filesystem access

Higher layers should access operating system functionality exclusively through this layer.

---

## Layer Dependency Rules

The Desktop Companion follows strict dependency rules.

- Layers communicate only with adjacent layers.
- Dependencies always flow downward.
- Lower layers must never depend on higher layers.
- Circular dependencies are not permitted.

This architecture preserves modularity and simplifies testing and future refactoring.

---

## Design Principles

The Desktop Companion follows these principles:

- single responsibility
- loose coupling
- dependency inversion
- explicit interfaces
- protocol isolation

---

## Out of Scope

This document does not define:

- packet formats
- threading model
- implementation details
- service implementations
- plugin APIs

These topics are documented separately.

---

End of Document

