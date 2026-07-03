# Coding Standards

## Purpose

The Coding Standards define the engineering practices used throughout PSP DevLink.

Their objectives are to:

- improve readability
- improve maintainability
- encourage consistent implementation
- reduce architectural drift
- support long-term development

These standards complement the Project Foundation and apply to all project source code.

---

## General Principles

All code should prioritize:

- clarity over cleverness
- simplicity over unnecessary complexity
- explicit behavior over implicit behavior
- maintainability over short-term convenience

Every module should have a single, well-defined responsibility.

---

## Naming

Names should be:

- descriptive
- consistent
- unambiguous
- appropriate to their scope

Avoid unexplained abbreviations.

Use consistent terminology throughout the project.

---

## Functions

Functions should:

- perform a single responsibility
- remain reasonably small
- avoid hidden side effects
- expose clear inputs and outputs
- document non-obvious behavior

Large functions should be decomposed into smaller units where practical.

---

## State Management

Application state should follow these principles:

- minimize global mutable state
- define clear ownership of shared data
- maintain a single source of truth
- avoid duplicate representations of the same information

State ownership should be documented whenever shared between modules.

---

## Module Design

Modules should:

- expose well-defined interfaces
- minimize dependencies
- avoid circular dependencies
- communicate through explicit interfaces

Implementation details should remain internal to the module whenever possible.

---

## Error Handling

Errors should:

- be detected early
- be handled explicitly
- provide useful diagnostic information
- never fail silently

Unexpected failures should leave the application in a predictable state.

---

## Documentation

Public interfaces should be documented.

Architecture documentation should remain synchronized with implementation.

Significant architectural changes must be reflected in the appropriate documentation before a milestone is considered complete.

---

## Code Reviews

Before accepting changes, verify:

- architecture consistency
- coding standard compliance
- readability
- maintainability
- documentation updates
- successful verification

No implementation should be considered complete until it has been verified.

---

## Out of Scope

This document does not define:

- language-specific style rules
- formatting tools
- compiler configuration
- repository workflow

These topics are documented separately where applicable.

---

End of Document
