# Language Detection Metrics (Beta 1)

> **Linux-only fork notice:** This repository and its releases target Linux only. For the original Windows Notepad++ application, visit [notepad-plus-plus.org](https://notepad-plus-plus.org/).

This document defines acceptance metrics and report format for Phase 9 language detection.

## Scope

- Applies to automatic language detection + auto-lexer switching.
- Evaluated by `language_detection_regression_test`.
- Fixture corpus is located at `tests/fixtures/language-detection`.

## Beta 1 acceptance thresholds

1. `language_detection_accuracy >= 0.95`
2. `language_detection_false_positive_rate <= 0.05`
3. `language_detection_low_confidence_failures == 0`
4. Corpus must include Markdown, HTML/XML, and programming-language samples.

## Report format

The regression test emits newline-delimited `key=value` metrics:

```text
language_detection_total=<int>
language_detection_correct=<int>
language_detection_accuracy=<float>
language_detection_false_positive_rate=<float>
language_detection_low_confidence_failures=<int>
```

## Run locally

```bash
cmake --build build -j
ctest --test-dir build --output-on-failure -R language_detection_regression_test
```

## CI integration note

For Beta 1 gating, this test must run in required Linux CI jobs and remain green.
