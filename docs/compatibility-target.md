# Compatibility Target (Linux MVP)

Last updated: 2026-02-13
Target platform: Arch Linux and derivatives (including Manjaro as baseline)

## Purpose

Define what "compatible enough" means for the first Linux-native release so implementation stays focused and shippable.

## Compatibility model

This fork targets **behavioral compatibility**, not binary/ABI compatibility with Windows builds.

For Linux MVP:

- Keep core editing behavior familiar to Notepad++ users.
- Accept UI and plugin architecture differences where Win32 assumptions are deep.
- Prioritize reliability and performance over visual parity.

## Must-have for Linux MVP (release gate)

1. Editor core
   - Open/edit/save text files reliably.
   - Encoding support for common Notepad++ workflows (UTF-8, UTF-16 LE/BE, ANSI fallback behavior policy documented).
   - EOL handling for LF/CRLF.
2. Multi-document workflow
   - Tabbed editing.
   - Recent files and basic session restore.
3. Search
   - Find/replace in active document.
   - Basic multi-file find in folder.
4. Language support
   - Scintilla/Lexilla-based syntax highlighting.
   - Theme loading (at least default light/dark plus one custom theme path).
5. Linux integration
   - Native app launch (no Wine).
   - XDG-compliant config/data/cache paths.
   - Desktop entry + icon + file associations for common text/code files.

## Should-have for Linux MVP (high priority, not strict gate)

1. Split view and document map parity for common workflows.
2. Shortcut customization import/export.
3. Basic external tool/run command support.
4. Startup/session performance within practical parity of baseline behavior.

## Explicitly out of scope for Linux MVP

1. Windows plugin ABI compatibility.
2. Win32 message-level behavior parity.
3. 1:1 recreation of Windows dark-mode internals.
4. Full parity for every legacy dialog/control at first release.

## Plugin compatibility target

- Linux MVP plugin target: **new Linux-native plugin API surface (or no plugin API in MVP)**
- No promise to load existing Windows `.dll` plugins.
- Provide migration path documentation after core MVP stabilizes.

## Acceptance criteria

Linux MVP is considered achieved when:

1. All Must-have items are implemented and manually verified on Arch Linux derivatives (including Manjaro).
2. A clean install package can be built and launched on a fresh Arch Linux derivative environment.
3. Core workflows (open/edit/save/search/session) pass regression checks.
4. Known incompatibilities are documented in release notes.

## Versioning recommendation

- Mark first Linux-native release as `0.x` (or `alpha/beta`) to signal migration phase.
- Move to `1.0` only after plugin strategy and broader parity are stabilized.
