# Skin Format v1

This document defines the Phase 10 skin file format for the Linux-native UI.

## File location

- User skin config file: `~/.config/notepad-plus-plus-linux/theme-linux.json`
- JSON Schema: `docs/schemas/skin-v1.schema.json`

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
