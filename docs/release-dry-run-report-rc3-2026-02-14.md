# RC3 Release Dry-Run Report - 2026-02-14

> **Linux-only fork notice:** This repository and its releases target Linux only. For the original Windows Notepad++ application, visit [notepad-plus-plus.org](https://notepad-plus-plus.org/).

This report records RC3 release-engineering dry-run evidence for:

1. artifact generation
2. checksum verification
3. reproducibility check
4. signing flow validation
5. rollback path validation

## Executed command

```bash
./scripts/release/run_release_dry_run.sh --version v0.9.9-rc.3-dryrun
```

## Result summary

- Status: pass
- Rollback probe: pass
- Artifact build: pass
- Checksum verification: pass (`sha256sum -c`)
- Reproducibility check: pass (second rebuild checksum matched)
- Signing verification: pass (ephemeral dry-run key in isolated `GNUPGHOME`)

## Produced dry-run artifacts

- `out/release-dry-run/notepad-plus-plus-linux-v0.9.9-rc.3-dryrun-x86_64.tar.xz`
- `out/release-dry-run/notepad-plus-plus-linux-v0.9.9-rc.3-dryrun-x86_64.sha256`
- `out/release-dry-run/notepad-plus-plus-linux-v0.9.9-rc.3-dryrun-x86_64.tar.xz.asc`
- `out/release-dry-run/notepad-plus-plus-linux-v0.9.9-rc.3-dryrun-x86_64.sha256.asc`

## Checksum value

- `a2696b465c941955ac71ba184244c082edb78710782d40a90dca38317f189c1f  notepad-plus-plus-linux-v0.9.9-rc.3-dryrun-x86_64.tar.xz`

## Notes

- Dry-run harness: `scripts/release/run_release_dry_run.sh`
- Artifact builder supports override build dirs to avoid stale root-owned output from containerized builds:
  - `NPP_RELEASE_BUILD_DIR`
  - `NPP_RELEASE_STAGE_DIR`
