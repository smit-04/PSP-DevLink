# Communication Protocol

## Purpose

This document defines the shared communication protocol used by the Desktop Companion and the PSP application.

This milestone establishes only the communication foundation.

It does not implement:

- USB communication
- runtime data transfer
- packet serialization
- message processing

---

## Design Goals

The protocol shall be:

- transport independent
- versioned
- extensible
- platform independent
- deterministic

---

## Module Structure

shared/
└── protocol/
    ├── include/
    │   └── protocol/
    └── src/

The protocol module exposes a shared public interface that can be used by both the Desktop Companion and the PSP application.

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
Transport Implementation

Milestone 3 provides only stub transport implementations.

Actual USB communication will be implemented in a later milestone.