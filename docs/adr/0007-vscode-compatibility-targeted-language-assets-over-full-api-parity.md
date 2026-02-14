# ADR 0007: VS Code Compatibility via Targeted Language Assets, Not Full API Parity

> **Linux-only fork notice:** This repository and its releases target Linux only. For the original Windows Notepad++ application, visit [notepad-plus-plus.org](https://notepad-plus-plus.org/).

- Status: Accepted
- Date: 2026-02-13
- Decision Makers: project maintainers
- Related: `docs/extension-api-v1.md`, `docs/plugin-strategy.md`

## Context

Phase 11 requires a VS Code compatibility strategy that supports extension value for users
without pulling in the full VS Code extension-host surface.

Two realistic options were evaluated:

1. full API parity with VS Code extension APIs,
2. targeted compatibility for language assets (TextMate grammars + language configs).

## Decision

Adopt targeted compatibility in v1:

1. parse VS Code `package.json` language/grammar contributions,
2. import TextMate grammar references and language configuration metadata,
3. normalize into project-owned compatibility snapshot format.

Do not attempt full VS Code API parity in Beta 2 scope.

## Rationale

1. Full API parity would require high-risk host emulation and large compatibility surface.
2. Language assets provide immediate user-visible value (highlighting/language metadata).
3. Targeted compatibility aligns with current architecture and extension API v1 boundaries.
4. This path keeps security posture manageable by avoiding broad remote extension execution models.

## Consequences

### Positive

1. Faster delivery for language support expansion.
2. Lower maintenance burden than full API emulation.
3. Clear security boundaries remain enforceable.

### Negative

1. Most VS Code behavior-oriented extensions remain out of scope in v1.
2. Some users may expect broader compatibility than provided.

## Alternatives Considered

1. Full VS Code extension API parity now.
   - Rejected: too broad and high-risk for Beta 2.
2. No VS Code compatibility path.
   - Rejected: misses practical language-asset interoperability gains.

## Implementation Notes

1. Compatibility parser reads `contributes.languages` and `contributes.grammars`.
2. Import path validates referenced files exist.
3. Output is normalized as `npp-language-pack-vscode.json`.
4. Validation fixtures include at least three popular language extensions.

## Evidence

- `platform/linux/VscodeLanguageCompatibility.cpp`
- `tests/regression/vscode_language_asset_compatibility_test.cpp`
- `tests/fixtures/vscode-language-assets/`
