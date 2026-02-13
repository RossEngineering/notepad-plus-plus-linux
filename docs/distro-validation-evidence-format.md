# Distro Validation Evidence Format (RC1+)

Last updated: 2026-02-13

Use this format for each distro report attached to RC1/RC2/RC3 release checklists.

## Required metadata

1. Report date (UTC)
2. Reporter
3. Candidate version/tag
4. Commit SHA
5. Distro name/version
6. Kernel version
7. Toolchain summary (`cmake`, `ninja`, compiler, Qt version)

## Required checks per distro

1. Configure/build (`debug` + `release`)
2. Regression tests (`ctest`)
3. Sanitizer lane (if supported)
4. Package/install smoke check
5. App launch smoke check
6. Core editor smoke checks:
   - Open/save text file
   - Language detection + syntax highlighting
   - Crash-recovery prompt path (if journal exists)
   - Extension enable/disable flow

## Evidence capture rules

1. Include command lines used.
2. Include pass/fail outcome per check.
3. Include links to CI runs where applicable.
4. Include issue references for failures.
5. Mark each failure as `release-blocking` or `non-blocking`.

## Report template

```markdown
# Distro Validation Report - <candidate>

- Date: <YYYY-MM-DD>
- Reporter: <name>
- Candidate: <tag/version>
- Commit: <sha>

## <distro name/version>

- Kernel: <kernel>
- Toolchain: <summary>

### Checks

| Check | Result | Evidence | Blocking | Notes |
| --- | --- | --- | --- | --- |
| Configure/build debug | pass/fail | <log or command ref> | yes/no | ... |
| Configure/build release | pass/fail | <log or command ref> | yes/no | ... |
| `ctest` | pass/fail | <log or command ref> | yes/no | ... |
| Sanitizer | pass/fail/na | <log or command ref> | yes/no | ... |
| Package/install smoke | pass/fail | <log or command ref> | yes/no | ... |
| Launch smoke | pass/fail | <log or command ref> | yes/no | ... |
| Core editor smoke | pass/fail | <log or command ref> | yes/no | ... |

## Open issues

1. <issue link + summary + owner>
```
