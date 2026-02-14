# Arch-Derivatives Build Status (Manjaro Baseline)

> **Linux-only fork notice:** This repository and its releases target Linux only. For the original Windows Notepad++ application, visit [notepad-plus-plus.org](https://notepad-plus-plus.org/).

Last updated: 2026-02-13
Host: Manjaro Linux (`x86_64`, kernel `6.18.8-1-MANJARO`)

## Goal

Capture what currently builds on the Arch-derivatives baseline environment (Manjaro) and what fails, with reproducible probe commands.

## Environment snapshot

- `g++ (GCC) 15.2.1 20260103`
- `GNU Make 4.4.1`
- `pkg-config gtk+-3.0` -> `3.24.51`
- `pkg-config Qt6Core` -> `6.10.1`
- `qmake` -> Qt `5.15.18`
- `cmake` -> not installed (`command not found`)
- `ninja` -> not installed (`command not found`)
- `clang++` -> not installed (`command not found`)

## Build probes and results

## 1) Lexilla static build

Command:

```sh
make -C lexilla/src -j$(nproc)
```

Result: `FAILED`

Primary error:

- `scintilla/include/Sci_Position.h:21:9: error: 'intptr_t' does not name a type`
- compiler note indicates missing include: `#include <cstdint>`

Observation:

- Toolchain is capable of compiling many translation units, but build stops on a header portability/compiler-compat issue.

## 2) Scintilla GTK build

Command:

```sh
make -C scintilla/gtk -j$(nproc)
```

Result: `FAILED`

Primary errors:

- missing pkg-config package expected by makefile: `gtk+-2.0`
- missing build tool: `glib-genmarshal`
- missing header: `glib.h`

Observation:

- A significant portion of Scintilla core sources compile first, then build fails in GTK-specific stages.

## 3) PowerEditor GCC path

Command:

```sh
make -C PowerEditor/gcc -j$(nproc)
```

Result: `FAILED`

Primary error:

- `/bin/sh: line 1: cmd: command not found`

Observation:

- This make path is still Windows-command oriented and not Linux-native.

## Current baseline conclusion

- The repo does not currently provide a Linux-native top-level build path.
- Portable code compiles partially, but multiple blockers stop full build:
  - compiler/header portability mismatch (`intptr_t` include)
  - GTK2-era dependency assumptions
  - Windows command usage in PowerEditor make flow

## Recommended next build-focused actions

1. Introduce top-level Linux CMake entrypoint (Phase 2 item).
2. Build `scintilla/src` + `lexilla` via Linux-first targets, independent of Windows makefiles.
3. Resolve/patch header compatibility issue in `scintilla/include/Sci_Position.h`.
4. Avoid GTK2 dependency path for first milestone; proceed with Qt-based UI integration plan.
