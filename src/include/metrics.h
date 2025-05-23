//
// Created by doepp on 23.05.2025.
//

#ifndef METRICS_H
#define METRICS_H
#include <stddef.h>

typedef struct BenchmarkRun {
    double real_time;
    double sys_time;
    double user_time;
    double max_rss;
    int exit_code;
} BenchmarkRun;

typedef struct BenchmarkStats {
    double mean;
    double median;
    double min;
    double max;
    double stddev;
} BenchmarkStats;

typedef struct BenchmarkResult {
    BenchmarkStats real_time_stats;
    BenchmarkStats sys_time_stats;
    BenchmarkStats user_time_stats;
    BenchmarkStats max_rss_stats;
    int exit_code;
} BenchmarkResult;

void init_stats(BenchmarkStats *stats);

double get_median(double *values, int count);

#endif //METRICS_H
