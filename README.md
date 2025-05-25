# 🏁 Forksta – Lightweight CLI Benchmarking

**Forksta** is a fast, lightweight, cross-platform benchmarking tool for programs and scripts. Written in C for maximum performance, Forksta is ideal for **quick performance analysis, metric collection, and optional visualization** via Python.

> 📊 Perfect for developers who want system-level benchmarking without heavy setup.

---

## ⚙️ Features

- 🔁 Run multiple times with `--runs`
- ⏱️ Metrics for **Real Time**, **CPU Time**, **Memory (RSS)**, and **Exit Codes**
- 📉 Optional: **Visualization** via Heatmap, Table, Plot, or List
- 🧪 Compare two programs (`--compare --with`)
- 🧵 Support for Python scripts (`-py`)
- 📄 Export results to CSV or JSON
- 💡 `.conf` file support for configuration

---

## 🚀 Quickstart

```bash
forksta -a --runs 5 ./my_program arg1 arg2
forksta -r --visual --compare ./my_exec --with -py ./my_script.py
```

Without `--visual`, output is shown as a simple list in the terminal.

---

## 📦 Installation

> Compile using any C compiler (GCC, Clang, MSVC).

```bash
gcc -o forksta forksta.c
```

### Dependencies

| Feature        | Dependency                  |
|----------------|-----------------------------|
| `-py`          | Python ≥ 3.7                |
| `--visual`     | `colorama` (via pip)        |

```bash
pip install colorama
```
---

## 🧩 Options

```text
Usage: forksta [options] <program> [args...] [--with <program> [args...]]

Options:
  -r                 Show real time (default)
  -c                 Show CPU user/sys time
  -m                 Show max RSS (memory usage)
  -e                 Show exit code
  -a                 Show all metrics
  -py              * Specifies a .py program
  --runs N           Run the program N times (default: 1)
  --timeout SEC      Kill the program after SEC seconds
  --timeout-m M      Kill the program after M minutes
  --dump csv         Dump results to benchmark_results.csv
  --dump json        Dump results to benchmark_results.json
  --help             Show help message and exit
  --version          Show version and exit
  --dependencies     Show dependencies and exit
  --visual         * Visualize output using forksta.conf
  --compare --with   Compare two benchmarks

* see dependencies
```

---

## 🗂️ Visualization & Configuration

Visualization is controlled via an optional **`forksta.conf`** file, which defines:

- Number of benchmark runs (`runs`)
- Number of warm-up runs
- Visualization style (`heatmap`, `plot`, `table`, `list`)

### 🔍 Config file locations:

| OS        | Path                                                   |
|-----------|--------------------------------------------------------|
| Windows   | Same folder as `forksta.exe`                           |
| Linux     | `~/.config/forksta/`                                   |
| macOS     | `~/Library/Application Support/forksta/`               |

---

## 💡 Example `forksta.conf`

```ini
[DEFAULT]
runs = 5
warmup = 1
visual = TABLE
```

---

## 📚 License

MIT License  
© 2025 konni332  
https://opensource.org/license/mit

---

## ❗ Notes

Forksta is a hobby project and under active development. Bugs may occur – feedback is welcome!
