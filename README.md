# notepad-plus-plus-linux

Linux-native fork and reengineering effort based on Notepad++.

This repository tracks a transition from the original Windows-first architecture to a native Linux application, with Manjaro as the primary target environment.

## Project status

Current state:

- Codebase is largely upstream Notepad++ and remains Windows-centric.
- Build and platform docs still describe Windows and MinGW workflows.
- Linux-native application shell is not implemented yet.

Planned state:

- Native Linux desktop app (no Wine dependency).
- Modernized architecture with clear separation between editor core, platform services, and UI.
- Linux-first build, packaging, and release process.

See [`TODO.md`](TODO.md) for the migration roadmap and active work items.
Plugin direction for Linux is documented in [`docs/plugin-strategy.md`](docs/plugin-strategy.md).
Package split plan is documented in [`docs/package-split-strategy.md`](docs/package-split-strategy.md).
Reproducible Linux release guidance is in [`docs/reproducible-release.md`](docs/reproducible-release.md).

## Goals

- Preserve Notepad++ editing strengths (performance, usability, language tooling).
- Replace Win32-only dependencies with cross-platform or Linux-native implementations.
- Make Manjaro the reference distribution for development and packaging.

## Repository layout (high level)

- `PowerEditor/`: current Notepad++ application code (Windows-oriented).
- `scintilla/`: Scintilla editing component.
- `lexilla/`: Lexers and syntax highlighting support.
- `boostregex/`: bundled Boost regex subset used by current build.

## Build notes

The existing build documentation in [`BUILD.md`](BUILD.md) is still upstream-oriented and primarily Windows focused.

As Linux-native build support is introduced, this README and `BUILD.md` will be updated with:

- Linux dependency list
- CMake presets / build commands
- Manjaro packaging instructions

## Contributing

Contributions are welcome. Please follow [`CONTRIBUTING.md`](CONTRIBUTING.md).

For migration priorities and sequencing, check [`TODO.md`](TODO.md).

## License

This project remains under the GPL. See [`LICENSE`](LICENSE).

