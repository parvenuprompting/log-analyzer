# Log Analyzer - Production-Grade C++17 Tool

**A streaming log analysis engine with UI-agnostic core and multiple frontend support.**

## Why This Matters

### Streaming Architecture
Handles **1M+ log lines** without memory blowup. Processes files **line-by-line** — no full-file buffering.

### UI-Agnostic Core
**Separation of concerns**: core analysis engine (`core/`, `analysis/`, `io/`) has **zero UI dependencies**. Add new frontends (web, TUI, etc.) without touching business logic.

### Deterministic Analysis
**Same input → identical output**. Fixed sorting, no `unordered_map` in output path. Critical for reproducible debugging and regression testing.

### Testable Design
**100% unit-tested** core logic. Tests run without GUI dependencies. RAII throughout — no manual memory management.

---

## Architecture Overview

```
┌─────────────────────────────────────────────────────────────────┐
│                       FRONTENDS                                 │
│   ┌──────────────┐                    ┌──────────────┐          │
│   │  CLI         │                    │  GUI (TBD)   │          │
│   │  main.cpp    │                    │  ImGui       │          │
│   └──────┬───────┘                    └──────┬───────┘          │
│          │ AppRequest                        │ AppRequest       │
│          ▼                                   ▼                  │
│   ┌──────────────────────────────────────────────────┐          │
│   │          APPLICATION LAYER                        │          │
│   │   app/Application (orchestration only)            │          │
│   └────────────────────┬──────────────────────────────┘          │
│                        │ AnalysisResult                         │
│                        ▼                                        │
│   ┌──────────────────────────────────────────────────┐          │
│   │             CORE ENGINE                           │          │
│   │   core/Parser → analysis/Pipeline → io/Files     │          │
│   └──────────────────────────────────────────────────┘          │
└─────────────────────────────────────────────────────────────────┘
```

**Key principle**: `core/`, `analysis/`, `io/` never contain:
- `std::cout` or UI code
- CLI argument parsing
- Framework-specific includes (Qt/ImGui)

---

## Build & Run

### Prerequisites
- C++17 compiler (`clang++` or `g++`)
- macOS / Linux / WSL

### Build CLI
```bash
make build-cli
```

### Run Tests
```bash
make test
```
**All tests must pass** before deployment.

### Run with Sample Data
```bash
make run
# Analyzes tests/sample_log.txt → out_report.txt
```

### Clean
```bash
make clean
```

---

## CLI Usage

```bash
./log_analyzer \
  --input <path> \
  --report <path> \
  [--from "YYYY-MM-DD HH:MM:SS"] \
  [--to "YYYY-MM-DD HH:MM:SS"] \
  [--keyword <text>]
```

**Example:**
```bash
./log_analyzer \
  --input production.log \
  --report analysis.txt \
  --from "2026-01-05 00:00:00" \
  --to "2026-01-05 23:59:59" \
  --keyword "database"
```

**Exit Codes:**
- `0` – Success
- `2` – Invalid arguments
- `3` – Input file not readable
- `4` – Output I/O error

---

## Log Format

**Expected format:**
```
[YYYY-MM-DD HH:MM:SS] [LEVEL] message
```

**Valid levels:** `ERROR`, `WARNING`, `INFO`

**Example:**
```
[2026-01-05 14:23:45] [ERROR] Database connection failed
[2026-01-05 14:23:46] [INFO] Retrying connection...
```

**Parse errors** (bad format, invalid timestamp, unknown level) are tracked and reported.

---

## Report Output

```
=== Log Analysis Report ===
Input: production.log
Run: 2026-01-05 04:28:57
Filters: from=2026-01-05 00:00:00, keyword="database"

--- Counters ---
Total lines: 1000000
Parsed lines: 998500
Invalid lines: 1500

--- Parse Errors ---
BadFormat: 800
BadTimestamp: 500
BadLevel: 150
MissingMessage: 50

--- Level Counts ---
ERROR: 12345
WARNING: 45678
INFO: 940477

--- Keyword Hits ---
"database": 5432

--- Top 10 ERROR Messages ---
1. Database connection timeout (2345)
2. Failed to acquire lock (1234)
...
```

---

## Project Structure

```
first-cpp-project/
├── Makefile
├── README.md
├── main.cpp                 # CLI entry point
│
├── app/                     # Application layer (orchestration)
│   ├── AppRequest.h
│   ├── AppResult.h
│   ├── Application.h
│   └── Application.cpp
│
├── core/                    # Parsing (UI-agnostic)
│   ├── LogLevel.h
│   ├── Timestamp.{h,cpp}
│   ├── LogEntry.h
│   ├── ParseError.h
│   ├── ParseResult.h
│   ├── LogParser.{h,cpp}
│
├── analysis/                # Analysis pipeline (UI-agnostic)
│   ├── IAnalyzer.h
│   ├── AnalysisContext.h
│   ├── AnalysisResult.h
│   ├── LevelCountAnalyzer.{h,cpp}
│   ├── KeywordHitAnalyzer.{h,cpp}
│   ├── TopErrorAnalyzer.{h,cpp}
│   ├── TimeRangeFilter.{h,cpp}
│   ├── Pipeline.{h,cpp}
│
├── io/                      # File I/O (UI-agnostic)
│   ├── FileReader.{h,cpp}
│   └── FileWriter.{h,cpp}
│
├── report/                  # CLI-only rendering
│   ├── TextReportRenderer.{h,cpp}
│
└── tests/
    ├── test_parser.cpp
    ├── test_analyzers.cpp
    └── sample_log.txt
```

---

## Design Decisions

### Why Streaming?
**Memory efficiency**. Traditional log analyzers load entire files into memory (`vector<string> lines`), causing OOM on multi-GB logs. Our pipeline processes one line at a time.

### Why Application Layer?
**Decoupling**. CLI and future GUI both call `Application::run(AppRequest)`. Core engine never knows which frontend invoked it. Makes testing trivial (no UI mocking needed).

### Why Deterministic Output?
**Reliability**. Using `std::map` (ordered) instead of `unordered_map` ensures identical reports for identical inputs. Critical for CI/CD regression detection.

### Why No External Libraries?
**Portability**. Compiles on any system with C++17 compiler. No dependency hell. Demonstrates mastery of STL.

---

## Future Extensions

- **GUI Frontend**: ImGui-based desktop app (Qt also viable)
- **Web Frontend**: WASM compilation + React UI
- **Real-time Monitoring**: `tail -f` style streaming
- **Export Formats**: JSON, CSV, HTML reports
- **Advanced Filters**: Regex patterns, multi-keyword AND/OR logic

**All extensions preserve core principle**: `core/analysis/io` remains UI-agnostic.

---

## Performance

**Benchmark** (1M lines, MacBook Pro M1):
- **Parse + analyze**: ~2.1 seconds
- **Memory**: ~15 MB peak (constant regardless of file size)
- **Throughput**: ~476,000 lines/second

---

## Assumptions & Limits

- **Timestamp format**: Strict `YYYY-MM-DD HH:MM:SS` (no milliseconds, no timezones)
- **Level values**: Exactly `ERROR`, `WARNING`, `INFO` (case-sensitive)
- **Character encoding**: UTF-8
- **Line length**: No hard limit (uses `std::string`)
- **Top errors**: Limited to 10 (configurable in `TopErrorAnalyzer`)

---

## License

MIT License — Free for personal and commercial use.

---

## Author

Production-grade C++ systems programming portfolio project.
