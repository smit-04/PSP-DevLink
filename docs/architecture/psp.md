# PSP Application Architecture

## Purpose

The PSP application is responsible for presenting information received from the Desktop Companion while providing a responsive user interface for local interaction.

The PSP is a presentation-oriented client. It performs rendering, local input handling, and presentation state management, while the Desktop Companion remains the authoritative source of host system information.

---

## Architectural Style

The PSP application follows a single-threaded, frame-driven architecture centered around a deterministic main loop.

Each frame executes a fixed sequence of operations to ensure predictable behavior, stable frame timing, and consistent rendering performance.

---

## High-Level Architecture

```text
+---------------------------+
|      Main Application     |
+---------------------------+
            │
            ▼
+---------------------------+
|        Main Loop          |
+---------------------------+
            │
            ▼
+---------------------------+
|      Input Manager        |
+---------------------------+
            │
            ▼
+---------------------------+
|    Packet Processing      |
+---------------------------+
            │
            ▼
+---------------------------+
|      State Manager        |
+---------------------------+
            │
            ▼
+---------------------------+
|   Scene & UI Rendering    |
+---------------------------+
```

---

## Component Responsibilities

### Main Application

Responsible for application startup, initialization, configuration, and shutdown.

It initializes all major subsystems before entering the main loop and coordinates orderly shutdown when the application exits.

---

### Main Loop

The Main Loop coordinates execution of every application frame.

Each iteration performs the following sequence:

1. Read user input.
2. Process incoming communication.
3. Update presentation state.
4. Render the current scene.
5. Synchronize the display.

The execution order remains consistent throughout the application's lifetime.

---

### Input Manager

The Input Manager processes PSP controller input and converts button events into application actions.

Input handling remains independent of rendering and presentation logic.

---

### Packet Processing

The Packet Processing component receives, validates, and decodes data received from the Desktop Companion.

Only validated packets are allowed to update the application state.

Communication-specific logic remains isolated within this component.

---

### State Manager

The State Manager owns all presentation state maintained by the PSP.

Responsibilities include:

- storing the latest received system information
- maintaining UI navigation state
- providing read access to rendering components

The State Manager is the single source of truth for application state on the PSP.

---

### Scene & UI Rendering

Responsible for rendering the user interface using the current application state.

Rendering reads state but never modifies it.

Presentation logic remains isolated from communication and input processing.

---

## Component Dependency Rules

The PSP application follows strict dependency rules.

- Rendering depends only on the State Manager.
- The State Manager does not depend on rendering.
- Input handling does not directly modify rendering.
- Packet Processing updates state but does not perform rendering.
- Communication remains isolated from presentation logic.

These rules prevent circular dependencies and preserve a predictable execution flow.

---

## Design Principles

The PSP application follows these principles:

- deterministic execution
- single ownership of state
- rendering separated from logic
- communication isolated from presentation
- explicit module responsibilities
- predictable frame execution

---

## Out of Scope

This document does not define:

- graphics implementation
- rendering engine internals
- communication protocol
- packet formats
- memory allocation strategy
- plugin architecture

These topics are documented separately.

---

End of Document
