#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "utils.h"
#include "ui.h"

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

void print_benchmark_result(const BenchmarkResult result, const config_t cfg) {
    printf("Final results:\n");
    printf("----\n");
    if (cfg.show_realtime || cfg.show_all) {
        printf("    real time mean: %f\n", result.real_time_stats.mean);
        printf("    real time median: %f\n", result.real_time_stats.median);
        double cv_rt = (result.real_time_stats.stddev / result.real_time_stats.mean) * 100.0;
        printf("    real time standard deviation: %.8f -> cv ~ ", result.real_time_stats.stddev);
        print_cv_result(cv_rt);
        printf("    real time max: %f -> run %d\n", result.real_time_stats.max, result.real_time_stats.max_run);
        printf("    real time min: %f -> run %d\n", result.real_time_stats.min, result.real_time_stats.min_run);
        printf("----\n");
    }
    if (cfg.show_cpu_times || cfg.show_all) {
        printf("    user time mean: %.4f\n", result.user_time_stats.mean);
        printf("    user time median: %.4f\n", result.user_time_stats.median);
        double cv_us = (result.user_time_stats.stddev / result.user_time_stats.mean) * 100.0;
        printf("    user time standard deviation: %.8f -> cv ~ ", result.user_time_stats.stddev);
        print_cv_result(cv_us);
        printf("    user time max: %.4f -> run %d\n", result.user_time_stats.max, result.user_time_stats.max_run);
        printf("    user time min: %.4f -> run %d\n", result.user_time_stats.min, result.user_time_stats.min_run);
        printf("----\n");
        printf("    sys time mean: %.4f\n", result.sys_time_stats.mean);
        printf("    sys time median: %.4f\n", result.sys_time_stats.median);
        double cv_sys = (result.sys_time_stats.stddev / result.sys_time_stats.mean) * 100.0;
        printf("    sys time standard deviation: %.8f -> cv ~ ", result.sys_time_stats.stddev);
        print_cv_result(cv_sys);
        printf("    sys time max: %.4f -> run %d\n", result.sys_time_stats.max, result.sys_time_stats.max_run);
        printf("    sys time min: %.4f -> run %d\n", result.sys_time_stats.min, result.sys_time_stats.min_run);
        printf("----\n");
    }
    if (cfg.show_max_rss || cfg.show_all) {
        printf("    max rss mean: %0.f\n", result.max_rss_stats.mean);
        printf("    max rss median: %0.f\n", result.max_rss_stats.median);
        double cv_rss = (result.max_rss_stats.stddev / result.max_rss_stats.mean) * 100.0;
        printf("    max rss standard deviation: %.2f -> cv ~ ", result.max_rss_stats.stddev);
        print_cv_result(cv_rss);
        printf("    max rss max: %0.f -> run %d\n", result.max_rss_stats.max, result.max_rss_stats.max_run);
        printf("    max rss min: %0.f -> run %d\n", result.max_rss_stats.min, result.max_rss_stats.min_run);
        printf("----\n");
    }
    if (cfg.show_exit_code || cfg.show_all) {
        printf("    most recent exit code: %d\n", result.exit_code);
        printf("----\n");
    }
}

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
    printf(ANSI_BOLD ANSI_GREEN "Usage: " ANSI_RESET "forksta [options] <program> [args...]\n\n");

    printf(ANSI_BOLD "Options:\n" ANSI_RESET);
    printf(ANSI_CYAN "  -r            " ANSI_RESET "Show real time (default)\n");
    printf(ANSI_CYAN "  -c            " ANSI_RESET "Show CPU user/sys time\n");
    printf(ANSI_CYAN "  -m            " ANSI_RESET "Show max RSS (memory usage)\n");
    printf(ANSI_CYAN "  -e            " ANSI_RESET "Show exit code\n");
    printf(ANSI_CYAN "  -a            " ANSI_RESET "Show all metrics\n");
    printf(ANSI_CYAN "  -py           " ANSI_RESET "Specifies a .py programm. Adds neccessary python3 argument\n");
    printf(ANSI_CYAN "  --runs N      " ANSI_RESET "Run the program N times (default: 1)\n");
    printf(ANSI_CYAN "  --timeout SEC " ANSI_RESET "Kill the program after SEC seconds\n");
    printf(ANSI_CYAN "  --timeout-m M " ANSI_RESET "Kill the program after M minutes\n");
    printf(ANSI_CYAN "  --dump csv    " ANSI_RESET "Dump results to benchmark_results.csv\n");
    printf(ANSI_CYAN "  --dump json   " ANSI_RESET "Dump results to benchmark_results.json\n");
    printf(ANSI_CYAN "  --help        " ANSI_RESET "Show this help message and exit\n");
    printf(ANSI_CYAN "  --version     " ANSI_RESET "Show version and exit\n");

    printf("\n" ANSI_BOLD "Example:\n" ANSI_RESET);
    printf("  forksta -a --runs 5 ./my_program arg1 arg2\n\n");

    printf(ANSI_YELLOW "Forksta – lightweight benchmarking for real programs.\n" ANSI_RESET);
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

