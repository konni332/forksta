//
// Created by doepp on 23.05.2025.
//

#ifndef RUNNER_H
#define RUNNER_H
#include <stdint.h>
#include "metrics.h"


int run_target(char **argv, BenchmarkRun *result, uint64_t timeout_ms);

#endif //RUNNER_H
