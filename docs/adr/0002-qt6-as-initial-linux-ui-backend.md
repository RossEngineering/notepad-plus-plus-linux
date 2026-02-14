# ADR 0002: Qt 6 as Initial Linux UI Backend

> **Linux-only fork notice:** This repository and its releases target Linux only. For the original Windows Notepad++ application, visit [notepad-plus-plus.org](https://notepad-plus-plus.org/).

- Status: Accepted
- Date: 2026-02-13
- Decision Makers: project maintainers
- Related: `docs/ui-decision.md`

## Context

We needed a practical, stable path to deliver a Linux-native UI shell quickly.

## Decision

Use Qt 6 as the first stable Linux UI backend.

## Consequences

### Positive

- Existing Scintilla Qt integration shortens implementation path.
- Mature Linux desktop APIs for dialogs/input/menus.

### Negative

- Adds Qt runtime dependency.
- Defers evaluation of GTK/wx alternatives.

## Alternatives Considered

1. GTK-based shell.
2. wxWidgets-based shell.

## Implementation Notes

- Shell target built as `npp_linux_shell`/`notepad-plus-plus-linux` via CMake.

## Evidence

- `docs/ui-decision.md`
- `CMakeLists.txt`
- `ui/qt/MainWindow.cpp`
