#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "utils.h"
#include "ui.h"

void print_progress_bar(int current, int total) {
    const int bar_width = 30;
    float progress = (float) current / total;
    int filled = (int) (progress * bar_width);
    printf("\rRunning: [");
    for (int i = 0; i < bar_width; i++) {
        if (i < filled) {
            printf("#");
        } else {
            printf("-");
        }
    }
    printf("] %3d%%", (int) (progress * 100));
    fflush(stdout);
}

void print_version() {
    printf(ANSI_BOLD ANSI_GREEN "forksta" ANSI_RESET " version " ANSI_CYAN VERSION "\n" ANSI_RESET);
    printf(ANSI_YELLOW "© 2025 konni332. Licensed under the MIT License.\n" ANSI_RESET);
    printf(ANSI_YELLOW "See https://opensource.org/license/mit for details.\n" ANSI_RESET);
}

void print_help() {
    printf(ANSI_BOLD ANSI_GREEN "Usage: " ANSI_RESET "forksta [options] <program> [args...] [--with <program> [args...]]\n\n");

    printf(ANSI_BOLD "Options:\n" ANSI_RESET);
    printf(ANSI_CYAN "  -r                 " ANSI_RESET "Show real time (default)\n");
    printf(ANSI_CYAN "  -c                 " ANSI_RESET "Show CPU user/sys time\n");
    printf(ANSI_CYAN "  -m                 " ANSI_RESET "Show max RSS (memory usage)\n");
    printf(ANSI_CYAN "  -e                 " ANSI_RESET "Show exit code\n");
    printf(ANSI_CYAN "  -a                 " ANSI_RESET "Show all metrics\n");
    printf(ANSI_CYAN "  -py              * " ANSI_RESET "Specifies a .py programm\n");
    printf(ANSI_CYAN "  --runs N           " ANSI_RESET "Run the program N times (default: 1)\n");
    printf(ANSI_CYAN "  --timeout SEC      " ANSI_RESET "Kill the program after SEC seconds\n");
    printf(ANSI_CYAN "  --timeout-m M      " ANSI_RESET "Kill the program after M minutes\n");
    printf(ANSI_CYAN "  --dump csv         " ANSI_RESET "Dump results to benchmark_results.csv\n");
    printf(ANSI_CYAN "  --dump json        " ANSI_RESET "Dump results to benchmark_results.json\n");
    printf(ANSI_CYAN "  --help             " ANSI_RESET "Show this help message and exit\n");
    printf(ANSI_CYAN "  --version          " ANSI_RESET "Show version and exit\n");
    printf(ANSI_CYAN "  --dependencies     " ANSI_RESET "Show dependencies and exit\n");
    printf(ANSI_CYAN "  --visual         * " ANSI_RESET "Dump JSON file and visualize according to forksta.conf\n");
    printf(ANSI_CYAN "  --compare --with   " ANSI_RESET "Compares 2 program benchmarks\n");
    printf("*see dependencies\n");
    printf("\n" ANSI_BOLD "Example:\n" ANSI_RESET);
    printf("  forksta -a --runs 5 ./my_program arg1 arg2\n");
    printf("  forksta -r --visual --compare ./my_executable arg1 --with -py ./my_script arg1 arg2\n\n");

    printf(ANSI_YELLOW "Forksta – lightweight benchmarking for real programs.\n" ANSI_RESET);
}

void print_cv_result(const double cv) {
    if (isnan(cv)) {
        printf(ANSI_YELLOW "undefined (NaN)\n" ANSI_RESET);
    } else if (isinf(cv)) {
        printf(ANSI_YELLOW "undefined (inf)\n" ANSI_RESET);
    } else if (cv <= 15.0){
        printf(ANSI_GREEN "%.6f%%\n" ANSI_RESET, cv);
    } else if (cv <= 30.0) {
        printf(ANSI_YELLOW "%.6f%%\n" ANSI_RESET, cv);
    } else {
        printf(ANSI_RED "%.6f%%\n" ANSI_RESET, cv);
    }
}

void print_stat(const BenchmarkStats *stats, const char *metric, const int runs) {
    printf(ANSI_BOLD ANSI_CYAN "Metric: "ANSI_RESET ANSI_GREEN "%s\n" ANSI_RESET, metric);
    printf(ANSI_BOLD ANSI_CYAN "Runs: " ANSI_RESET ANSI_GREEN "%d\n" ANSI_RESET, runs);
    printf(ANSI_BOLD "---------------------------------------------" ANSI_RESET "\n");
    printf(ANSI_BOLD ANSI_CYAN "Mean: " ANSI_RESET ANSI_GREEN "%.8f\n" ANSI_RESET, stats->mean);
    printf(ANSI_BOLD ANSI_CYAN "Median: " ANSI_RESET ANSI_GREEN "%.8f\n" ANSI_RESET, stats->median);
    printf(ANSI_BOLD ANSI_CYAN "Min: " ANSI_RESET ANSI_GREEN "%.8f\n" ANSI_RESET, stats->min);
    printf(ANSI_BOLD ANSI_CYAN "Max: " ANSI_RESET ANSI_GREEN "%.8f\n" ANSI_RESET, stats->max);
    printf(ANSI_BOLD ANSI_CYAN "Stddev: " ANSI_RESET ANSI_GREEN "%.8f\n" ANSI_RESET, stats->stddev);
    printf(ANSI_BOLD ANSI_CYAN "CV%%: " ANSI_RESET);
    print_cv_result(stats->cv_percent);
    printf(ANSI_BOLD "---------------------------------------------" ANSI_RESET "\n");
    printf("\n");
}

void print_dependencies(void) {
    printf(ANSI_BOLD ANSI_CYAN "======= Dependencies: =======\n" ANSI_RESET);
    printf(ANSI_YELLOW "(These only apply for -py and --visual usage! All basic functions are dependency-free!)\n\n" ANSI_RESET);

    printf(ANSI_YELLOW "    -py:\n" ANSI_RESET);
    printf(ANSI_CYAN "        All dependencies of the Python script to be benchmarked.\n" ANSI_RESET);

    printf(ANSI_YELLOW "    --visual:\n" ANSI_RESET);
    printf(ANSI_CYAN "        Python 3.7+ required:\n" ANSI_RESET);
    printf(ANSI_GREEN "            https://www.python.org/downloads/\n" ANSI_RESET);
    printf(ANSI_CYAN "        Python package:\n" ANSI_RESET);
    printf(ANSI_GREEN "            pip install colorama\n" ANSI_RESET);
}

void print_args(char **argv, int argc) {
    for (int i = 0; i < argc; i++) {
        printf(ANSI_CYAN "%s " ANSI_RESET, argv[i]);
    }
}

