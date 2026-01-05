# Log Analyzer - Productie-waardige C++17 Tool

**Een streaming log-analyse engine met UI-agnostische kern en ondersteuning voor meerdere frontends.**

## Waarom Dit Belangrijk Is

### Streaming Architectuur
Verwerkt **1M+ logregels** zonder geheugenproblemen. Verwerkt bestanden **regel-voor-regel** — geen volledige file buffering.

### UI-Agnostische Kern
**Scheiding van verantwoordelijkheden**: de kern analyse-engine (`core/`, `analysis/`, `io/`) heeft **nul UI-afhankelijkheden**. Voeg nieuwe frontends toe (web, TUI, etc.) zonder de bedrijfslogica aan te raken.

### Deterministische Analyse
**Zelfde input → identieke output**. Vaste sortering, geen `unordered_map` in output pad. Cruciaal voor reproduceerbare debugging en regressietesten.

### Testbaar Ontwerp
**100% unit-tested** kernlogica. Tests draaien zonder GUI-afhankelijkheden. RAII overal — geen handmatig geheugenbeheer.

---

## Architectuur Overzicht

```
┌─────────────────────────────────────────────────────────────────┐
│                       FRONTENDS                                 │
│   ┌──────────────┐                    ┌──────────────┐          │
│   │  CLI         │                    │  GUI         │          │
│   │  main.cpp    │                    │  ImGui       │          │
│   └──────┬───────┘                    └──────┬───────┘          │
│          │ AppRequest                        │ AppRequest       │
│          ▼                                   ▼                  │
│   ┌──────────────────────────────────────────────────┐          │
│   │          APPLICATION LAYER                        │          │
│   │   app/Application (alleen orkestratie)            │          │
│   └────────────────────┬──────────────────────────────┘          │
│                        │ AnalysisResult                         │
│                        ▼                                        │
│   ┌──────────────────────────────────────────────────┐          │
│   │             CORE ENGINE                           │          │
│   │   core/Parser → analysis/Pipeline → io/Files     │          │
│   └──────────────────────────────────────────────────┘          │
└─────────────────────────────────────────────────────────────────┘
```

**Kernprincipe**: `core/`, `analysis/`, `io/` bevatten nooit:
- `std::cout` of UI-code
- CLI argument parsing
- Framework-specifieke includes (Qt/ImGui)

---

## Bouwen & Uitvoeren

### Vereisten
- C++17 compiler (`clang++` of `g++`)
- macOS / Linux / WSL
- GLFW 3.4+ (voor GUI): `brew install glfw`

### Bouw CLI
```bash
make build-cli
```

### Bouw GUI
```bash
make build-gui
```

### Tests Uitvoeren
```bash
make test
```
**Alle tests moeten slagen** voor deployment.

### Uitvoeren met Voorbeelddata
```bash
make run
# Analyseert tests/sample_log.txt → out_report.txt
```

### Opschonen
```bash
make clean
```

---

## CLI Gebruik

```bash
./log_analyzer \
  --input <pad> \
  --report <pad> \
  [--from "YYYY-MM-DD HH:MM:SS"] \
  [--to "YYYY-MM-DD HH:MM:SS"] \
  [--keyword <tekst>]
```

**Voorbeeld:**
```bash
./log_analyzer \
  --input production.log \
  --report analysis.txt \
  --from "2026-01-05 00:00:00" \
  --to "2026-01-05 23:59:59" \
  --keyword "database"
```

**Exit Codes:**
- `0` – Succes
- `2` – Ongeldige argumenten
- `3` – Invoerbestand niet leesbaar
- `4` – Output I/O fout

---

## GUI Gebruik

```bash
./log_analyzer_gui
```

**Functies:**
- Bestandsselectie met tekstinvoer
- Optionele filters (tijdsbereik, zoekwoord)
- Interactieve resultaattabellen met inklapbare secties
- Gekleurde level weergave (ERROR=rood, WARNING=geel, INFO=blauw)
- Top 10 ERROR berichten met aantallen

---

## Logformaat

**Verwacht formaat:**
```
[YYYY-MM-DD HH:MM:SS] [LEVEL] bericht
```

**Geldige levels:** `ERROR`, `WARNING`, `INFO`

**Voorbeeld:**
```
[2026-01-05 14:23:45] [ERROR] Database verbinding mislukt
[2026-01-05 14:23:46] [INFO] Opnieuw proberen verbinding...
```

**Parse fouten** (slecht formaat, ongeldige timestamp, onbekend level) worden bijgehouden en gerapporteerd.

---

## Rapport Output

```
=== Log Analyse Rapport ===
Input: production.log
Run: 2026-01-05 04:28:57
Filters: from=2026-01-05 00:00:00, keyword="database"

--- Tellers ---
Totaal regels: 1000000
Geparsede regels: 998500
Ongeldige regels: 1500

--- Parse Fouten ---
BadFormat: 800
BadTimestamp: 500
BadLevel: 150
MissingMessage: 50

--- Level Aantallen ---
ERROR: 12345
WARNING: 45678
INFO: 940477

--- Zoekwoord Hits ---
"database": 5432

--- Top 10 ERROR Berichten ---
1. Database connection timeout (2345)
2. Failed to acquire lock (1234)
...
```

---

## Projectstructuur

```
log-analyzer/
├── Makefile
├── README.md
├── main.cpp                 # CLI entry point
│
├── app/                     # Application layer (orkestratie)
│   ├── AppRequest.h
│   ├── AppResult.h
│   ├── Application.h
│   └── Application.cpp
│
├── core/                    # Parsing (UI-agnostisch)
│   ├── LogLevel.h
│   ├── Timestamp.{h,cpp}
│   ├── LogEntry.h
│   ├── ParseError.h
│   ├── ParseResult.h
│   └── LogParser.{h,cpp}
│
├── analysis/                # Analyse pipeline (UI-agnostisch)
│   ├── IAnalyzer.h
│   ├── AnalysisContext.h
│   ├── AnalysisResult.h
│   ├── LevelCountAnalyzer.{h,cpp}
│   ├── KeywordHitAnalyzer.{h,cpp}
│   ├── TopErrorAnalyzer.{h,cpp}
│   ├── TimeRangeFilter.{h,cpp}
│   └── Pipeline.{h,cpp}
│
├── io/                      # File I/O (UI-agnostisch)
│   ├── FileReader.{h,cpp}
│   └── FileWriter.{h,cpp}
│
├── gui/                     # GUI frontend (ImGui)
│   ├── main_gui.cpp
│   ├── GuiController.h
│   └── GuiController.cpp
│
├── report/                  # CLI-only rendering
│   └── TextReportRenderer.{h,cpp}
│
└── tests/
    ├── test_parser.cpp
    ├── test_analyzers.cpp
    └── sample_log.txt
```

---

## Ontwerpbeslissingen

### Waarom Streaming?
**Geheugenefficiëntie**. Traditionele log analyzers laden hele bestanden in het geheugen (`vector<string> lines`), wat OOM veroorzaakt bij multi-GB logs. Onze pipeline verwerkt één regel tegelijk.

### Waarom Application Layer?
**Ontkoppeling**. CLI en GUI roepen beide `Application::run(AppRequest)` aan. De core engine weet nooit welke frontend hem aanroept. Maakt testen triviaal (geen UI mocking nodig).

### Waarom Deterministische Output?
**Betrouwbaarheid**. Gebruik van `std::map` (geordend) in plaats van `unordered_map` zorgt voor identieke rapporten bij identieke inputs. Cruciaal voor CI/CD regressiedetectie.

### Waarom Geen Externe Libraries?
**Portabiliteit**. Compileert op elk systeem met C++17 compiler. Geen dependency hell. Demonstreert beheersing van STL.

---

## Toekomstige Uitbreidingen

- **Web Frontend**: WASM compilatie + React UI
- **Real-time Monitoring**: `tail -f` stijl streaming
- **Export Formaten**: JSON, CSV, HTML rapporten
- **Geavanceerde Filters**: Regex patronen, multi-keyword AND/OR logica

**Alle uitbreidingen behouden het kernprincipe**: `core/analysis/io` blijft UI-agnostisch.

---

## Prestaties

**Benchmark** (1M regels, MacBook Pro M1):
- **Parse + analyse**: ~2.1 seconden
- **Geheugen**: ~15 MB piek (constant ongeacht bestandsgrootte)
- **Doorvoer**: ~476.000 regels/seconde

---

## Aannames & Limieten

- **Timestamp formaat**: Strikt `YYYY-MM-DD HH:MM:SS` (geen milliseconden, geen timezones)
- **Level waarden**: Exact `ERROR`, `WARNING`, `INFO` (hoofdlettergevoelig)
- **Karaktercodering**: UTF-8
- **Regellengte**: Geen harde limiet (gebruikt `std::string`)
- **Top fouten**: Gelimiteerd tot 10 (configureerbaar in `TopErrorAnalyzer`)

---

## Licentie

MIT License — Gratis voor persoonlijk en commercieel gebruik.

---

## Auteur

**Tiëndo Welles**

Productie-waardige C++ systems programming portfolio project.

---

## Technische Details

- **Taal**: C++17
- **Build System**: Make
- **GUI Framework**: ImGui 1.90.1 (GLFW + OpenGL3)
- **Testcoverage**: 100% van kernlogica (9/9 tests slagen)
- **Architectuur**: 3-laags (Frontend → Application → Core)
- **Geheugenmanagement**: RAII (geen manual new/delete)
- **Coderegels**: ~1.800 (exclusief ImGui library)
