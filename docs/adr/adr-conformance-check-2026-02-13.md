# ADR Conformance Check (2026-02-13)

This review checks current implementation status against ADRs in `docs/adr`.

## Summary

- Overall status: **Mostly aligned**
- High-risk gaps: **none blocking current Phase 0-6 outcomes**
- Residual gaps: **testing depth and distro breadth (already queued in Phase 7/8)**

## Detailed check

1. ADR 0001 (Linux-native target)
   - Status: aligned
   - Evidence: `docs/compatibility-target.md`, Linux Qt shell in `ui/qt/`

2. ADR 0002 (Qt backend)
   - Status: aligned
   - Evidence: `docs/ui-decision.md`, Qt targets in `CMakeLists.txt`

3. ADR 0003 (platform boundary)
   - Status: aligned
   - Evidence: `platform/include/`, `platform/linux/`, boundary workflow

4. ADR 0004 (CMake + Windows coexistence)
   - Status: aligned
   - Evidence: Linux CMake path + existing Windows workflows still present

5. ADR 0005 (plugin strategy)
   - Status: aligned
   - Evidence: `docs/plugin-strategy.md` (native Linux API direction)

6. ADR 0006 (packaging/release)
   - Status: aligned with caveats
   - Evidence: `packaging/arch/PKGBUILD`, release workflow and artifact script
   - Caveat: signed artifact path is optional unless GPG secrets are configured in CI

## Follow-up recommendations

- Keep ADR evidence links updated whenever implementation files move.
- Add test coverage for encoding, session, and lexer/theme behavior in Phase 7.
- Add non-Arch packaging strategy ADR(s) when distro support expands beyond Manjaro-first.
