//
// Created by doepp on 23.05.2025.
//

#ifndef UTILS_H
#define UTILS_H
#include <stdint.h>
#include "metrics.h"

typedef struct {
    int show_realtime;
    int show_cpu_times;
    int show_max_rss;
    int show_exit_code;
    int show_all;
    int runs;
    int dump_csv;
    int dump_json;
    char *target;
    char **target_cmd;
    int target_args_count;
    uint64_t timeout_ms;
} config_t;

void parse_args(int argc, char **argv, config_t *cfg);
int check_target_cmd(char **target_cmd, int argc);
uint64_t seconds_to_ms(char *seconds);
uint64_t minutes_to_ms(char *minutes);
void print_benchmark_result(BenchmarkResult result, config_t cfg);
void print_progress_bar(int current, int total);
#endif //UTILS_H
