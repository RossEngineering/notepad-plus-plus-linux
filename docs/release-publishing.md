# Publishing Signed Linux Release Artifacts

> **Linux-only fork notice:** This repository and its releases target Linux only. For the original Windows Notepad++ application, visit [notepad-plus-plus.org](https://notepad-plus-plus.org/).

Last updated: 2026-02-14

## Automation

GitHub workflow: `.github/workflows/linux-release.yml`

Triggers:

- Tag push matching `v*` (publishes a GitHub release)
- Manual `workflow_dispatch` (build + artifact upload, with optional release publish)

Manual workflow inputs:

- `version` (optional): artifact/release version label
- `publish_release` (boolean): if true, also publish/update a GitHub release
- `prerelease` (boolean): mark manual published release as prerelease

Outputs:

- `notepad-plus-plus-linux-<version>-x86_64.tar.xz`
- `notepad-plus-plus-linux-<version>-x86_64.sha256`
- `notepad-plus-plus-linux_<version>_amd64.deb`
- `notepad-plus-plus-linux-<version>-<release>.x86_64.rpm`
- `notepad-plus-plus-linux-<version>-1-x86_64.pkg.tar.zst`
- Optional detached ASCII signatures (`.asc`) for each file

## Required secrets for signing

- `LINUX_RELEASE_GPG_KEY`:
  - base64-encoded private key block
- `LINUX_RELEASE_GPG_PASSPHRASE`:
  - passphrase for the imported key (empty if key is not passphrase-protected)

If no key secret is configured, the workflow still publishes unsigned artifacts + checksum.

## Local dry-run

Default local run now produces all artifact formats and requires:
`dpkg-deb`, `rpmbuild`, and `zstd`.

```bash
./scripts/release/create_linux_release_artifacts.sh v0.0.0-local out/release
```

Build tarball-only outputs locally (skip distro package formats):

```bash
NPP_RELEASE_NATIVE_PACKAGES=0 ./scripts/release/create_linux_release_artifacts.sh v0.0.0-local out/release
```

RC3 release-engineering dry-run (artifacts + checksum + reproducibility + signing path + rollback probe):

```bash
./scripts/release/run_release_dry_run.sh --version v0.9.9-rc.3-dryrun
```

## Local install update channel

Packaged installs now include a default update channel marker at:
`/usr/share/notepad-plus-plus-linux/default-update-channel`.

For local installs, you can override the channel metadata during install:

```bash
./scripts/linux/install-local.sh --from-stage out/release/stage --prefix ~/.local --update-channel candidate
```

## Tag-based publish flow

```bash
git tag -a v0.6.0 -m "v0.6.0"
git push origin v0.6.0
```

The workflow will build, checksum, sign (if configured), and attach artifacts to the GitHub release for that tag.
Tags containing a pre-release suffix (for example `-beta.3`, `-rc.1`) are
published as GitHub prereleases and are not marked as "latest".

## Manual run (Actions UI)

You can run **Linux Release Artifacts** manually and choose:

1. build-only (artifact upload), or
2. build + GitHub release publish.

For manual publish, the workflow uses `version` as `tag_name` (or `manual-<sha>` if omitted).
