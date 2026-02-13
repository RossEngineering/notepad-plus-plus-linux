# Publishing Signed Linux Release Artifacts

Last updated: 2026-02-13

## Automation

GitHub workflow: `.github/workflows/linux-release.yml`

Triggers:

- Tag push matching `v*` (publishes a GitHub release)
- Manual `workflow_dispatch` (build + artifact upload only)

Outputs:

- `notepad-plus-plus-linux-<version>-x86_64.tar.xz`
- `notepad-plus-plus-linux-<version>-x86_64.sha256`
- Optional detached ASCII signatures (`.asc`) for each file

## Required secrets for signing

- `LINUX_RELEASE_GPG_KEY`:
  - base64-encoded private key block
- `LINUX_RELEASE_GPG_PASSPHRASE`:
  - passphrase for the imported key (empty if key is not passphrase-protected)

If no key secret is configured, the workflow still publishes unsigned artifacts + checksum.

## Local dry-run

```bash
./scripts/release/create_linux_release_artifacts.sh v0.0.0-local out/release
```

## Tag-based publish flow

```bash
git tag -a v0.6.0 -m "v0.6.0"
git push origin v0.6.0
```

The workflow will build, checksum, sign (if configured), and attach artifacts to the GitHub release for that tag.
