# Skin Format v1

This document defines the Phase 10 skin file format for the Linux-native UI.

## File location

- User skin config file: `~/.config/notepad-plus-plus-linux/theme-linux.json`
- JSON Schema: `docs/schemas/skin-v1.schema.json`
- Installed first-party skins: `/usr/share/notepad-plus-plus-linux/skins`

## Goals

- Provide a stable, versioned JSON format for:
  - app chrome (main window, menus, status bar)
  - editor surface and syntax token colors
  - dialogs/buttons
- Keep backward compatibility with legacy flat `theme-linux.json` keys.

## Top-level structure

```json
{
  "$schema": "https://raw.githubusercontent.com/RossEngineering/notepad-plus-plus-linux/master/docs/schemas/skin-v1.schema.json",
  "formatVersion": 1,
  "metadata": {
    "id": "builtin.light",
    "name": "Built-in Light",
    "author": "notepad-plus-plus-linux",
    "description": "Default light skin."
  },
  "appChrome": {},
  "editor": {},
  "dialogs": {}
}
```

## Color format

- Colors are strings in `#RRGGBB` format.
- Invalid or missing values fall back to compiled defaults.

## `appChrome` tokens

- `windowBackground`
- `windowForeground`
- `menuBackground`
- `menuForeground`
- `statusBackground`
- `statusForeground`
- `accent`

### Derived visual mapping

To keep v1 compact while still covering the full UI shell, these surfaces are derived from the
tokens above plus dialog/editor tokens:

- tab strip/background states (`QTabWidget`/`QTabBar`)
- form fields in dialogs (`QLineEdit`, `QSpinBox`, `QComboBox`, `QTextEdit`)
- tooltip and hover/focus states

Derived values blend existing tokens (for example `menuBackground` + `windowBackground`) so skins
remain backward compatible.

## `editor` tokens

- `background`
- `foreground`
- `lineNumberBackground`
- `lineNumberForeground`
- `caretLineBackground`
- `selectionBackground`
- `selectionForeground`
- `comment`
- `keyword`
- `number`
- `stringColor`
- `operatorColor`

## `dialogs` tokens

- `background`
- `foreground`
- `buttonBackground`
- `buttonForeground`
- `border`

## Compatibility note

- Existing flat theme files remain supported.
- Skin v1 section keys take precedence when both formats are present.

## First-party skin set

Phase 10 ships these built-in skins:

- `light.json`
- `dark.json`
- `high-contrast.json`

To apply one manually now, copy it to the user theme path:

```bash
cp /usr/share/notepad-plus-plus-linux/skins/dark.json ~/.config/notepad-plus-plus-linux/theme-linux.json
```

## Runtime selection and persistence

- Runtime switcher is available at `View > Skins`.
- Current options: `Light`, `Dark`, `High Contrast`.
- Selected skin is persisted in `editor-settings.json` as:

```json
{
  "skinId": "builtin.dark"
}
```
