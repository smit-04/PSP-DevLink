# PROJECT_STATE.md

Version: 1.0
Status: ACTIVE

---

# Project

PSP DevLink

---

# Current Milestone

Milestone 0 — Development Environment

Status: COMPLETED

---

# Objective

Establish a fully verified and reproducible PSP homebrew development environment before beginning feature development.

---

# Verified Environment

## Host OS

- Windows

## Linux Environment

- WSL2
- Ubuntu 24.04.4 LTS

## Development Tools

- Git
- GNU Make
- PSPDEV Toolchain
- PSPSDK
- psp-gcc

## Editor

- Visual Studio Code
- Remote - WSL Extension

## Emulator

- PPSSPP (Windows)

---

# Repository

Repository Name

PSP-DevLink

Branch

main

Remote

GitHub (SSH)

Repository Status

- Initial commit completed
- Remote configured
- Push verified
- Working tree clean

---

# Verified Workflow

Source Code

↓

Compile using PSP SDK

↓

Generate ELF

↓

Generate EBOOT.PBP

↓

Launch in PPSSPP

↓

Rendering Verified

↓

Controller Input Verified

↓

Clean Exit Verified

---

# Repository Structure

PSP-DevLink/

docs/

src/

.gitignore

Makefile

README.md

---

# Smoke Test

Current smoke test verifies:

- PSP SDK
- Rendering
- Controller initialization
- Main loop
- Clean application exit

---

# Next Milestone

Milestone 1

Objective:

Design the complete software architecture before implementing features.