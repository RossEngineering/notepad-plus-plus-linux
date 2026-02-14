# Portable Module Inventory

> **Linux-only fork notice:** This repository and its releases target Linux only. For the original Windows Notepad++ application, visit [notepad-plus-plus.org](https://notepad-plus-plus.org/).

Last updated: 2026-02-13
Scope: prioritize components already cross-platform (or close) for Linux migration.

## Summary

Static scan indicates:

- `PowerEditor/src`: `670` files total
- `PowerEditor/src` files with obvious Win32 tokens: `152` (from previous audit)
- Practical implication: a substantial subset can be reused, but the app shell/UI layers are still Windows-bound.

## Tier A: Cross-platform ready (high confidence)

These areas show no direct `windows.h` / core Win32 type matches in quick scans.

1. `scintilla/src` (`74` files)
2. `lexilla/lexlib` (+ most of `lexilla/lexers`) (`156` files scanned across lexlib+lexers, one notable exception listed below)
3. `PowerEditor/src/uchardet` (`54` files)
4. `PowerEditor/src/pugixml` and `PowerEditor/src/TinyXml` (`10` files)
5. `PowerEditor/src/MISC/sha1` and `PowerEditor/src/MISC/sha2`
6. `PowerEditor/src/EncodingMapper.cpp`
7. `PowerEditor/src/Utf8_16.cpp`

These modules are good first candidates for reuse in `core`.

## Tier B: Mostly portable with small exceptions

1. `PowerEditor/src/MISC/sha512/sha512.cpp`
   - Includes `windows.h` directly.
   - Likely fixable with small conditional-include cleanup.
2. `lexilla/lexers/LexUser.cxx`
   - Includes `windows.h`.
   - Isolated exception inside otherwise portable lexer stack.

## Tier C: Windows-bound and needs abstraction/replacement

1. App shell and message loop:
   - `PowerEditor/src/winmain.cpp`
   - `PowerEditor/src/Notepad_plus_Window.cpp`
   - `PowerEditor/src/NppBigSwitch.cpp`
2. UI/control layer:
   - `PowerEditor/src/WinControls/**`
3. Dark mode / DPI:
   - `PowerEditor/src/NppDarkMode.cpp`
   - `PowerEditor/src/DarkMode/*`
   - `PowerEditor/src/dpiManagerV2.cpp`
4. Shared utility area with Win32 bleed:
   - `PowerEditor/src/MISC/Common/*`
   - `PowerEditor/src/ScintillaComponent/Buffer.cpp`
5. Windows build entry path:
   - `PowerEditor/gcc/makefile` still executes `cmd`.

## Migration leverage

Recommended reuse-first sequence:

1. Keep Scintilla + Lexilla engines intact.
2. Pull `uchardet`, XML libs, and pure encoding helpers into platform-neutral `core` modules.
3. Wrap/replace `MISC/Common` and `Buffer` filesystem dependencies behind new `platform/include` interfaces.
4. Rebuild app shell + dialogs in Linux UI layer (Qt) rather than translating Win32 messages directly.

## Notes

- Inventory is based on static token scanning and known architectural boundaries.
- Final classification should be refined while wiring Linux build targets in Phase 2.
