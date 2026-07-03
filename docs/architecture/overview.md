# PSP DevLink Architecture Overview

## Purpose

PSP DevLink transforms a Sony PSP into a USB-connected developer companion for a desktop computer.

The desktop application gathers information from the host computer and transmits it to the PSP using a custom binary protocol. The PSP renders this information through a lightweight user interface designed for real-time monitoring.

The architecture emphasizes:

- Separation of concerns
- Maintainability
- Extensibility
- Testability
- Clear module ownership

---

# High-Level Architecture

```
+------------------------------------------------------+
|                  Desktop Computer                    |
|                                                      |
|  System Services                                     |
|  Device Manager                                      |
|  Git Integration                                     |
|  Notification Service                                |
|  Terminal Service                                    |
|                                                      |
|            │                                         |
|            ▼                                         |
|      Protocol Encoder                               |
|            │                                         |
|            ▼                                         |
|         USB Transport                               |
+------------------------------------------------------+

                    USB Binary Protocol

+------------------------------------------------------+
|                    Sony PSP                          |
|                                                      |
| USB Driver                                           |
| Protocol Decoder                                     |
| Packet Dispatcher                                    |
| UI System                                            |
| Graphics Engine                                      |
| Scene Manager                                        |
|                                                      |
+------------------------------------------------------+
```

---

# Major Components

The project consists of two independent applications.

## Desktop Companion

Runs on the host computer.

Responsibilities include:

- Collecting system information
- Managing communication
- Encoding protocol packets
- Sending updates to the PSP

---

## PSP Client

Runs on the PSP.

Responsibilities include:

- Receiving packets
- Decoding messages
- Maintaining local state
- Rendering the user interface
- Handling user input

---

## Shared Protocol

The communication protocol forms the contract between the desktop application and the PSP.

Both applications depend on the protocol specification but remain otherwise independent.

---

# Architectural Principles

The project follows these principles.

## Layered Design

Each subsystem has a single responsibility.

---

## Loose Coupling

Modules communicate through interfaces rather than direct implementation knowledge.

---

## Protocol First

Communication is defined by documented packet structures rather than implementation details.

---

## Documentation Driven

Architecture documentation is produced before feature implementation.

---

## Repository as Source of Truth

The repository contains all documentation required to understand and continue development.

No chat history should be required.

---

# Documentation Structure

Additional architecture documents describe each subsystem in detail.

- repository.md
- desktop.md
- psp.md
- communication.md
- build.md
- coding_standards.md

---

# Out of Scope

This document intentionally does not describe:

- packet formats
- rendering implementation
- threading
- graphics
- plugins
- feature implementation

These are documented separately.

---

End of Document
