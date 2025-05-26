import json
from colorama import Fore, Style, init
from dataclasses import dataclass, field, asdict
from typing import List, Dict, Any, Optional, Union
import argparse

@dataclass
class MetaData:
    target: str
    args: List[str]
    runs: int
    timeout_ms: int

@dataclass
class RunResult:
    real_time: float
    sys_time: float
    user_time: float
    max_rss: int
    exit_code: int

@dataclass
class MetricStats:
    mean: float
    median: float
    min: float
    min_run: int
    max: float
    max_run: int
    stddev: float
    cv_percent: float

@dataclass
class ResultSummary:
    real_time: MetricStats
    sys_time: MetricStats
    user_time: MetricStats
    max_rss: MetricStats
    exit_code: int

@dataclass
class BenchmarkResult:
    meta: MetaData
    runs: List[RunResult]
    results: ResultSummary



def parse_benchmark_json(path: str) -> BenchmarkResult:
    with open(path, 'r') as f:
        raw = json.load(f)

    meta = MetaData(**raw["meta"])

    # Alle Runs extrahieren
    run_keys = [k for k in raw.keys() if k.startswith("run_")]
    run_keys_sorted = sorted(run_keys, key=lambda x: int(x.split("_")[1]))

    runs = [RunResult(**raw[k]) for k in run_keys_sorted]

    def parse_metric(name: str) -> MetricStats:
        data = raw["results"][name]
        return MetricStats(
            mean=data["mean"],
            median=data["median"],
            min=data["min"],
            min_run=data["min_run"],
            max=data["max"],
            max_run=data["max_run"],
            stddev=data["stddev"],
            cv_percent=data["cv"]
        )

    results = ResultSummary(
        real_time=parse_metric("real_time"),
        sys_time=parse_metric("sys_time"),
        user_time=parse_metric("user_time"),
        max_rss=parse_metric("max_rss"),
        exit_code=raw["results"]["exit_code"]
    )

    return BenchmarkResult(meta=meta, runs=runs, results=results)


init(autoreset=True)

def print_list(obj: Union[RunResult, MetricStats], title: str = ""):
    data = asdict(obj)
    max_key_len = max(len(k) for k in data.keys())

    if title:
        print(f"{Style.BRIGHT}{Fore.BLUE}{'=' * (max_key_len + 25)}")
        print(f"{Style.BRIGHT}{Fore.BLUE}  {title}")
        print(f"{Style.BRIGHT}{Fore.BLUE}{'=' * (max_key_len + 25)}")

    for key, value in data.items():
        label = f"{Fore.CYAN}{key.replace('_', ' ').capitalize():<{max_key_len}}"

        if isinstance(value, (int, float)):
            # Format standardmäßig als float (außer ints)
            if isinstance(value, int):
                value_str = f"{Fore.WHITE}{value}"
            else:
                # Sonderbehandlung für cv_percent
                if key == "cv_percent":
                    if value > 30:
                        color = Fore.RED
                    elif value > 15:
                        color = Fore.YELLOW
                    else:
                        color = Fore.GREEN
                    value_str = f"{color}{value:.2f} %"
                else:
                    value_str = f"{Fore.GREEN}{value:.6f}"
        else:
            value_str = f"{Fore.BLACK}{value}"

        print(f"{label} : {value_str}")

    print(f"{Style.DIM}{'-' * (max_key_len + 25)}\n")

def print_ascii_table(obj: Union[RunResult, MetricStats], title: str = ""):
    data = asdict(obj)
    headers = ["Key", "Value"]
    rows = []

    max_key_len = max(len(k) for k in data.keys())
    max_val_len = 0

    for key, value in data.items():
        label = key.replace('_', ' ').capitalize()
        if isinstance(value, float):
            if key == "cv_percent":
                if value > 30:
                    color = Fore.RED
                elif value > 15:
                    color = Fore.YELLOW
                else:
                    color = Fore.GREEN
                value_str = f"{color}{value:.2f} %"
            else:
                value_str = f"{Fore.GREEN}{value:.6f}"
        elif isinstance(value, int):
            value_str = f"{str(value)}"
        else:
            value_str = str(value)

        rows.append((label, value_str))
        max_val_len = max(max_val_len, len(strip_ansi(value_str)))

    table_width = max_key_len + max_val_len + 7  # Padding + borders

    if title:
        print(f"{Style.BRIGHT}{Fore.BLUE}{'=' * table_width}")
        print(f"{Style.BRIGHT}{Fore.BLUE}  {title}")
        print(f"{Style.BRIGHT}{Fore.BLUE}{'=' * table_width}")

    # Header
    print(f"{Style.BRIGHT}{Fore.WHITE}| {'Key':<{max_key_len}} | {'Value':<{max_val_len}} |")
    print(f"{'-' * table_width}")

    for key, val in rows:
        print(f"{Fore.CYAN}| {key:<{max_key_len}} | {val:<{max_val_len}} |")

    print(f"{'-' * table_width}\n")


def strip_ansi(s):
    """Hilfsfunktion, um ANSI-Farbcodes aus Längenberechnung zu entfernen"""
    import re
    ansi_escape = re.compile(r'\x1B[@-_][0-?]*[ -/]*[@-~]')
    return ansi_escape.sub('', s)


def plot_runs_with_min_max(runs: List[RunResult], stats: MetricStats, metric_name: str, width: int = 50):
    values = [getattr(run, metric_name) for run in runs]

    min_idx = stats.min_run - 1
    max_idx = stats.max_run - 1

    min_val = stats.min
    max_val = stats.max

    range_val = max_val - min_val if max_val != min_val else 1e-9

    print(f"{Style.BRIGHT}Plot of '{metric_name}' for {len(runs)} runs:")
    print(f"Min run: {min_idx + 1} ({min_val:.6f}), Max run: {max_idx + 1} ({max_val:.6f})\n")

    for i, val in enumerate(values):
        pos = int((val - min_val) / range_val * (width - 1))

        line = ['-'] * width

        line[pos] = '|'

        if i == min_idx:
            line[pos] = f"{Style.BRIGHT}{Fore.GREEN}M{Style.RESET_ALL}"  # M für Min
        elif i == max_idx:
            line[pos] = f"{Style.BRIGHT}{Fore.RED}X{Style.RESET_ALL}"    # X für Max
        else:
            line[pos] = f"{Style.BRIGHT}{Fore.CYAN}|{Style.RESET_ALL}"

        print(f"Run {i + 1:3}: {''.join(line)} {val:.6f}")


def print_heatmap(bm_result: BenchmarkResult, max_runs=50):
    runs = bm_result.runs
    num_runs = len(runs)

    if num_runs > max_runs:
        raise ValueError(f"Too many runs ({num_runs}). Max allowed: {max_runs}")

    metrics = {
        "real_time": lambda r: r.real_time,
        "sys_time":  lambda r: r.sys_time,
        "user_time": lambda r: r.user_time,
        "max_rss":   lambda r: r.max_rss,
    }

    values = {name: [getter(run) for run in runs] for name, getter in metrics.items()}

    mins = {m: min(vs) for m, vs in values.items()}
    maxs = {m: max(vs) for m, vs in values.items()}

    metric_width   = max(len(m) for m in metrics.keys())
    run_num_width  = len(str(num_runs))
    cell_width     = max(run_num_width, 1) + 1  # 1 extra Platz für Abstand

    def color_block(v, vmin, vmax):
        ratio = 0 if vmax == vmin else (v - vmin) / (vmax - vmin)
        if ratio < 0.2:
            code = 32  # grün
        elif ratio < 0.4:
            code = 92  # hellgrün
        elif ratio < 0.6:
            code = 93  # gelb
        elif ratio < 0.8:
            code = 91  # hellrot
        else:
            code = 31  # rot
        return f"\033[{code}m█\033[0m".rjust(cell_width)

    header = f"{'Metric'.ljust(metric_width)} |"
    for i in range(1, num_runs + 1):
        header += f" {str(i).rjust(run_num_width)}"
    print(header)

    print("-" * (metric_width + 3 + (cell_width * num_runs)))

    for m, vs in values.items():
        line = f"{m.ljust(metric_width)} |"
        for v in vs:
            line += color_block(v, mins[m], maxs[m])
        print(line)

def main():
    parser = argparse.ArgumentParser(description="Benchmark Visualizer")
    parser.add_argument(
        "--mode",
        choices=["heatmap", "plot", "table", "list"],
        default="heatmap",
        help="Ausgabemodus wählen"
    )
    parser.add_argument(
        "--file",
        required=True,
        help="Pfad zur Benchmark JSON Datei"
    )

    group = parser.add_mutually_exclusive_group()
    group.add_argument("-r", "--realtime", action="store_true", help="show only real time (default)")
    group.add_argument("-c", "--cpu", action="store_true", help="show system and user time")
    group.add_argument("-m", "--maxrss", action="store_true", help="show only max rss")
    group.add_argument("-a", "--allmetrics", action="store_true", help="show all metrics")

    args = parser.parse_args()

    bm_result = parse_benchmark_json(args.file)

    selected_metrics = []
    if args.mode in ["table", "list", "plot"]:
        if args.realtime:
            selected_metrics = ["real_time"]
        elif args.cpu:
            selected_metrics = ["sys_time", "user_time"]
        elif args.maxrss:
            selected_metrics = ["max_rss"]
        elif args.allmetrics:
            selected_metrics = ["real_time", "sys_time", "user_time", "max_rss"]
        else:
            selected_metrics = ["real_time"]

    if args.mode == "heatmap":
        print_heatmap(bm_result)
    elif args.mode == "plot":
        for i, value in enumerate(selected_metrics):
            stats = getattr(bm_result.results, value)
            plot_runs_with_min_max(bm_result.runs, stats, value)
    elif args.mode == "table":
        for i, value in enumerate(selected_metrics):
            stats = getattr(bm_result.results, value)
            print_ascii_table(stats, value)
    elif args.mode == "list":
        for i, value in enumerate(selected_metrics):
            stats = getattr(bm_result.results, value)
            print_list(stats, value)

    else:
        print("Unbekannter Modus:", args.mode)

if __name__ == "__main__":
    main()
