//
// Created by doepp on 23.05.2025.
//
#include "metrics.h"
#include <stdlib.h>

void init_stats(BenchmarkStats *stats) {
    stats->mean = 0;
    stats->median = 0;
    stats->min = 0;
    stats->max = 0;
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

