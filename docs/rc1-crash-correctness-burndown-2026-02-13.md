# RC1 Crash/Correctness Burndown (2026-02-13)

> **Linux-only fork notice:** This repository and its releases target Linux only. For the original Windows Notepad++ application, visit [notepad-plus-plus.org](https://notepad-plus-plus.org/).

- Candidate: `v0.9.3-rc.1`
- Candidate SHA: `fe525477f`
- Status: complete for RC1 cut

## Summary

RC1 stabilization goals for crash/correctness are complete for the candidate scope.
No unresolved P0 items remain.
One P1 item (`RC1-B04`) is deferred to RC2 with explicit scope rationale.

## Disposition by blocker

| Blocker | Priority | Disposition | Evidence |
| --- | --- | --- | --- |
| `RC1-B01` Extension permissions UX | P0 | fixed | explicit deny semantics, rollback-on-deny, permission reset action, lifecycle smoke test coverage |
| `RC1-B02` Syntax color consistency | P0 | fixed | shared lexer style map + targeted regression tests (`lexer_style_config_regression_test`, syntax smoke coverage) |
| `RC1-B03` Cross-distro baseline | P0 | fixed | Arch/Ubuntu/Fedora build/test/install/launch evidence in `docs/distro-validation-report-rc1-2026-02-13.md` |
| `RC1-B04` LSP baseline confidence | P1 | deferred to RC2 | baseline stability proven by `lsp_client_foundation_smoke_test`; deeper UX scope remains RC2 |
| `RC1-B05` Crash/correctness burn-down | P1 | closed | this report + RC1 blocker file + RC1 checklist updates |

## Validation evidence

1. Local regression suites:
   - `ctest --preset debug --output-on-failure` (`12/12`)
   - `ctest --preset release --output-on-failure` (`12/12`)
2. Sanitizer-equivalent lane:
   - ASan/UBSan subset (`10/10`) on local host with `ASAN_OPTIONS=detect_leaks=0` due local ptrace/LSan restriction.
   - Full leak gating remains required in GitHub CI sanitizer lane.
3. Boundary safety:
   - `./scripts/check_win32_boundaries.sh HEAD~1` pass.

## RC1 conclusion

Crash/correctness objectives are satisfied for RC1.
Open LSP depth work is intentionally tracked in RC2 and does not block `v0.9.3-rc.1`.
