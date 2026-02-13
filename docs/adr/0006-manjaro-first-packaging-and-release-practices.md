# ADR 0006: Arch-Derivatives-First (Manjaro Baseline) Packaging and Signed Release Practices

- Status: Accepted
- Date: 2026-02-13
- Decision Makers: project maintainers
- Related: `packaging/arch/PKGBUILD`, `.github/workflows/linux-release.yml`

## Context

Project goals specify Arch Linux derivative distribution first, with Manjaro as the baseline environment, and reproducible release behavior.

## Decision

Adopt Arch Linux derivative packaging artifacts and release automation with checksums and optional GPG
signing as the initial distribution path.

## Consequences

### Positive

- Provides immediate local package workflow for target users.
- Establishes reproducible artifact and signing pipeline.

### Negative

- Other distro packaging remains deferred.
- Release quality depends on CI secret management for signing.

## Alternatives Considered

1. Delay packaging until after full parity.
2. Start with multiple distro packages simultaneously.

## Implementation Notes

- Runtime package path implemented first; split strategy documented for debug/plugins.

## Evidence

- `packaging/arch/PKGBUILD`
- `packaging/arch/notepad-plus-plus-linux.install`
- `docs/package-split-strategy.md`
- `docs/reproducible-release.md`
- `docs/release-publishing.md`
- `.github/workflows/linux-release.yml`
