#include "metrics.h"
#include <math.h>
#include <stdlib.h>
#include <float.h>

void init_stats(BenchmarkStats *stats, const int runs) {
    stats->mean = 0;
    stats->median = 0;
    stats->min = 0;
    stats->min_run = -1;
    stats->max = 0;
    stats->max_run = -1;
    stats->stddev = 0;
    stats->runs = (double *)malloc(sizeof(double) * runs);
    if (!stats->runs) {
        perror("malloc failed");
        exit(1);
    }
    for (int i = 0; i < runs; i++) {
        stats->runs[i] = 0;
    }
}

void init_benchmark_result(BenchmarkResult *result, const int runs) {
    init_stats(&result->real_time_stats, runs);
    init_stats(&result->user_time_stats, runs);
    init_stats(&result->sys_time_stats, runs);
    init_stats(&result->max_rss_stats, runs);
    result->exit_code = 0;
}

void init_benchmark_run(BenchmarkRun *run) {
    run->real_time = 0;
    run->user_time = 0;
    run->sys_time = 0;
    run->max_rss = 0;
    run->exit_code = 0;
}

void init_benchmark(Benchmark *benchmark, const int runs) {
    benchmark->valid_runs = 0;
    benchmark->runs = runs;
    benchmark->ran = 1;
    benchmark->num_fails = 0;
    init_benchmark_result(&benchmark->result, runs);
    benchmark->runs_array = (BenchmarkRun *)malloc(sizeof(BenchmarkRun) * runs);
    if (!benchmark->runs_array) {
        perror("malloc failed");
        exit(1);
    }
    for (int i = 0; i < runs; i++) {
        init_benchmark_run(&benchmark->runs_array[i]);
    }
}

void destroy_benchmark(Benchmark *benchmark) {
    if (!benchmark) return;
    if (benchmark->runs_array) {
        free(benchmark->runs_array);
        benchmark->runs_array = NULL;
    }
    if (benchmark->result.real_time_stats.runs) {
        free(benchmark->result.real_time_stats.runs);
        benchmark->result.real_time_stats.runs = NULL;
    }
    if (benchmark->result.user_time_stats.runs) {
        free(benchmark->result.user_time_stats.runs);
        benchmark->result.user_time_stats.runs = NULL;
    }
    if (benchmark->result.sys_time_stats.runs) {
        free(benchmark->result.sys_time_stats.runs);
        benchmark->result.sys_time_stats.runs = NULL;
    }
    if (benchmark->result.max_rss_stats.runs) {
        free(benchmark->result.max_rss_stats.runs);
        benchmark->result.max_rss_stats.runs = NULL;
    }
}

int cmp(const void *a, const void *b) {
    double diff = *(double *)a - *(double *)b;
    if (diff < 0) return -1;
    if (diff > 0) return 1;
    return 0;
}

double get_median(double *values, int count) {
    if (!values) return 0;
    if (count == 0) return 0;
    if (count == 1) return values[0];
    qsort(values, count, sizeof(double), cmp);

    if (count % 2 == 0) {
        return (values[count / 2 - 1] + values[count / 2]) / 2.0;
    }

    return values[count / 2];
}


double calculate_stddev(const double *values, const int count, const double mean) {
    if (count <= 1) return 0.0;
    double sum = 0.0;
    for (int i = 0; i < count; i++) {
        double diff = values[i] - mean;
        sum += diff * diff;
    }

    double stddev = sqrt(sum / (count - 1));
    const double epsilon = 1e-8;
    return (stddev < epsilon) ? 0.0 : stddev;
}


double calculate_cv_percent(const double stddev, const double mean) {
    const double epsilon = 1e-8;

    if (fabs(mean) < epsilon) {
        // Mittelwert zu klein → CV nicht sinnvoll definierbar
        return NAN;
    }

    if (fabs(stddev) < epsilon) {
        // Standardabweichung zu klein → praktisch null Variation
        return 0.0;
    }

    return (stddev / mean) * 100.0;
}


void calculate_stats(Benchmark *benchmark) {
    if (!benchmark) return;
    if (benchmark->valid_runs == 0) return;

    benchmark->result.real_time_stats.median = get_median(benchmark->result.real_time_stats.runs, benchmark->valid_runs);
    benchmark->result.real_time_stats.mean = benchmark->result.real_time_stats.median;
    benchmark->result.real_time_stats.stddev =
        calculate_stddev(benchmark->result.real_time_stats.runs,
            benchmark->valid_runs,
            benchmark->result.real_time_stats.mean);

    benchmark->result.user_time_stats.median = get_median(benchmark->result.user_time_stats.runs, benchmark->valid_runs);
    benchmark->result.user_time_stats.mean = benchmark->result.user_time_stats.median;
    benchmark->result.user_time_stats.stddev =
        calculate_stddev(benchmark->result.user_time_stats.runs,
            benchmark->valid_runs,
            benchmark->result.user_time_stats.mean);

    benchmark->result.sys_time_stats.median = get_median(benchmark->result.sys_time_stats.runs, benchmark->valid_runs);
    benchmark->result.sys_time_stats.mean = benchmark->result.sys_time_stats.median;
    benchmark->result.sys_time_stats.stddev =
        calculate_stddev(benchmark->result.sys_time_stats.runs,
            benchmark->valid_runs,
            benchmark->result.sys_time_stats.mean);

    benchmark->result.max_rss_stats.median = get_median(benchmark->result.max_rss_stats.runs, benchmark->valid_runs);
    benchmark->result.max_rss_stats.mean = benchmark->result.max_rss_stats.median;
    benchmark->result.max_rss_stats.stddev =
        calculate_stddev(benchmark->result.max_rss_stats.runs,
            benchmark->valid_runs,
            benchmark->result.max_rss_stats.mean);
}