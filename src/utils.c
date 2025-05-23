//
// Created by doepp on 23.05.2025.
//
#include <stdlib.h>
#include "utils.h"
#include <string.h>
#include <stdio.h>
#include "metrics.h"


int check_target_cmd(char **target_cmd, int argc) {
    if (target_cmd == NULL || target_cmd[0] == NULL) {
        return -1;
    }

    int j = 0;
    while (target_cmd[j] != NULL) {
        j++;
        if (j > (argc)) {
            fprintf(stderr, "Error: target_cmd array not NULL-terminated\n");
            return -1;
        }
    }
    return 0;
}

void parse_args(int argc, char **argv, config_t *cfg) {
    if (argc < 2) {
        fprintf(stderr, "Missing target\n");
        exit(1);
    }

    // Default Werte
    cfg->help = 0;
    cfg->version = 0;
    cfg->show_realtime = 1;
    cfg->show_cpu_times = 0;
    cfg->show_max_rss = 0;
    cfg->show_exit_code = 0;
    cfg->show_all = 0;
    cfg->runs = 1;
    cfg->dump_csv = 0;
    cfg->dump_json = 0;
    cfg->target = NULL;
    cfg->target_cmd = NULL;
    cfg->target_args_count = 0;
    cfg->timeout_ms = DEFAULT_TIMEOUT_MS;

    int i = 1;
    // Optionen parsen
    for (; i < argc; i++) {
        if (argv[i][0] != '-') {
            // break when a no flag item is found!
            break;
        }

        // Flags auswerten
        if (strcmp(argv[i], "-r") == 0) {
            cfg->show_realtime = 1;
            cfg->show_all = 0;
        } else if (strcmp(argv[i], "-c") == 0) {
            cfg->show_cpu_times = 1;
            cfg->show_all = 0;
        } else if (strcmp(argv[i], "-m") == 0) {
            cfg->show_max_rss = 1;
            cfg->show_all = 0;
        } else if (strcmp(argv[i], "-e") == 0) {
            cfg->show_exit_code = 1;
            cfg->show_all = 0;
        } else if (strcmp(argv[i], "-a") == 0) {
            cfg->show_all = 1;
            cfg->show_realtime = 0;
            cfg->show_cpu_times = 0;
            cfg->show_max_rss = 0;
            cfg->show_exit_code = 0;
        } else if (strncmp(argv[i], "--dump", 6) == 0) {
            if (i + 1 < argc) {
                if (strcmp(argv[i + 1], "csv") == 0) {
                    cfg->dump_csv = 1;
                    i++;
                } else if (strcmp(argv[i + 1], "json") == 0) {
                    cfg->dump_json = 1;
                    i++;
                } else {
                    fprintf(stderr, "Invalid dump format: %s\n", argv[i + 1]);
                    exit(1);
                }
            } else {
                fprintf(stderr, "Missing dump format\n");
                exit(1);
            }
        } else if (strcmp(argv[i], "--runs") == 0) {
            if (i + 1 < argc) {
                cfg->runs = atoi(argv[i + 1]);
                if (cfg->runs < 1) cfg->runs = 1;
                i++;
            } else {
                fprintf(stderr, "Missing number of runs\n");
                exit(1);
            }
        } else if (strcmp(argv[i], "--timeout") == 0) {
            if (i + 1 < argc) {
                cfg->timeout_ms = seconds_to_ms(argv[i + 1]);
                i++;
            } else {
                fprintf(stderr, "Missing timeout\n");
                exit(1);
            }
        }
        else if (strcmp(argv[i], "--timeout-m") == 0) {
            if (i + 1 < argc) {
                cfg->timeout_ms = minutes_to_ms(argv[i + 1]);
                i++;
            } else {
                fprintf(stderr, "Missing timeout\n");
                exit(1);
            }
        } else if (strcmp(argv[i], "--help") == 0) {
            cfg->help = 1;
            break;
        } else if (strcmp(argv[i], "--version") == 0) {
            cfg->version = 1;
            break;
        }

        else {
            fprintf(stderr, "Unknown option: %s\n", argv[i]);
            exit(1);
        }
    }

    if (i >= argc) {
        fprintf(stderr, "Missing target program\n");
        exit(1);
    }
    cfg->target = argv[i]; // Programm
    cfg->target_cmd = &argv[i]; // Programm with arguments
    cfg->target_args_count = argc - i;
}

uint64_t seconds_to_ms(char *seconds) {
    if (!seconds) {
        fprintf(stderr, "Invalid seconds value: %s\n", seconds);
        exit(1);
    }
    char *endptr;
    const uint64_t val = strtol(seconds, &endptr, 10);
    if (*endptr != '\0') {
        fprintf(stderr, "Invalid seconds value: %s\n", seconds);
        exit(1);
    }
    return val * 1000;
}

uint64_t minutes_to_ms(char *minutes) {
    if (!minutes) {
        fprintf(stderr, "Invalid minutes value: %s\n", minutes);
        exit(1);
    }
    char *endptr;
    const uint64_t val = strtol(minutes, &endptr, 10);
    if (*endptr != '\0') {
        fprintf(stderr, "Invalid minutes value: %s\n", minutes);
        exit(1);
    }
    return val * 60 * 1000;
}

void print_benchmark_result(BenchmarkResult result, config_t cfg) {
    printf("Final results:\n");
    printf("----\n");
    if (cfg.show_realtime || cfg.show_all) {
        printf("    real time mean: %f\n", result.real_time_stats.mean);
        printf("    real time median: %f\n", result.real_time_stats.median);
        double cv_rt = (result.real_time_stats.stddev / result.real_time_stats.mean) * 100.0;
        printf("    real time standard deviation: %.8f -> cv ~ %.6f%%\n", result.real_time_stats.stddev, cv_rt);
        printf("    real time max: %f -> run %d\n", result.real_time_stats.max, result.real_time_stats.max_run);
        printf("    real time min: %f -> run %d\n", result.real_time_stats.min, result.real_time_stats.min_run);
        printf("----\n");
    }
    if (cfg.show_cpu_times || cfg.show_all) {
        printf("    user time mean: %.4f\n", result.user_time_stats.mean);
        printf("    user time median: %.4f\n", result.user_time_stats.median);
        double cv_us = (result.user_time_stats.stddev / result.user_time_stats.mean) * 100.0;
        printf("    user time standard deviation: %.8f -> cv ~ %.6f%%\n", result.user_time_stats.stddev, cv_us);
        printf("    user time max: %.4f -> run %d\n", result.user_time_stats.max, result.user_time_stats.max_run);
        printf("    user time min: %.4f -> run %d\n", result.user_time_stats.min, result.user_time_stats.min_run);
        printf("----\n");
        printf("    sys time mean: %.4f\n", result.sys_time_stats.mean);
        printf("    sys time median: %.4f\n", result.sys_time_stats.median);
        double cv_sys = (result.sys_time_stats.stddev / result.sys_time_stats.mean) * 100.0;
        printf("    sys time standard deviation: %.8f -> cv ~ %.6f%%\n", result.sys_time_stats.stddev, cv_sys);
        printf("    sys time max: %.4f -> run %d\n", result.sys_time_stats.max, result.sys_time_stats.max_run);
        printf("    sys time min: %.4f -> run %d\n", result.sys_time_stats.min, result.sys_time_stats.min_run);
        printf("----\n");
    }
    if (cfg.show_max_rss || cfg.show_all) {
        printf("    max rss mean: %0.f\n", result.max_rss_stats.mean);
        printf("    max rss median: %0.f\n", result.max_rss_stats.median);
        double cv_rss = (result.max_rss_stats.stddev / result.max_rss_stats.mean) * 100.0;
        printf("    max rss standard deviation: %.2f -> cv ~ %.6f%%\n", result.max_rss_stats.stddev, cv_rss);
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

// int color_enabled();
//
// #define COLOR_IF(x) (color_enabled() ? x : "")
// #define ANSI_RESET   COLOR_IF("\x1b[0m")
// #define ANSI_BOLD    COLOR_IF("\x1b[1m")
// #define ANSI_CYAN    COLOR_IF("\x1b[36m")
// #define ANSI_GREEN   COLOR_IF("\x1b[32m")
// #define ANSI_YELLOW  COLOR_IF("\x1b[33m")
//
// int color_enabled() {
//     return isatty(STDOUT_FILENO);
// }
//




void print_version() {
    printf(ANSI_BOLD ANSI_GREEN "forksta" ANSI_RESET " version " ANSI_CYAN VERSION"\n" ANSI_RESET);
    printf(ANSI_YELLOW "© 2025 konni332. All rights reserved.\n" ANSI_RESET);
}

void print_help() {
    printf(ANSI_BOLD ANSI_GREEN "Usage: " ANSI_RESET "forksta [options] <program> [args...]\n\n");

    printf(ANSI_BOLD "Options:\n" ANSI_RESET);
    printf(ANSI_CYAN "  -r            " ANSI_RESET "Show real time (default)\n");
    printf(ANSI_CYAN "  -c            " ANSI_RESET "Show CPU user/sys time\n");
    printf(ANSI_CYAN "  -m            " ANSI_RESET "Show max RSS (memory usage)\n");
    printf(ANSI_CYAN "  -e            " ANSI_RESET "Show exit code\n");
    printf(ANSI_CYAN "  -a            " ANSI_RESET "Show all metrics\n");
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



