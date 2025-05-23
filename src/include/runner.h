//
// Created by doepp on 23.05.2025.
//

#ifndef RUNNER_H
#define RUNNER_H

typedef struct BenchmarkResult {
    double real_time;
    double sys_time;
    double user_time;
    double memory_usage;
    int exit_code;
} BenchmarkResult;

int run_target(char **argv, BenchmarkResult *result);

#endif //RUNNER_H
