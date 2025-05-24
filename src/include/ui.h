//
// Created by doepp on 24.05.2025.
//

#ifndef UI_H
#define UI_H
#include "utils.h"
void print_benchmark_result(const BenchmarkResult result, const config_t cfg);
void print_progress_bar(int current, int total);
void print_help(void);
void print_version(void);
void print_comparison_result(BenchmarkResult target_result, BenchmarkResult comparison_result, config_t cfg);
#endif //UI_H
