# notepad-plus-plus-linux

Linux-native fork and reengineering effort based on Notepad++.

This repository tracks a transition from the original Windows-first architecture to a native Linux application, targeting Arch Linux and derivatives (including Manjaro, used as the baseline environment).

## Project status

Current state:

- Repository status: **Incubator** (intentional migration stage).
- Linux-native Qt shell builds and runs on Arch Linux derivatives (Manjaro baseline).
- Foundation migration phases are complete (`0-12`).
- Release train progress:
  - `v0.8.0-beta.1` published.
  - `v0.9.0-beta.2` published.
  - `v0.9.3-rc.1` published.
  - `v0.9.6-rc.2` published.
- Next target: `v0.9.9-rc.3`, then `v1.0.0`.
- Upstream codebase heritage remains Windows-centric; Linux-first migration is still active.

Planned state:

- Native Linux desktop app (no Wine dependency).
- Modernized architecture with clear separation between editor core, platform services, and UI.
- Linux-first build, packaging, and release process.

See [`TODO.md`](TODO.md) and [`docs/roadmap.md`](docs/roadmap.md) for active sequencing.

## Migration and release dashboard

| Milestone | Scope | Status | Primary Artifact |
| --- | --- | --- | --- |
| Foundation (Phases `0-8`) | Baseline, architecture, build, platform abstraction, Linux UI, packaging, quality, docs | Complete | `docs/todo-archive-phases-0-8.md` |
| Beta 1 (`v0.8.0-beta.1`) | Phase `9`: language detection + syntax-highlighting quality gates | Complete | `docs/releases/v0.8.0-beta.1.md` |
| Beta 2 (`v0.9.0-beta.2`) | Phases `10-12`: skinning, extension platform, hardening/language intelligence | Complete | `docs/releases/v0.9.0-beta.2.md` |
| RC1 (`v0.9.3-rc.1`) | Stabilization baseline, distro matrix baseline, extension permission hardening | Complete | `docs/releases/v0.9.3-rc.1.md` |
| RC2 (`v0.9.6-rc.2`) | LSP baseline UX wiring, compatibility expansion, CI performance gates, distro install docs | Complete / Live | `docs/releases/v0.9.6-rc.2.md` |
| RC3 (`v0.9.9-rc.3`) | Feature freeze, full regression, release dry-run, blocker closure | In progress | `TODO.md` |
| GA (`v1.0.0`) | Final go/no-go and production release | Planned | `docs/releases/v1.0.0-checklist.md` |

## Key documentation

- Roadmap and active backlog:
  - [`TODO.md`](TODO.md)
  - [`docs/roadmap.md`](docs/roadmap.md)
- Build, install, and release:
  - [`BUILD.md`](BUILD.md)
  - [`docs/install-linux.md`](docs/install-linux.md)
  - [`docs/release-publishing.md`](docs/release-publishing.md)
  - [`docs/reproducible-release.md`](docs/reproducible-release.md)
- Architecture and compatibility:
  - [`docs/architecture.md`](docs/architecture.md)
  - [`docs/compatibility-target.md`](docs/compatibility-target.md)
  - [`docs/plugin-strategy.md`](docs/plugin-strategy.md)
  - [`docs/extension-api-v1.md`](docs/extension-api-v1.md)
- Design decisions and policy:
  - [`docs/decisions.md`](docs/decisions.md)
  - [`docs/adr`](docs/adr)
  - [`docs/versioning-policy.md`](docs/versioning-policy.md)
- User support:
  - [`docs/help-and-support.md`](docs/help-and-support.md)
  - [`SECURITY.md`](SECURITY.md)

## Goals

- Preserve Notepad++ editing strengths (performance, usability, language tooling).
- Replace Win32-only dependencies with cross-platform or Linux-native implementations.
- Make Arch Linux derivatives the reference target for development and packaging, with Manjaro as the baseline.

## Repository layout (high level)

- `PowerEditor/`: current Notepad++ application code (Windows-oriented).
- `scintilla/`: Scintilla editing component.
- `lexilla/`: Lexers and syntax highlighting support.
- `boostregex/`: bundled Boost regex subset used by current build.

## Build notes

Linux-first build and test instructions are in [`BUILD.md`](BUILD.md).
Validated distro install guidance is in [`docs/install-linux.md`](docs/install-linux.md).
For one-command Arch-family developer setup (validated on Manjaro), see [`docs/dev-setup-manjaro.md`](docs/dev-setup-manjaro.md).

## Contributing

Contributions are welcome. Please follow [`CONTRIBUTING.md`](CONTRIBUTING.md).

For migration priorities and sequencing, check [`TODO.md`](TODO.md).

## License

This project remains under the GPL. See [`LICENSE`](LICENSE).
