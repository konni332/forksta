
#ifndef RUNNER_H
#define RUNNER_H
#include <stdint.h>
#include "metrics.h"
#include "utils.h"
/**
 * @brief runs target and saves results into BenchmarkRun
 * @param argv list of arguments for run
 * @param result struct to save metrics of target run
 * @param timeout_ms the maximum time a target can run before being killed
 * @return success of run
 */
int run_target(char **argv, BenchmarkRun *result, uint64_t timeout_ms);
/**
 * @brief runs the benchmark according to config
 * @param cfg configurations
 * @return success
 */
int run(config_t *cfg);
/**
 * @brief runs a single target
 * @param cfg configurations
 * @return success
 */
int run_single_benchmark(config_t *cfg);
/**
 * @brief runs two targets for comparison
 * @param cfg configurations
 * @return success
 */
int run_comparison(config_t *cfg);
/**
 * @brief loop to run target cfg.runs times
 * @param cfg configurations
 * @param bm struct to save metrics and calculated stats
 * @param target target name
 * @param target_cmd entire target command line
 * @return success
 */
int run_loop(config_t cfg, Benchmark *bm, char *target, char **target_cmd);

#endif //RUNNER_H
