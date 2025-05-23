//
// Created by doepp on 23.05.2025.
//
#include "metrics.h"
#include <math.h>
#include <stdlib.h>

void init_stats(BenchmarkStats *stats) {
    stats->mean = 0;
    stats->median = 0;
    stats->min = 0;
    stats->min_run = -1;
    stats->max = 0;
    stats->max_run = -1;
    stats->stddev = 0;
}


int cmp(const void *a, const void *b) {
    double diff = *(double *)a - *(double *)b;
    if (diff < 0) return -1;
    if (diff > 0) return 1;
    return 0;
}

double get_median(double *values, int count) {
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

    return sqrt(sum / (count - 1));
}

double calculate_cv_percent(const double stddev, const double mean) {
    return (stddev / mean) * 100.0;
}
