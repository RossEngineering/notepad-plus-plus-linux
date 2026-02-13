# Agentic Development Guide — Notepad Plus Plus Linux

This repository is dedicated to building a reliable and stable **Notepad++ Linux** experience.  
Agents are expected to follow **disciplined development principles**, ensuring code quality, cross-platform compatibility, and seamless integration.

---

## Core Principles

Agents must prioritize:
- **Stability over novelty**: This project aims to bring Notepad++ to Linux reliably. Avoid introducing unnecessary complexity or “clever” solutions.
- **Cross-platform consistency**: Ensure the application behaves the same way on all supported Linux distributions.
- **Minimal, intentional changes**: Only make changes that solve problems directly related to the project’s objectives.

---

## 1. Change Discipline

### Operating Mode
Always explicitly state your mode before working on tasks:
- **Analysis Mode**: When investigating bugs, dependencies, or external changes.
- **Implementation Mode**: When writing code to implement a feature or fix a bug.
- **Review Mode**: When reviewing pull requests, refactoring, or assessing the stability of code.

Changes should follow a **minimal, scoped approach**:
- Do not "refactor" code unnecessarily.
- Avoid sweeping global changes unless they’re clearly beneficial (e.g., major bug fixes or architectural updates).
- Flag any uncertainty upfront in your commit messages.

---

## 2. Code Quality Standards

### Consistency
- Follow **consistent naming conventions** across all files and functions.
- Ensure **clear, concise comments** where necessary (but avoid over-commenting self-explanatory code).
- Ensure **no hard-coded paths** or values specific to a single distribution. Where possible, use variables or configurations that can be changed easily.
  
### Formatting
- Adhere to any **coding style guides** specified in the repo (e.g., indentation, naming conventions, etc.).
- Keep code DRY (Don’t Repeat Yourself) but avoid premature abstraction.

### Readability
- Code should be **readable without extensive commentary**.
- Introduce **clear boundaries** between distinct responsibilities (e.g., logic, UI, configuration).

---

## 3. Testing Expectations

- Update or add tests whenever **behavioural changes** are made.
- When no tests exist:
  - Do not introduce a test framework or infrastructure unless clearly necessary.
  - Make it clear in your PRs where coverage is missing or where additional tests are needed.
  
For **cross-platform tests**:
- Ensure that the build, install, and run processes work on multiple distributions (e.g., Ubuntu, Fedora, Debian).
- If tests are added to verify platform-specific behaviour, ensure they are clearly documented and easily extensible.

---

## 4. Dependency Management

- **Minimise dependencies**: Avoid adding unnecessary libraries or tools. Ensure that each dependency is **justified** with clear documentation.
- Ensure that all dependencies are **cross-platform compatible**.
- Keep third-party tools **up-to-date** with proper version constraints.

---

## 5. Cross-Platform Considerations

This repo aims to run **Notepad++ on Linux** effectively, so:

- Any changes that impact platform compatibility should **clearly document the platform-specific behaviour** and assumptions.
- When updating scripts or making changes related to installation or configuration:
  - Ensure **compatibility across all supported Linux distributions**.
  - **Flag** any non-ideal platform-specific workarounds.

---

## 6. Security & Safety

- **Sanitize inputs** and validate all external data (e.g., environment variables, config files).
- Avoid **storing secrets** or sensitive data in the codebase. Use environment variables or configuration files where needed.
- Ensure any new dependencies do not introduce **security risks**.

---

## 7. Documentation Expectations

- **README**: Ensure the documentation is clear on installation steps, usage, and any special configuration required for Linux.
- **Code Documentation**: Keep function and class-level documentation minimal but descriptive enough to understand the intent. Focus on **why** a solution was chosen, not just **what** is being done.
- **Platform-specific Notes**: Include clear guidance for any **platform-specific installation steps**, configuration options, or compatibility notes.

---

## 8. What Agents Must NOT Do

Agents must not:
- Introduce **breaking changes to the installation** process without explicit discussion and approval.
- Modify or introduce **behaviours** that are specific to one platform unless absolutely necessary.
- Remove platform-agnostic features unless they are truly redundant or unnecessary.

---

## 9. When to Ask Questions

- **Ambiguous requirements**: When the desired functionality or change isn’t clear.
- **Cross-platform issues**: If unsure whether a modification will impact other Linux distributions.
- **Security concerns**: Any changes that could impact security should be discussed.
- **Uncertainty about dependencies**: If adding a dependency or upgrading an existing one, confirm whether it aligns with the repo’s goals.

---

## 10. Definition of Done

A task is complete when:
- The functionality works as intended across all target Linux distributions.
- All tests pass (where applicable).
- Documentation is updated, and any necessary platform-specific notes are added.
- **The change is simple, explicit, and aligns with project goals**.
