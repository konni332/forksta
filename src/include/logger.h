
#ifndef LOGGER_H
#define LOGGER_H

#include "utils.h"

/**
 * @brief dumps Benchmark results into JSON file
 * @param filename filename to dump in
 * @param cfg configurations
 * @param result Benchmark results to dump
 * @param runs list of runs and all there metrics
 * @param num_runs size of runs array
 * @return success
 */
int dump_json(const char *filename, config_t cfg, BenchmarkResult result, const BenchmarkRun *runs, int num_runs);
/**
 * @brief dumps Benchmark results into CSV file
 * @param filename filename to dump in
 * @param cfg configurations
 * @param result Benchmark results to dump
 * @param runs list of runs and all there metrics
 * @param num_runs size of runs array
 * @return success
 */
int dump_csv(const char *filename, config_t cfg, BenchmarkResult result, const BenchmarkRun *runs, int num_runs);
/**
 * @brief deletes file
 * @param filename name of file
 * @return success
 */
int clean_dump_file(const char *filename);
#endif //LOGGER_H
