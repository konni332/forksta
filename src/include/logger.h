
#ifndef LOGGER_H
#define LOGGER_H

#include "utils.h"

int dump_json(const char *filename, config_t cfg, BenchmarkResult result, const BenchmarkRun *runs, int num_runs);
int dump_csv(const char *filename, config_t cfg, BenchmarkResult result, const BenchmarkRun *runs, int num_runs);

#endif //LOGGER_H
