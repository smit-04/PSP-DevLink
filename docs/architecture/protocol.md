# Communication Protocol

## Purpose

This document defines the shared communication protocol used by the Desktop Companion and the PSP application.

This milestone establishes only the communication foundation.

It does not implement:

* runtime data transfer
* packet serialization
* message processing

USB transport is implemented independently of the protocol layer through platform-specific transport backends.

---

## Design Goals

The protocol shall be:

* transport independent
* versioned
* extensible
* platform independent
* deterministic

---

## Module Structure

```text
shared/
└── protocol/
    ├── include/
    │   └── protocol/
    │       └── transport.h
    └── CMakeLists.txt
```

The shared protocol module exports only the public protocol interfaces.

Platform-specific transport implementations are provided by each application:

```text
apps/
├── desktop/
│   └── src/
│       └── transport_usb.cpp
└── psp/
    └── src/
        └── transport_usb.c
```

---

## Layering

```text
Application
    │
    ▼
Protocol
    │
    ▼
Transport Interface
    │
    ▼
Platform Transport Backend
```

The shared protocol layer owns only the transport interface.

Desktop and PSP each provide their own transport implementation while exposing the same public interface, allowing the protocol layer to remain completely transport-independent.

Milestone 4 establishes the transport architecture. Protocol handshaking, packet serialization, session management, and message processing will be implemented in later milestones.
