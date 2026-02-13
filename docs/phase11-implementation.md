# Phase 11 Implementation Notes

Last updated: 2026-02-13

This document records implementation evidence for the Phase 11 Beta 2 scope.

## 1) Extension lifecycle support

Implemented operations:

1. discover
2. install (from extension folder)
3. enable
4. disable
5. remove

Primary implementation:

- `platform/linux/LinuxExtensionService.h`
- `platform/linux/LinuxExtensionService.cpp`
- `ui/qt/MainWindow.cpp` (`Tools > Extensions` actions)

Validation:

- `tests/platform/extension_lifecycle_smoke_test.cpp`

## 2) Permission prompts (filesystem/network/process)

Prompted permission classes:

1. `filesystem.read:<path>`
2. `filesystem.write:<path>`
3. `network.client`
4. `process.spawn`

Behavior:

1. prompt with modes `Deny`, `Allow Once`, `Allow Session`, `Allow Always`
2. persist `Allow Always` grants in
   `${XDG_CONFIG_HOME}/notepad-plus-plus-linux/extensions-permissions.json`
3. keep session grants in memory for the running process
4. fail closed with explicit host `kPermissionDenied` errors on denied requests
5. support permission decision reset from extension management (`Reset Permissions`)

Primary implementation:

- `platform/linux/LinuxExtensionService.cpp`
- `ui/qt/MainWindow.cpp` permission prompt callback

## 3) VS Code targeted compatibility path

Supported VS Code contribution data:

1. `contributes.languages`
2. `contributes.grammars`

Output:

- normalized snapshot `npp-language-pack-vscode.json`

Primary implementation:

- `platform/linux/VscodeLanguageCompatibility.h`
- `platform/linux/VscodeLanguageCompatibility.cpp`

## 4) Validation with popular VS Code language extensions

Fixture coverage:

1. `ms-python.python`
2. `golang.go`
3. `redhat.vscode-yaml`

Validation test:

- `tests/regression/vscode_language_asset_compatibility_test.cpp`

Fixture root:

- `tests/fixtures/vscode-language-assets/`
