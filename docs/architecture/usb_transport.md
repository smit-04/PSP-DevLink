# USB Transport Architecture

Status: Draft
Milestone: 4

---

## Purpose

This document defines the USB transport layer used by PSP DevLink.

The transport layer is responsible only for reliable raw byte transfer.

It is **not** responsible for:

- packet parsing
- protocol versioning
- message processing
- session management
- application logic

---

## Layering

Application
    │
    ▼
Protocol
    │
    ▼
Transport Interface
    │
    ▼
USB Transport Backend
    │
    ▼
PSP USB Driver

---

## Responsibilities

Desktop

- Initialize USB transport
- Send raw bytes
- Receive raw bytes
- Shutdown transport

PSP

- Register USB driver
- Initialize USB subsystem
- Send raw bytes
- Receive raw bytes
- Shutdown USB subsystem

---

## Planned Implementation

The PSP transport backend will implement:

- transport_initialize()
- transport_shutdown()
- transport_send()
- transport_receive()

The Desktop transport backend will expose the same interface.

---

## Milestone Scope

Milestone 4 implements only:

- USB initialization
- USB shutdown
- Raw byte transmission
- Raw byte reception

Future milestones will implement:

- protocol handshake
- packet framing
- serialization
- message routing
- desktop services