# Performance Baseline

This project tracks a lightweight baseline for startup cost and typing latency to detect regressions as Linux-native work continues.

## Benchmark harness

- Executable: `startup_typing_benchmark`
- Source: `tests/perf/startup_typing_benchmark.cpp`
- Build command:

```bash
cmake -S . -B build-release -G Ninja -DBUILD_TESTING=ON -DCMAKE_BUILD_TYPE=Release
cmake --build build-release -j --target startup_typing_benchmark
./build-release/bin/startup_typing_benchmark
```

## Baseline snapshot

- Timestamp: `2026-02-13T19:53:02+00:00`
- Host: `Linux 6.18.8-1-MANJARO x86_64 GNU/Linux`
- CPU threads: `24`
- Build: `Release`

Results:

- `startup_mean_us=7.98`
- `startup_p95_us=7.61`
- `typing_mean_us=0.13`
- `typing_p95_us=0.13`
- `startup_iterations=200`
- `typing_iterations=5000`

## Notes

- Startup benchmark measures creation of core `Document` state, insertion of seed text, and one `cpp` lexing pass.
- Typing benchmark measures repeated single-character appends on an initialized document.
- Keep comparison runs on the same machine profile when possible.
