# Tester Quickstart (RC/GA Validation)

> **Linux-only fork notice:** This repository and its releases target Linux only. For the original Windows Notepad++ application, visit [notepad-plus-plus.org](https://notepad-plus-plus.org/).

This guide is for external testers validating release candidates and GA readiness.

## 1. Install

Use one of these paths:

- Consumer install flow: `docs/install-consumer-linux.md`
- Developer/source install flow: `docs/install-linux.md`
- Latest release assets: `https://github.com/RossEngineering/notepad-plus-plus-linux/releases`

Record which method you used. Include that in bug reports.

## 2. 10-minute smoke test checklist

- Launch from application menu/launcher and verify the app starts without warnings.
- Open a text file by double-clicking it from your file manager.
- Open a second file via `File > Open`.
- Save edits and confirm reopen retains content.
- Toggle light/dark skin and confirm controls remain readable.
- Open `Preferences` and verify setting changes persist after restart.
- Validate syntax highlighting on one source file you use regularly.
- Run `Format Document` on a file and verify output is sane for that language.
- If you use extensions, confirm at least one extension loads and basic function works.
- Close and relaunch once to check startup and session stability.

## 3. Report issues

- Bug form (preferred): `https://github.com/RossEngineering/notepad-plus-plus-linux/issues/new?template=bug_report.yml`
- Feature request form: `https://github.com/RossEngineering/notepad-plus-plus-linux/issues/new?template=2-feature-request.yml`
- Support hub: `docs/help-and-support.md`

Include:

- Version/tag or commit SHA.
- Distro, kernel, desktop environment/window manager.
- Install method.
- Reproduction steps and expected/actual behavior.
- Screenshot/log when available.

## 4. High-priority report types

Please flag these clearly in the issue title:

- Crash or startup failure
- Data loss or file corruption
- Save/open regression
- Broken launcher, file association, or desktop integration
- Severe dark/light theme readability regressions
