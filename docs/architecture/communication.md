# Communication Architecture

## Purpose

The Communication Architecture defines how the Desktop Companion and the PSP exchange information while remaining independent of each other's internal implementation.

Its objectives are to:

- provide a stable communication interface
- isolate communication from application logic
- support future protocol evolution
- minimize coupling between system components
- establish clear ownership of transmitted data

This document defines architectural communication principles rather than protocol implementation details.

---

## Communication Model

PSP DevLink follows a client-server communication model.

The Desktop Companion acts as the authoritative source of host system information.

The PSP acts as a presentation-oriented client responsible for displaying information and forwarding user-initiated commands.

Neither application depends on the internal implementation of the other.

---

## High-Level Data Flow

```text
                 User Input
                      │
                      ▼
+----------------------------------+
|               PSP                |
|                                  |
|  UI                              |
|  Presentation State              |
|  Packet Processing               |
+----------------------------------+
              │
      Commands │
              ▼
====================================
       Communication Layer
====================================
              ▲
        Updates │
              │
+----------------------------------+
|        Desktop Companion         |
|                                  |
|  Services                        |
|  Protocol Encoder                |
|  Transport                       |
+----------------------------------+
```

---

## Communication Responsibilities

### Desktop Companion

The Desktop Companion owns:

- host system information
- service execution
- packet generation
- connection management

It is the single authoritative source of external system data.

---

### PSP

The PSP owns:

- presentation state
- user interface
- navigation state
- rendering
- user input

The PSP does not own host system information.

---

## Data Ownership Rules

PSP DevLink follows strict ownership rules.

- Host system information is owned exclusively by the Desktop Companion.
- Presentation state is owned exclusively by the PSP.
- Commands originate from the PSP.
- Data updates originate from the Desktop Companion.
- Shared mutable state between applications is not permitted.

These rules eliminate conflicting sources of truth and simplify synchronization.

---

## Communication Principles

The communication layer follows these principles:

- protocol independent of implementation
- transport abstraction
- explicit message ownership
- versionable protocol
- communication isolated from application logic
- presentation isolated from transport

---

## Reliability

The communication architecture is designed to tolerate:

- temporary disconnections
- corrupted or invalid packets
- protocol mismatches
- application restarts

Communication failures should be detected, reported, and handled gracefully without causing either application to terminate unexpectedly.

---

## Versioning

The communication protocol should expose explicit version information.

Future protocol revisions must detect incompatible versions before exchanging application data.

Compatibility policies belong to the protocol specification rather than individual application modules.

---

## Architectural Constraints

The communication layer must satisfy the following constraints:

- Application modules must never communicate directly across devices.
- All inter-device communication passes through the Communication Layer.
- Transport implementation must remain replaceable without affecting higher-level modules.
- Communication components must not contain presentation logic.

These constraints preserve modularity and reduce long-term maintenance costs.

---

## Out of Scope

This document does not define:

- packet layouts
- binary encoding
- USB implementation
- serialization
- checksum algorithms
- protocol implementation details

These topics belong to the protocol specification.

---

End of Document
