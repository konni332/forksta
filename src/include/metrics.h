
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
    int min_run;
    double max;
    int max_run;
    double stddev;
    double *runs;
} BenchmarkStats;

typedef struct BenchmarkResult {
    BenchmarkStats real_time_stats;
    BenchmarkStats sys_time_stats;
    BenchmarkStats user_time_stats;
    BenchmarkStats max_rss_stats;
    int exit_code;
} BenchmarkResult;

typedef struct Benchmark {
    int runs;
    int valid_runs;
    int num_fails;
    int ran;
    BenchmarkRun *runs_array;
    BenchmarkResult result;
} Benchmark;

void init_stats(BenchmarkStats *stats, const int runs);

void init_benchmark_result(BenchmarkResult *result, const int runs);

void init_benchmark_run(BenchmarkRun *run);

void init_benchmark(Benchmark *benchmark, const int runs);

void destroy_benchmark(Benchmark *benchmark);

double get_median(double *values, int count);

double calculate_stddev(const double *values, const int count, const double mean);

double calculate_cv_percent(const double stddev, const double mean);

void calculate_stats(Benchmark *benchmark);

#endif //METRICS_H
