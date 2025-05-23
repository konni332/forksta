
# Forksta

**Forksta** is a lightweight benchmarking tool for real programs. It measures wall-clock time, CPU user/system time, memory usage (RSS), and exit codes of any executable program. It works on both POSIX systems (Linux, macOS) and Windows.

It has no external dependencies and is easy to use.

---

## Features

- Benchmark native executables and .py files
- Measure real (wall-clock) time  
- Capture CPU user and system time  
- Track maximum resident set size (memory usage)  
- Report program exit code  
- Run multiple iterations to average results (incld. mean, median, min, max, stddev, cv%)  
- Timeout support (in seconds or minutes)  
- Export results as CSV or JSON files  
- Cross-platform (POSIX & Windows)  
- No dependencies  

---

## Installation

_in progress_

---

## Usage

```bash
forksta [options] <program> [args...]
```

---

### Options

| Option                                                        |  Desciption                                                               |
|---------------------------------------------------------------|---------------------------------------------------------------------------|
| -r                                                            | Show real time                                                            |
| -c                                                            | Show CPU user and system time                                             |
| -m                                                            | Show max RSS (memory usage)                                               |
| -e                                                            | Show exit code                                                            |
| -a                                                            | Show all metrics (default)                                                |
| -py                                                           | Indicate running .py file: -py example.py. No pyhton3 argument neccessary |
| --runs N                                                      | Run the program N times (default: 1)                                      |
| --tiemout SEC                                                 | Kill the program after SEC seconds                                        |
| --timeout-m M                                                 | Kill the program after M minutes                                          |
| --dump csv                                                    | Dump results to `benchmark_results.csv`                                   |
| --dump json                                                   | Dump results to `benchmark_results.json`                                  |
| --compare <program1 [args1]> --with <program2 [args2]> {wip!} | Benchmark 2 programs and compare the specified metrics                    |
| --compare-json <filename1> --with <filename2> {wip!}          | Compare specified metrics of 2 .json files                                |
| --compare-csv <filename1> --with <filename2> {wip!}           | Compare specified metrics of 2 .csv files                                 |
| --compare-from <program [args]> --to-json <filename> {wip!}   | Benchmark a program and compare specified metrics to existing .json file  |
| --compare-from <program [args]> --to-csv <filename> {wip!}    | Benchmark a program and compare specified metrics to existing .csv file   |


---

## Example

```bash
forksta -a --runs 5 ./my_program arg1 arg2
```

---

## License

MIT License

```
MIT License

Copyright (c) 2025 konni332

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
```

---

## Contact

If you have questions, bug reports, or contributions, please open issues or pull requests on GitHub.

---

Happy benchmarking with **forksta**!
