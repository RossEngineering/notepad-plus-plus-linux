# Consumer Install Guide (Linux)

Last updated: 2026-02-14

This guide covers consumer-friendly install and uninstall flows with desktop integration.

## Option A: Install from release tarball (user-local, recommended)

This installs into `~/.local`, registers desktop/menu metadata, MIME data, and icon caches.

```bash
./scripts/linux/install-local.sh \
  --from-tar out/release/notepad-plus-plus-linux-v0.9.6-rc.2-x86_64.tar.xz \
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
2. Desktop entry
3. Icon
4. MIME package definition
5. Installed skin assets

## Notes

1. The installer uses user-local paths and does not require `sudo`.
2. Desktop cache/mime/icon cache updates are run when tooling is available:
   - `update-desktop-database`
   - `update-mime-database`
   - `gtk-update-icon-cache`
3. `--set-default` uses `xdg-mime` when available to set default handlers for common text/code mime types.
