# Versioning and Compatibility Policy

Last updated: 2026-02-13

## Current release phase

This repository is in pre-1.0 migration status.

- Recommended version range: `0.x.y`
- Use tag format: `v<major>.<minor>.<patch>`

## Version increment rules

- Patch (`x.y.Z`): bug fixes and non-breaking maintenance.
- Minor (`x.Y.z`): additive features and meaningful behaviour improvements.
- Major (`X.y.z`): reserved for explicit compatibility resets after 1.0.

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
