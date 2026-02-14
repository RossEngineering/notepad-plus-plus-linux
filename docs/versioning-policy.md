# Versioning and Compatibility Policy

> **Linux-only fork notice:** This repository and its releases target Linux only. For the original Windows Notepad++ application, visit [notepad-plus-plus.org](https://notepad-plus-plus.org/).

Last updated: 2026-02-14

## Current release phase

This repository is in pre-1.0 migration status.

- Recommended version range: `0.x.y`
- Stable tag format: `v<major>.<minor>.<patch>`
- Pre-release tag format: `v<major>.<minor>.<patch>-<channel>.<n>`
  - Example: `v0.8.0-beta.1`
- Maintenance respin suffixes are allowed for late RC fixes when needed:
  - Example: `v0.9.9-rc.3a`

## Current release direction

- Final alpha release: `v0.7.0-alpha.1`
- No further alpha releases planned.
- Latest beta release: `v0.9.0-beta.2` (Phases 10-12 complete).
- RC train:
  - `v0.9.3-rc.1` (RC1)
  - `v0.9.6-rc.2` (RC2)
  - `v0.9.9-rc.3` (RC3)
- RC3a maintenance respin: `v0.9.9-rc.3a` (published).
- RC3b maintenance respin: `v0.9.9-rc.3b` (published).
- Development prerelease section close: `v0.10.0-beta.3` (Linux integration/distribution).
- Latest development prerelease (ongoing dev branch): `v0.10.0-beta.4` (RPM packaging maintenance fix for beta.3 artifacts).
- RC3 feature freeze remains the base policy: `docs/feature-freeze-rc3.md`
- `v1.0.0` is targeted after satisfactory RC3 maintenance validation.

## Version increment rules

- Patch (`x.y.Z`): bug fixes and non-breaking maintenance.
- Minor (`x.Y.z`): additive features and meaningful behaviour improvements.
- Major (`X.y.z`): reserved for explicit compatibility resets after 1.0.
- Pre-release channels:
  - `alpha`: early internal milestones (currently closed out).
  - `beta`: broader validation milestone before release candidates.
  - `rc`: release candidate stabilization before final.
- TODO-section cadence rule:
  - when a planned TODO section is completed, publish a tagged prerelease and corresponding checklist/release notes update before starting the next section.

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
