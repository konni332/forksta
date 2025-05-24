
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
        printf("%.6f%%\n", cv);
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

void print_comparison_real(const BenchmarkResult target_result, const BenchmarkResult comparison_result) {
    if (target_result.real_time_stats.mean < comparison_result.real_time_stats.mean) {
        printf(ANSI_GREEN "    target real time mean:               %f\n" ANSI_RESET, target_result.real_time_stats.mean);
        printf(ANSI_RED   "    comparison real time mean:           %f\n\n" ANSI_RESET, comparison_result.real_time_stats.mean);
    } else {
        printf(ANSI_RED   "    target real time mean:               %f\n" ANSI_RESET, target_result.real_time_stats.mean);
        printf(ANSI_GREEN "    comparison real time mean:           %f\n\n" ANSI_RESET, comparison_result.real_time_stats.mean);
    }
    if (target_result.real_time_stats.median < comparison_result.real_time_stats.median) {
        printf(ANSI_GREEN "    target real time median:             %f\n" ANSI_RESET, target_result.real_time_stats.median);
        printf(ANSI_RED   "    comparison real time median:         %f\n\n" ANSI_RESET, comparison_result.real_time_stats.median);
    } else {
        printf(ANSI_RED   "    target real time median:             %f\n" ANSI_RESET, target_result.real_time_stats.median);
        printf(ANSI_GREEN "    comparison real time median:         %f\n\n" ANSI_RESET, comparison_result.real_time_stats.median);
    }
    if (target_result.real_time_stats.stddev < comparison_result.real_time_stats.stddev) {
        printf(ANSI_GREEN "    target real time standard deviation: %.8f\n" ANSI_RESET, target_result.real_time_stats.stddev);
        printf(ANSI_RED   "    comparison real time standard deviation: %.8f\n\n" ANSI_RESET, comparison_result.real_time_stats.stddev);
    } else {
        printf(ANSI_RED   "    target real time standard deviation: %.8f\n" ANSI_RESET, target_result.real_time_stats.stddev);
        printf(ANSI_GREEN "    comparison real time standard deviation: %.8f\n\n" ANSI_RESET, comparison_result.real_time_stats.stddev);
    }
    if (target_result.real_time_stats.max < comparison_result.real_time_stats.max) {
        printf(ANSI_GREEN "    target real time max:                %f\n" ANSI_RESET, target_result.real_time_stats.max);
        printf(ANSI_RED   "    comparison real time max:            %f\n\n" ANSI_RESET, comparison_result.real_time_stats.max);
    } else {
        printf(ANSI_RED   "    target real time max:                %f\n" ANSI_RESET, target_result.real_time_stats.max);
        printf(ANSI_GREEN "    comparison real time max:            %f\n\n" ANSI_RESET, comparison_result.real_time_stats.max);
    }
    if (target_result.real_time_stats.min < comparison_result.real_time_stats.min) {
        printf(ANSI_GREEN "    target real time min:                %f\n" ANSI_RESET, target_result.real_time_stats.min);
        printf(ANSI_RED   "    comparison real time min:            %f\n\n" ANSI_RESET, comparison_result.real_time_stats.min);
    } else {
        printf(ANSI_RED   "    target real time min:                %f\n" ANSI_RESET, target_result.real_time_stats.min);
        printf(ANSI_GREEN "    comparison real time min:            %f\n\n" ANSI_RESET, comparison_result.real_time_stats.min);
    }
}

void print_comparison_user(const BenchmarkResult target_result, const BenchmarkResult comparison_result) {
    if (target_result.user_time_stats.mean < comparison_result.user_time_stats.mean) {
        printf(ANSI_GREEN "    target user time mean:               %f\n" ANSI_RESET, target_result.user_time_stats.mean);
        printf(ANSI_RED   "    comparison user time mean:           %f\n\n" ANSI_RESET, comparison_result.user_time_stats.mean);
    } else {
        printf(ANSI_RED   "    target user time mean:               %f\n" ANSI_RESET, target_result.user_time_stats.mean);
        printf(ANSI_GREEN "    comparison user time mean:           %f\n\n" ANSI_RESET, comparison_result.user_time_stats.mean);
    }
    if (target_result.user_time_stats.median < comparison_result.user_time_stats.median) {
        printf(ANSI_GREEN "    target user time median:             %f\n" ANSI_RESET, target_result.user_time_stats.median);
        printf(ANSI_RED   "    comparison user time median:         %f\n\n" ANSI_RESET, comparison_result.user_time_stats.median);
    } else {
        printf(ANSI_RED   "    target user time median:             %f\n" ANSI_RESET, target_result.user_time_stats.median);
        printf(ANSI_GREEN "    comparison user time median:         %f\n\n" ANSI_RESET, comparison_result.user_time_stats.median);
    }
    if (target_result.user_time_stats.stddev < comparison_result.user_time_stats.stddev) {
        printf(ANSI_GREEN "    target user time standard deviation: %.8f\n" ANSI_RESET, target_result.user_time_stats.stddev);
        printf(ANSI_RED   "    comparison user time standard deviation: %.8f\n\n" ANSI_RESET, comparison_result.user_time_stats.stddev);
    } else {
        printf(ANSI_RED   "    target user time standard deviation: %.8f\n" ANSI_RESET, target_result.user_time_stats.stddev);
        printf(ANSI_GREEN "    comparison user time standard deviation: %.8f\n\n" ANSI_RESET, comparison_result.user_time_stats.stddev);
    }
    if (target_result.user_time_stats.max < comparison_result.user_time_stats.max) {
        printf(ANSI_GREEN "    target user time max:                %f\n" ANSI_RESET, target_result.user_time_stats.max);
        printf(ANSI_RED   "    comparison user time max:            %f\n\n" ANSI_RESET, comparison_result.user_time_stats.max);
    } else {
        printf(ANSI_RED   "    target user time max:                %f\n" ANSI_RESET, target_result.user_time_stats.max);
        printf(ANSI_GREEN "    comparison user time max:            %f\n\n" ANSI_RESET, comparison_result.user_time_stats.max);
    }
    if (target_result.user_time_stats.min < comparison_result.user_time_stats.min) {
        printf(ANSI_GREEN "    target user time min:                %f\n" ANSI_RESET, target_result.user_time_stats.min);
        printf(ANSI_RED   "    comparison user time min:            %f\n\n" ANSI_RESET, comparison_result.user_time_stats.min);
    } else {
        printf(ANSI_RED   "    target user time min:                %f\n" ANSI_RESET, target_result.user_time_stats.min);
        printf(ANSI_GREEN "    comparison user time min:            %f\n\n" ANSI_RESET, comparison_result.user_time_stats.min);
    }
}

void print_comparison_sys(const BenchmarkResult target_result, const BenchmarkResult comparison_result) {
    if (target_result.sys_time_stats.mean < comparison_result.sys_time_stats.mean) {
        printf(ANSI_GREEN "    target sys time mean:                %f\n" ANSI_RESET, target_result.sys_time_stats.mean);
        printf(ANSI_RED   "    comparison sys time mean:            %f\n\n" ANSI_RESET, comparison_result.sys_time_stats.mean);
    } else {
        printf(ANSI_RED   "    target sys time mean:                %f\n" ANSI_RESET, target_result.sys_time_stats.mean);
        printf(ANSI_GREEN "    comparison sys time mean:            %f\n\n" ANSI_RESET, comparison_result.sys_time_stats.mean);
    }
    if (target_result.sys_time_stats.median < comparison_result.sys_time_stats.median) {
        printf(ANSI_GREEN "    target sys time median:              %f\n" ANSI_RESET, target_result.sys_time_stats.median);
        printf(ANSI_RED   "    comparison sys time median:          %f\n\n" ANSI_RESET, comparison_result.sys_time_stats.median);
    } else {
        printf(ANSI_RED   "    target sys time median:              %f\n" ANSI_RESET, target_result.sys_time_stats.median);
        printf(ANSI_GREEN "    comparison sys time median:          %f\n\n" ANSI_RESET, comparison_result.sys_time_stats.median);
    }
    if (target_result.sys_time_stats.stddev < comparison_result.sys_time_stats.stddev) {
        printf(ANSI_GREEN "    target sys time standard deviation: %.8f\n" ANSI_RESET, target_result.sys_time_stats.stddev);
        printf(ANSI_RED   "    comparison sys time standard deviation: %.8f\n\n" ANSI_RESET, comparison_result.sys_time_stats.stddev);
    } else {
        printf(ANSI_RED   "    target sys time standard deviation: %.8f\n" ANSI_RESET, target_result.sys_time_stats.stddev);
        printf(ANSI_GREEN "    comparison sys time standard deviation: %.8f\n\n" ANSI_RESET, comparison_result.sys_time_stats.stddev);
    }
    if (target_result.sys_time_stats.max < comparison_result.sys_time_stats.max) {
        printf(ANSI_GREEN "    target sys time max:                 %f\n" ANSI_RESET, target_result.sys_time_stats.max);
        printf(ANSI_RED   "    comparison sys time max:             %f\n\n" ANSI_RESET, comparison_result.sys_time_stats.max);
    } else {
        printf(ANSI_RED   "    target sys time max:                 %f\n" ANSI_RESET, target_result.sys_time_stats.max);
        printf(ANSI_GREEN "    comparison sys time max:             %f\n\n" ANSI_RESET, comparison_result.sys_time_stats.max);
    }
    if (target_result.sys_time_stats.min < comparison_result.sys_time_stats.min) {
        printf(ANSI_GREEN "    target sys time min:                 %f\n" ANSI_RESET, target_result.sys_time_stats.min);
        printf(ANSI_RED   "    comparison sys time min:             %f\n\n" ANSI_RESET, comparison_result.sys_time_stats.min);
    } else {
        printf(ANSI_RED   "    target sys time min:                 %f\n" ANSI_RESET, target_result.sys_time_stats.min);
        printf(ANSI_GREEN "    comparison sys time min:             %f\n\n" ANSI_RESET, comparison_result.sys_time_stats.min);
    }
}

void print_comparison_rss(const BenchmarkResult target_result, const BenchmarkResult comparison_result) {
    if (target_result.max_rss_stats.mean < comparison_result.max_rss_stats.mean) {
        printf(ANSI_GREEN "    target max RSS mean:                 %.0f\n" ANSI_RESET, target_result.max_rss_stats.mean);
        printf(ANSI_RED   "    comparison max RSS mean:             %.0f\n\n" ANSI_RESET, comparison_result.max_rss_stats.mean);
    } else {
        printf(ANSI_RED   "    target max RSS mean:                 %.0f\n" ANSI_RESET, target_result.max_rss_stats.mean);
        printf(ANSI_GREEN "    comparison max RSS mean:             %.0f\n\n" ANSI_RESET, comparison_result.max_rss_stats.mean);
    }
    if (target_result.max_rss_stats.median < comparison_result.max_rss_stats.median) {
        printf(ANSI_GREEN "    target max RSS median:               %.0f\n" ANSI_RESET, target_result.max_rss_stats.median);
        printf(ANSI_RED   "    comparison max RSS median:           %.0f\n\n" ANSI_RESET, comparison_result.max_rss_stats.median);
    } else {
        printf(ANSI_RED   "    target max RSS median:               %.0f\n" ANSI_RESET, target_result.max_rss_stats.median);
        printf(ANSI_GREEN "    comparison max RSS median:           %.0f\n\n" ANSI_RESET, comparison_result.max_rss_stats.median);
    }
    if (target_result.max_rss_stats.stddev < comparison_result.max_rss_stats.stddev) {
        printf(ANSI_GREEN "    target max RSS standard deviation:  %.2f\n" ANSI_RESET, target_result.max_rss_stats.stddev);
        printf(ANSI_RED   "    comparison max RSS standard deviation:  %.2f\n\n" ANSI_RESET, comparison_result.max_rss_stats.stddev);
    } else {
        printf(ANSI_RED   "    target max RSS standard deviation:  %.2f\n" ANSI_RESET, target_result.max_rss_stats.stddev);
        printf(ANSI_GREEN "    comparison max RSS standard deviation:  %.2f\n\n" ANSI_RESET, comparison_result.max_rss_stats.stddev);
    }
    if (target_result.max_rss_stats.max < comparison_result.max_rss_stats.max) {
        printf(ANSI_GREEN "    target max RSS max:                  %.0f\n" ANSI_RESET, target_result.max_rss_stats.max);
        printf(ANSI_RED   "    comparison max RSS max:              %.0f\n\n" ANSI_RESET, comparison_result.max_rss_stats.max);
    } else {
        printf(ANSI_RED   "    target max RSS max:                  %.0f\n" ANSI_RESET, target_result.max_rss_stats.max);
        printf(ANSI_GREEN "    comparison max RSS max:              %.0f\n\n" ANSI_RESET, comparison_result.max_rss_stats.max);
    }
    if (target_result.max_rss_stats.min < comparison_result.max_rss_stats.min) {
        printf(ANSI_GREEN "    target max RSS min:                  %.0f\n" ANSI_RESET, target_result.max_rss_stats.min);
        printf(ANSI_RED   "    comparison max RSS min:              %.0f\n\n" ANSI_RESET, comparison_result.max_rss_stats.min);
    } else {
        printf(ANSI_RED   "    target max RSS min:                  %.0f\n" ANSI_RESET, target_result.max_rss_stats.min);
        printf(ANSI_GREEN "    comparison max RSS min:              %.0f\n\n" ANSI_RESET, comparison_result.max_rss_stats.min);
    }
}

void print_comparison_result(BenchmarkResult target_result, BenchmarkResult comparison_result, config_t cfg) {
    printf("Comparison:\n");
    printf("----\n");
    if (cfg.show_realtime || cfg.show_all) {
        printf("----\n");
        print_comparison_real(target_result, comparison_result);
        printf("----\n");
    }
    if (cfg.show_cpu_times || cfg.show_all) {
        printf("----\n");
        print_comparison_user(target_result, comparison_result);
        printf("----\n");
        print_comparison_sys(target_result, comparison_result);
        printf("----\n");
    }
    if (cfg.show_max_rss || cfg.show_all) {
        printf("----\n");
        print_comparison_rss(target_result, comparison_result);
        printf("----\n");
    }
    if (cfg.show_exit_code || cfg.show_all) {
        printf("----\n");
        printf("    most recent target exit code: %d\n", target_result.exit_code);
        printf("----\n");
        printf("    most recent comparison exit code: %d\n", comparison_result.exit_code);
        printf("----\n");
    }
}
