//
// Created by doepp on 23.05.2025.
//

#ifndef RUNNER_H
#define RUNNER_H
#include <stddef.h>
#include <stdint.h>

typedef struct BenchmarkResult {
    double real_time;
    double sys_time;
    double user_time;
    size_t max_rss;
    int exit_code;
} BenchmarkResult;

int run_target(char **argv, BenchmarkResult *result, uint64_t timeout_ms);

#endif //RUNNER_H
