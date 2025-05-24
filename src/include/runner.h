
#ifndef RUNNER_H
#define RUNNER_H
#include <stdint.h>
#include "metrics.h"


int run_target(char **argv, BenchmarkRun *result, uint64_t timeout_ms);
int run(int argc, char **argv);
#endif //RUNNER_H
