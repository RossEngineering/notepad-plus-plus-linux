# ADR 0004: CMake Linux Build Path While Keeping Windows Paths

- Status: Accepted
- Date: 2026-02-13
- Decision Makers: project maintainers
- Related: `CMakeLists.txt`, `CMakePresets.json`

## Context

Linux-native work needed a modern, testable build path without breaking existing Windows
build workflows.

## Decision

Adopt top-level CMake presets/targets for Linux while preserving existing Windows Visual Studio
and MinGW paths during transition.

## Consequences

### Positive

- Enables Linux CI and local iteration.
- Minimizes disruption to existing Windows contributors.

### Negative

- Temporary dual-build-system maintenance overhead.

## Alternatives Considered

1. Replace all build paths immediately with one new build system.
2. Keep only legacy Windows build until late migration.

## Implementation Notes

- Scintilla, Lexilla, platform services, and Qt shell are now CMake targets.

## Evidence

- `CMakeLists.txt`
- `CMakePresets.json`
- `.github/workflows/linux-cmake.yml`
