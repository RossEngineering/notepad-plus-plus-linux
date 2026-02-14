# Decisions Log

> **Linux-only fork notice:** This repository and its releases target Linux only. For the original Windows Notepad++ application, visit [notepad-plus-plus.org](https://notepad-plus-plus.org/).

This file indexes architecture and governance decisions used by this repository.

## Core project decisions

- UI backend decision: `docs/ui-decision.md`
- Compatibility target: `docs/compatibility-target.md`
- Plugin strategy: `docs/plugin-strategy.md`
- Extension API v1 boundaries/security model: `docs/extension-api-v1.md`
- Phase 11 implementation status: `docs/phase11-implementation.md`
- Package split strategy: `docs/package-split-strategy.md`
- Reproducible release approach: `docs/reproducible-release.md`
- Release publishing/signing flow: `docs/release-publishing.md`
- GA blocker tracker: `docs/ga-blockers.md`
- GA CI continuity log: `docs/ga-ci-continuity-log-2026-02.md`
- GA gate exception for `v1.0.0` continuity veto: `docs/ga-gate-exception-2026-02-14.md`
- RC4 consolidation release + incubator exit: `docs/releases/v0.10.0-rc.4.md`

## ADR records (repo-local)

- ADR index: `docs/adr/README.md`
- Linux target model: `docs/adr/0001-linux-native-target-and-compatibility-model.md`
- Qt backend selection: `docs/adr/0002-qt6-as-initial-linux-ui-backend.md`
- Platform boundary policy: `docs/adr/0003-platform-boundary-and-win32-containment.md`
- Build strategy: `docs/adr/0004-cmake-linux-build-path-with-windows-coexistence.md`
- Plugin compatibility direction: `docs/adr/0005-linux-native-plugin-api-no-windows-abi-shim-for-mvp.md`
- Packaging/release strategy: `docs/adr/0006-manjaro-first-packaging-and-release-practices.md` (Arch-derivatives-first, Manjaro baseline)
- VS Code compatibility strategy: `docs/adr/0007-vscode-compatibility-targeted-language-assets-over-full-api-parity.md`

## External reference baseline

- Imported RossEngineering ADR snapshot: `docs/adr/rossengineering-bafc82c7/`
- Alignment review against that snapshot: `docs/adr/rossengineering-alignment-2026-02-13.md`
- Incubator promotion validation: `docs/incubator-promotion-validation-2026-02-13.md`
- Incubator promotion re-validation (post-RC3): `docs/incubator-promotion-validation-2026-02-14.md`
- Incubator promotion decision execution (RC4): `docs/releases/v0.10.0-rc.4.md`
