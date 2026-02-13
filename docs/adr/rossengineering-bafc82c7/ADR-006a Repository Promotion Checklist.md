# RossEngineering — Repository Promotion Checklist

This checklist is used to determine whether a repository is ready to be **promoted** within the RossEngineering organisation.

Promotion indicates that a project is **professional, intentional, and portfolio-grade**.

This checklist is derived from organisation-level decisions and ADRs.  
All items must be satisfied unless explicitly noted.

---

## 1. Intent & Scope

- [ ] The project has a clear, well-defined purpose
- [ ] The problem domain is explicitly stated
- [ ] The scope is intentional (not “everything at once”)
- [ ] Non-goals are clearly documented

---

## 2. README Quality (Required)

- [ ] README exists at the repository root
- [ ] README explains *why* the project exists
- [ ] README explains *what* the project is (at a system level)
- [ ] README explains *what it is not*
- [ ] README reflects quiet honesty (no exaggerated claims)
- [ ] README is written for engineers, not marketing

---

## 3. Architecture & Structure

- [ ] Repository structure reflects clear separation of concerns
- [ ] Solution name matches repository name
- [ ] Naming follows PascalCase and suffix conventions
- [ ] Boundaries are visible in code structure
- [ ] Architecture choices are defensible

---

## 4. Decision Log (Required)

- [ ] `docs/decisions.md` exists
- [ ] At least one meaningful decision is recorded
- [ ] Decisions explain *why*, not just *what*
- [ ] Trade-offs are acknowledged
- [ ] Superseded decisions are preserved, not deleted

---

## 5. Testing (Required)

- [ ] Automated tests exist
- [ ] Tests cover meaningful behaviour
- [ ] Tests align with current design
- [ ] Tests are not permanently lagging behind the code
- [ ] If the project builds, it is testable

> If a project builds successfully and has no tests, promotion must not proceed.

---

## 6. CI & Automation (Required)

- [ ] CI pipeline exists
- [ ] CI runs build and tests
- [ ] CI passes on `main`
- [ ] Failure on `main` is not permitted
- [ ] CI configuration is committed to the repository

---

## 7. Versioning & Compatibility Awareness

- [ ] Versioning strategy is documented or intentionally deferred
- [ ] Backward compatibility considerations are acknowledged
- [ ] Deprecation (if any) is justified and documented
- [ ] Error formats are consistent with organisational norms where applicable

---

## 8. Security Awareness

- [ ] Security posture is acknowledged (even if minimal)
- [ ] No obvious credential leaks or unsafe defaults
- [ ] Reporting path for security issues is clear or intentionally deferred
- [ ] Security expectations match the project’s scope

---

## 9. Runtime & Deployment Expectations

- [ ] Runtime assumptions are documented
- [ ] Backend projects include Docker (or successor) support
- [ ] Desktop software includes an installer or setup script
- [ ] Deployment or execution instructions are present and accurate

---

## 10. Documentation & Maintainability

- [ ] Documentation reflects prior thought, not just current state
- [ ] Complex areas are explained or justified
- [ ] Code can be read and reasoned about by others
- [ ] Future evolution is not obviously blocked by current design

---

## 11. Final Promotion Decision

- [ ] All required sections above are satisfied
- [ ] Known gaps are documented and intentional
- [ ] Promotion is an explicit, conscious decision

---

## Promotion Outcome

- **Promoted**: Repository meets RossEngineering standards  
- **Incubator**: Intentional but not yet complete  
- **Archive**: Valuable, but no longer active  
- **Reject / Defer**: Does not meet required baseline

---

### Notes

Promotion is not a reward for effort or time spent.  
It is recognition of **clarity, discipline, and intentional engineering**.

If promotion feels uncertain, the correct answer is usually **Incubator**, not compromise.
