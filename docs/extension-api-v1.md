# Extension API v1 Boundaries and Security Model

> **Linux-only fork notice:** This repository and its releases target Linux only. For the original Windows Notepad++ application, visit [notepad-plus-plus.org](https://notepad-plus-plus.org/).

Last updated: 2026-02-14
Status: Accepted for Phase 11 implementation

## Purpose

Define the first Linux-native extension contract for this project:

1. what extension capabilities are available in v1,
2. what is explicitly out of scope,
3. how extensions are isolated and permission-gated.

This document is the implementation target for Phase 11 item:
`Define Linux extension API v1 boundaries and security model`.

## Scope (v1)

Extension API v1 supports three extension classes:

1. `language-pack`
   - Adds TextMate grammars and language configuration metadata.
   - No executable code required.
2. `command-plugin`
   - Adds user-invokable commands via host extension IPC.
   - Can read editor/document state through host APIs.
3. `tool-integration`
   - Declares external-tool command templates integrated with editor commands.

Extension API v1 also supports optional formatter contributions from executable
extensions (`command-plugin` or `tool-integration`) so extensions can provide
language-specific document formatting entrypoints.

## Non-goals (v1)

These are explicitly not part of Extension API v1:

1. Windows `.dll` plugin ABI compatibility.
2. In-process arbitrary native code loading by default.
3. Arbitrary host UI tree injection (custom native widgets inside main window).
4. Unrestricted filesystem/network/process access without permission grants.
5. Marketplace protocol compatibility in v1.

## Packaging Boundary

Each extension ships as a directory or archive with:

1. `extension.json` (required manifest, schema-backed)
2. `README.md` (required human docs)
3. assets according to `type`:
   - `language-pack`: `syntaxes/*.tmLanguage.json`, `language-configuration.json`
   - `command-plugin`: executable entrypoint plus metadata
   - `tool-integration`: command templates
4. optional formatter contribution metadata:
   - `formatters[].languages`: language IDs handled by the extension formatter
   - `formatters[].args`: argument template list for entrypoint invocation

Reference schema: `docs/schemas/extension-manifest-v1.schema.json`.
Practical author workflow guide: `docs/extension-authoring-guide.md`.

## Runtime Boundary

Host and extension boundary in v1:

1. Extensions run out-of-process by default.
2. Host-extension communication uses JSON-RPC over stdio.
3. Host owns lifecycle (start/stop/restart, crash detection, timeouts).
4. Host provides a capability-scoped API surface only.

## API Surface Boundary (v1)

Host API namespaces available to extensions:

1. `workspace.*`
   - enumerate open documents, active file path, workspace root.
2. `document.*`
   - read current document text, selection, language id.
   - write/replace text only when write permission is granted.
3. `commands.*`
   - register command metadata and invoke host command IDs.
4. `notifications.*`
   - post info/warn/error messages through host notification channel.

Forbidden in v1:

1. direct pointer/object access to editor internals,
2. direct Qt object access,
3. direct process-wide environment mutation.

## Formatter contributions (v1)

Optional manifest field for executable extensions:

```json
"formatters": [
  {
    "languages": ["python", "json"],
    "args": ["--stdin-filepath", "${filePath}", "--tab-width", "${tabWidth}"]
  }
]
```

Rules:

1. Formatter contributions are ignored for `language-pack` extensions.
2. Formatter execution is out-of-process via the declared extension entrypoint.
3. Formatter input is the current document content over stdin.
4. Formatter output must be full-document text over stdout.
5. On formatter failure, the host reports an error and preserves original content.
6. If no formatter matches, host fallback formatter behavior applies.

## Security Model

### Trust assumptions

1. Extensions are untrusted by default.
2. No extension receives implicit broad access.
3. Permission grants are per extension ID and version.

### Permission classes (v1)

1. `workspace.read`
   - Read files under workspace root.
2. `workspace.write`
   - Modify files under workspace root.
3. `filesystem.read:<path>`
   - Read outside workspace, path-scoped.
4. `filesystem.write:<path>`
   - Write outside workspace, path-scoped.
5. `network.client`
   - Outbound network requests.
6. `process.spawn`
   - Spawn subprocesses (required for external formatter execution).
7. `clipboard.read`
   - Read system clipboard.
8. `clipboard.write`
   - Write system clipboard.

### Permission grant rules

1. Declared permissions live in `extension.json`.
2. First use requires explicit user approval (implemented via extension permission prompts).
3. Grants can be:
   - `once`
   - `session`
   - `always` (persisted).
4. Denied permissions fail closed with explicit host error.

### Isolation and guardrails

1. Extension process startup timeout and health checks.
2. Hard kill/restart on unresponsive extension.
3. Per-extension stderr/stdout log capture in diagnostics path.
4. Rate limiting for host notification spam.
5. Versioned API handshake:
   - extension declares `apiVersion`,
   - host rejects incompatible versions with clear error.

## Versioning and compatibility

1. `apiVersion` follows `major.minor`.
2. v1 host guarantees backward compatibility for `1.x` APIs.
3. Breaking changes require v2 and explicit migration notes.

## Install and state paths (Linux)

Using XDG-aligned directories:

1. installed extensions:
   - `${XDG_DATA_HOME}/notepad-plus-plus-linux/extensions`
2. extension state/cache:
   - `${XDG_STATE_HOME}/notepad-plus-plus-linux/extensions`
   - `${XDG_CACHE_HOME}/notepad-plus-plus-linux/extensions`
3. permission grant store:
   - `${XDG_CONFIG_HOME}/notepad-plus-plus-linux/extensions-permissions.json`

## Minimal manifest example

```json
{
  "schemaVersion": 1,
  "id": "ross.sample.formatter",
  "name": "Sample Formatter",
  "version": "0.1.0",
  "type": "command-plugin",
  "apiVersion": "1.0",
  "entrypoint": "bin/formatter-extension",
  "permissions": [
    "workspace.read",
    "workspace.write",
    "process.spawn"
  ],
  "formatters": [
    {
      "languages": [
        "python",
        "json"
      ],
      "args": [
        "--stdin-filepath",
        "${filePath}",
        "--tab-width",
        "${tabWidth}"
      ]
    }
  ]
}
```

## Implementation sequence

1. Implement discovery/install/enable/disable/remove manager against this manifest.
2. Implement permission prompt + grant persistence using this permission model.
3. Add ADR for VS Code compatibility scope and non-goals.
4. Implement targeted TextMate/language-config compatibility path.
