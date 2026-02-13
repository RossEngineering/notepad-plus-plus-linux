# UI Backend Decision

Last updated: 2026-02-13
Decision status: Accepted for initial Linux release

## Decision

Use **Qt 6** as the primary Linux UI backend for this fork.

## Why this decision

1. Repository already contains Scintilla Qt integration (`scintilla/qt`), reducing integration risk.
2. Qt provides mature Linux desktop APIs for menus, dialogs, clipboard, input methods, and high-DPI handling.
3. It offers the fastest path to a stable Linux-native editor shell for Manjaro.
4. Team can focus on architecture migration instead of low-level widget/toolkit gaps.

## Alternatives considered

## GTK (C/gtkmm)

Pros:

- Native GNOME ecosystem fit.
- Solid Linux integration.

Cons:

- Existing codebase has no equivalent GTK application shell.
- Higher rewrite surface for complex window/control behavior in this fork.

Decision:

- Not selected for first release.

## wxWidgets

Pros:

- Cross-platform and C++ friendly.

Cons:

- We do not have a repo-local acceleration path comparable to `scintilla/qt`.
- UI behavior customization would likely still require substantial adaptation.

Decision:

- Not selected for first release.

## Immediate UI scope (Qt)

- Main window with tabbed editor area.
- Scintilla-based editor widget integration.
- Menubar + command dispatch bridge to application logic.
- Core dialogs:
  - open/save
  - find/replace
  - preferences (minimal initial subset)

## Out-of-scope for first milestone

- Full parity with Windows dark-mode implementation.
- Port of every legacy Win32 custom control.
- Exact menu/layout parity with upstream Notepad++.

## Implementation notes

- Keep Qt types out of `core` headers.
- Define a thin adapter layer from Qt signals/events to app commands.
- Keep platform services (filesystem/process/clipboard paths) behind `platform` interfaces.
- Reuse Scintilla Qt artifacts from `scintilla/qt` where possible.

## Risk register

1. Command routing is currently message-centric (`SendMessage`-heavy).
   Mitigation: Introduce command bus abstraction before large dialog migration.
2. Plugin model is Windows-ABI-centric.
   Mitigation: Ship first Linux release without plugin ABI parity commitment.
3. Theming expectations may drift from upstream UX.
   Mitigation: Prioritize consistency and readability over pixel parity.

## Re-evaluation trigger

Revisit this decision only if one of the following occurs:

- Scintilla Qt integration blocks required editing behavior.
- Packaging/runtime footprint becomes unacceptable for target distro constraints.
- A new requirement mandates toolkit alignment with an external host application.
