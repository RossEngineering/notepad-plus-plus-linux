# Versioning and Compatibility Policy

> **Linux-only fork notice:** This repository and its releases target Linux only. For the original Windows Notepad++ application, visit [notepad-plus-plus.org](https://notepad-plus-plus.org/).

Last updated: 2026-02-13

## Current release phase

This repository is in pre-1.0 migration status.

- Recommended version range: `0.x.y`
- Stable tag format: `v<major>.<minor>.<patch>`
- Pre-release tag format: `v<major>.<minor>.<patch>-<channel>.<n>`
  - Example: `v0.8.0-beta.1`

## Current release direction

- Final alpha release: `v0.7.0-alpha.1`
- No further alpha releases planned.
- Latest beta release: `v0.9.0-beta.2` (Phases 10-12 complete).
- RC train:
  - `v0.9.3-rc.1` (RC1)
  - `v0.9.6-rc.2` (RC2)
  - `v0.9.9-rc.3` (RC3)
- RC3 feature freeze is active: `docs/feature-freeze-rc3.md`
- `v1.0.0` is targeted after satisfactory RC3 validation.

## Version increment rules

- Patch (`x.y.Z`): bug fixes and non-breaking maintenance.
- Minor (`x.Y.z`): additive features and meaningful behaviour improvements.
- Major (`X.y.z`): reserved for explicit compatibility resets after 1.0.
- Pre-release channels:
  - `alpha`: early internal milestones (currently closed out).
  - `beta`: broader validation milestone before release candidates.
  - `rc`: release candidate stabilization before final.

## Compatibility stance

- Behavioural compatibility with Linux MVP target is preferred.
- Breaking user-facing behaviour should not be introduced casually.
- Where breaking changes are necessary, document rationale and migration notes.

## Deprecation policy

- Prefer additive change over removal.
- Mark deprecated behaviour clearly in docs.
- Keep deprecated behaviour for a reasonable transition period unless there is
  a security or correctness reason for immediate removal.

## Related docs

- Compatibility target: `docs/compatibility-target.md`
- Plugin strategy: `docs/plugin-strategy.md`
- Decisions index: `docs/decisions.md`
