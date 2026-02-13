# notepad-plus-plus-linux

Linux-native fork and reengineering effort based on Notepad++.

This repository tracks a transition from the original Windows-first architecture to a native Linux application, with Manjaro as the primary target environment.

## Project status

Current state:

- Repository status: **Incubator** (intentional migration stage).
- Linux-native Qt shell builds and runs on Manjaro-class environments.
- Packaging and release automation foundation is implemented (Phase 6).
- Upstream codebase heritage remains Windows-centric; migration is ongoing.
- `v0.8.0-beta.1` scope and artifacts are prepared locally; publish/tag steps are pending.

Planned state:

- Native Linux desktop app (no Wine dependency).
- Modernized architecture with clear separation between editor core, platform services, and UI.
- Linux-first build, packaging, and release process.

See [`TODO.md`](TODO.md) for the migration roadmap and active work items.
Plugin direction for Linux is documented in [`docs/plugin-strategy.md`](docs/plugin-strategy.md).
Package split plan is documented in [`docs/package-split-strategy.md`](docs/package-split-strategy.md).
Reproducible Linux release guidance is in [`docs/reproducible-release.md`](docs/reproducible-release.md).
Release publishing and signing flow is in [`docs/release-publishing.md`](docs/release-publishing.md).
Architecture decisions are tracked in [`docs/adr`](docs/adr).
Imported RossEngineering ADR snapshot is in [`docs/adr/rossengineering-bafc82c7`](docs/adr/rossengineering-bafc82c7).
Versioning policy is in [`docs/versioning-policy.md`](docs/versioning-policy.md).
Repository decision index is in [`docs/decisions.md`](docs/decisions.md).
Performance benchmark baseline is in [`docs/performance-baseline.md`](docs/performance-baseline.md).
Security reporting guidance is in [`SECURITY.md`](SECURITY.md).

## Migration dashboard

| Phase | Focus | Status | Primary Artifact |
| --- | --- | --- | --- |
| Phase 0 | Baseline and audit | Complete | `docs/manjaro-build-status.md` |
| Phase 1 | Architecture boundaries | Complete | `docs/architecture.md` |
| Phase 2 | Build modernization | Complete | `CMakeLists.txt` |
| Phase 3 | Platform abstraction | Complete | `platform/include` |
| Phase 4 | Linux-native UI | Complete | `ui/qt` |
| Phase 5 | MVP parity | Complete | `docs/plugin-strategy.md` |
| Phase 6 | Packaging and distribution | Complete | `packaging/arch/PKGBUILD` |
| Phase 7 | Quality and performance | Complete | `docs/performance-baseline.md` |
| Phase 8 | Docs and developer UX | Complete | `docs/roadmap.md` |

Foundation progress: **8 / 8 foundation phases complete**.
Post-foundation phases (9+) are now tracked in `TODO.md` and `docs/roadmap.md`.

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

Linux-first build and test instructions are in [`BUILD.md`](BUILD.md).
For one-command Manjaro developer setup, see [`docs/dev-setup-manjaro.md`](docs/dev-setup-manjaro.md).

## Contributing

Contributions are welcome. Please follow [`CONTRIBUTING.md`](CONTRIBUTING.md).

For migration priorities and sequencing, check [`TODO.md`](TODO.md).

## License

This project remains under the GPL. See [`LICENSE`](LICENSE).
