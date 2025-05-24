
#ifndef RUNNER_H
#define RUNNER_H
#include <stdint.h>
#include "metrics.h"
#include "utils.h"

int run_target(char **argv, BenchmarkRun *result, uint64_t timeout_ms);
int run(config_t cfg);
int run_single_benchmark(config_t cfg);
int run_comparison(config_t cfg);
int run_loop(config_t cfg, Benchmark *bm, const char *target, const char **target_cmd);

#endif //RUNNER_H
