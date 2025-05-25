
#ifndef LOGGER_H
#define LOGGER_H

#include "utils.h"

int dump_json(const char *filename, config_t cfg, BenchmarkResult result, const BenchmarkRun *runs, int num_runs);
int dump_csv(const char *filename, config_t cfg, BenchmarkResult result, const BenchmarkRun *runs, int num_runs);
int clean_dump_file(const char *filename);
#endif //LOGGER_H
