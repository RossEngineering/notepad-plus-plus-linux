# Plugin Strategy (Linux)

Last updated: 2026-02-13
Status: Accepted for MVP

Detailed Phase 11 API boundary and permission model: `docs/extension-api-v1.md`.

## Decision

For Linux MVP, this fork will use a **new Linux-native plugin API** and will **not** provide a Windows ABI compatibility shim.

## Why this decision

1. Existing Notepad++ plugin contracts are tightly coupled to Win32 message handling and DLL loading semantics.
2. A compatibility shim would add major complexity and instability during core migration.
3. MVP priority is a stable native editor on Linux for Arch Linux derivatives (Manjaro baseline), not binary plugin parity.
4. A native API allows safer process boundaries and clearer long-term maintenance.

## MVP scope

- No support for loading Windows `.dll` plugins.
- No promise of drop-in compatibility with existing Windows plugins.
- Keep plugin hooks minimal in MVP while core editing workflows stabilize.

## Native API principles (post-MVP implementation target)

- UTF-8-first interfaces.
- Explicit capability-based host API (documents, selections, commands, notifications).
- Versioned ABI surface with compatibility checks at load time.
- Sandboxed execution model preference:
  - out-of-process plugins by default for crash isolation,
  - optional in-process extension path only where justified.

## Migration guidance for plugin authors

- Treat Linux plugins as a new target.
- Port business logic first; isolate platform-specific UI and command code.
- Prefer host-command and data APIs over direct UI internals.

## Re-evaluation trigger

Revisit only if one of the following becomes true:

1. A low-risk shim path emerges with measurable stability and maintenance cost.
2. Enterprise adoption requires a temporary compatibility bridge.
3. The native API implementation reaches feature parity and can absorb a translation layer safely.
