# Crash Recovery Journal

> **Linux-only fork notice:** This repository and its releases target Linux only. For the original Windows Notepad++ application, visit [notepad-plus-plus.org](https://notepad-plus-plus.org/).

Last updated: 2026-02-13

## Purpose

Provide automatic recovery of unsaved editor content after an unclean shutdown.

## Behavior

1. The Qt shell writes a crash-recovery journal periodically while running.
2. Only dirty tabs are captured.
3. Journal capture includes:
   - file path (if any)
   - full text content
   - encoding and EOL mode
   - lexer name
   - active tab index
   - capture timestamp
4. On next startup, if a journal exists, the app prompts to recover unsaved content.
5. On a clean shutdown, the journal is removed.

## Storage location

- Journal file: `<XDG_CONFIG_HOME>/notepad-plus-plus-linux/crash-recovery-journal.json`

## Settings integration

These keys are persisted in `editor-settings.json`:

- `crashRecoveryAutosaveSeconds` (default: `15`, range clamp: `5`-`300`)

