//
// Created by doepp on 23.05.2025.
//

#ifndef LOGGER_H
#define LOGGER_H
#include <stdio.h>

#include "utils.h"


int dump_json(config_t cfg, BenchmarkResult result, BenchmarkRun *run, int num_runs);
int dump_csv(config_t cfg, BenchmarkResult result, BenchmarkRun *run, int num_runs);

#endif //LOGGER_H
