# Wayland QA Pass

> **Linux-only fork notice:** This repository and its releases target Linux only. For the original Windows Notepad++ application, visit [notepad-plus-plus.org](https://notepad-plus-plus.org/).

Last updated: 2026-02-14

## Purpose

Catch regressions that only appear under native Wayland startup/runtime paths.

## Automation

- Workflow: `.github/workflows/linux-wayland-qa.yml`
- Script: `scripts/linux/run-wayland-qa.sh`

The script runs an Ubuntu 24.04 Docker container, installs Qt6 Wayland and weston, builds `npp_linux_shell`, and performs a headless Wayland smoke run using:

- `weston --backend=headless-backend.so`
- `QT_QPA_PLATFORM=wayland`

## Local run

```bash
./scripts/linux/run-wayland-qa.sh
```
