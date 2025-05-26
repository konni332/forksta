//
// Created by doepp on 24.05.2025.
//

#ifndef UI_H
#define UI_H
#include "utils.h"
/**
 * @brief prints results according to configurations
 * @param result Benchmark Result struct
 * @param cfg configurations
 */
void print_benchmark_result(const BenchmarkResult result, const config_t cfg);
/**
 * @brief prints progress bar
 * @param current current progress
 * @param total total progress (100%)
 */
void print_progress_bar(int current, int total);
/**
 * @brief print forksta help
 */
void print_help(void);
/**
 * @brief print forksta version
 */
void print_version(void);
/**
 * @brief print dependencies of forksta
 */
void print_dependencies(void);
/**
 *
 * @param stats stats struct
 * @param metric name of metric to print
 * @param runs number of runs
 */
void print_stat(const BenchmarkStats *stats, const char *metric, const int runs);
#endif //UI_H
