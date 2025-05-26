
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
    double cv_percent;
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

/**
 * @brief initializes Stats struct
 * @param stats Benchmark Stats struct
 * @param runs number of runs
 */
void init_stats(BenchmarkStats *stats, const int runs);

/**
 * @brief initializes Result struct
 * @param result Benchmark Result struct
 * @param runs number of runs for Benchmark
 */
void init_benchmark_result(BenchmarkResult *result, const int runs);

/**
 * @brief initializes Run struct
 * @param run Benchmark Run struct
 */
void init_benchmark_run(BenchmarkRun *run);

/**
 * @brief initializes Benchmark struct
 * @param benchmark benchmark struct
 * @param runs number of runs for Benchmark
 */
void init_benchmark(Benchmark *benchmark, const int runs);

/**
 * @brief frees all necessary memory
 * @param benchmark Benchmark struct
 */
void destroy_benchmark(Benchmark *benchmark);

/**
 *
 * @param values values array
 * @param count size of values array
 * @return median of values
 */
double get_median(double *values, int count);

/**
 *
 * @param values values array
 * @param count size of value array
 * @return q1 of values
 */
double get_q1(double *values, int count);

/**
 *
 * @param values array of values
 * @param count size of values array
 * @return q3 of values
 */
double get_q3(double *values, int count);

/**
 *
 * @param values array of values
 * @param count size of values array
 * @param mean mean of values
 * @return standard deviation
 */
double calculate_stddev(const double *values, const int count, const double mean);

/**
 *
 * @param stddev standard deviation
 * @param mean mean
 * @return cv in percent
 */
double calculate_cv_percent(const double stddev, const double mean);

/**
 * @brief calculates mean, median, standard deviation and cv% from runs
 * @param benchmark results of Benchmark
 */
void calculate_stats(Benchmark *benchmark);

#endif //METRICS_H
