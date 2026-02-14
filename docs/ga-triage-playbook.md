# GA Issue Triage Playbook

> **Linux-only fork notice:** This repository and its releases target Linux only. For the original Windows Notepad++ application, visit [notepad-plus-plus.org](https://notepad-plus-plus.org/).

Last updated: 2026-02-14
Owner: Maintainers
Applies to: `v1.0.0` GA readiness window

Use this playbook to classify and route incoming community issues quickly.

## Intake sources

1. Bug form: `.github/ISSUE_TEMPLATE/bug_report.yml`
2. Linux regression form: `.github/ISSUE_TEMPLATE/4-linux-regression.yml`
3. Feature form: `.github/ISSUE_TEMPLATE/2-feature-request.yml`

## Severity classification

### `P0` (GA blocker candidate)

- Crash on startup or common workflows.
- Data loss, corruption, failed save, or crash-recovery failure.
- Installer/uninstaller breaks app launch or leaves broken file handlers.
- Desktop integration regression that prevents normal launch/open workflows.

Action:

1. Label issue `priority:p0`.
2. Add or update blocker entry in `docs/ga-blockers.md`.
3. Link issue URL in blocker source/evidence.
4. Track affected distro/environment in `docs/tester-coverage-snapshot-2026-02.md`.

### `P1` (high-priority, non-blocking unless escalated)

- Significant UX regressions with workaround.
- Performance regressions not breaching hard release guardrails.
- Formatter/extension integration breakage limited to specific setups.

Action:

1. Label issue `priority:p1`.
2. Assign owner and milestone (`v1.0.0` or post-GA).
3. Record environment in coverage snapshot.

### `P2` (backlog)

- Nice-to-have improvements or low-impact bugs.
- Non-GA feature requests.

Action:

1. Label `priority:p2` (and `type:enhancement` where applicable).
2. Route to post-GA milestone unless explicitly promoted.

## Daily triage routine

1. Review all new issues from previous 24 hours.
2. Confirm reproducibility and gather missing environment details.
3. Update:
   - `docs/tester-coverage-snapshot-2026-02.md`
   - `docs/ga-blockers.md` (if blocker candidate)
   - `docs/ga-ci-continuity-log-2026-02.md` (if related CI evidence changed)
4. Close duplicates with canonical issue links.
5. Post daily summary in project thread/notes if needed.

## Exit criteria before `v1.0.0`

1. No unresolved `P0` blocker entries in `docs/ga-blockers.md`.
2. Coverage snapshot includes at least one reviewed pass/fail signal for each
   target distro family (Arch derivatives, Ubuntu LTS, Fedora stable).
3. Open `P1`/`P2` items are explicitly deferred with rationale and milestone.
