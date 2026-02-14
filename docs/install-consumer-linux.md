# Consumer Install Guide (Linux)

Last updated: 2026-02-14

This guide covers consumer-friendly install and uninstall flows with desktop integration.

## Option A: Install from release tarball (user-local, recommended)

This installs into `~/.local`, registers desktop/menu metadata, MIME data, and icon caches.
Launcher entries are patched to absolute binary paths for stable launch/dock behavior.

```bash
./scripts/linux/install-local.sh \
  --from-tar out/release/notepad-plus-plus-linux-<version>-x86_64.tar.xz \
  --set-default
```

If `~/.local/bin` is not on your `PATH`, add it:

```bash
echo 'export PATH="$HOME/.local/bin:$PATH"' >> ~/.profile
```

Then log out/in or source your profile.

## Option B: Install from local build stage

```bash
cmake --preset release
cmake --build --preset release -- -j"$(nproc)"
DESTDIR="$PWD/out/release/stage" cmake --install build/release
./scripts/linux/install-local.sh --from-stage out/release/stage --set-default
```

## Uninstall (clean removal)

```bash
./scripts/linux/uninstall-local.sh
```

This removes:

1. Binary (`~/.local/bin/notepad-plus-plus-linux`)
2. Desktop entry and icon integration
3. MIME package definition
4. Installed skin assets
5. Default MIME associations added by `--set-default` (for covered mime types)

## Notes

1. The installer uses user-local paths and does not require `sudo`.
2. Desktop cache/mime/icon cache updates are run when tooling is available:
   - `update-desktop-database`
   - `update-mime-database`
   - `gtk-update-icon-cache`
3. `--set-default` uses `xdg-mime` when available to set default handlers for common text/code mime types.
4. For first-run launcher discoverability:
   - GNOME: open Activities and search for `Notepad++ Linux`.
   - KDE Plasma: open application launcher and search for `Notepad++ Linux`.
5. To pin to dock/taskbar, launch once from the launcher and use the desktop environment pin/favorite action.
