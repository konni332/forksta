#include <stdio.h>
#include <stdlib.h>

#include "runner.h"
#include "metrics.h"
#include "utils.h"

int main(int argc, char *argv[]) {
    config_t cfg;
    BenchmarkRun run_result;
    BenchmarkStats stats_realtime;
    BenchmarkStats stats_sys_time;
    BenchmarkStats stats_user_time;
    BenchmarkStats stats_max_rss;
    init_stats(&stats_realtime);
    init_stats(&stats_sys_time);
    init_stats(&stats_user_time);
    init_stats(&stats_max_rss);



    parse_args(argc, argv, &cfg);
    if (cfg.target == NULL) {
        fprintf(stderr, "Error in target command: NULL\n");
        return -1;
    }

    // check target command for validity
    if (check_target_cmd(cfg.target_cmd, cfg.target_args_count) != 0) {
        fprintf(stderr, "Error in target command\n");
        return -1;
    }

    // allocate medians
    double *median_realtime = malloc(sizeof(double) * cfg.runs);
    double *median_sys_time = malloc(sizeof(double) * cfg.runs);
    double *median_user_time = malloc(sizeof(double) * cfg.runs);
    double *median_max_rss = malloc(sizeof(double) * cfg.runs);

    int ran = 1;
    int num_fails = 0;

    for (int i = 0; i < cfg.runs; ++i) {

        ran = run_target(cfg.target_cmd, &run_result, cfg.timeout_ms);
        if (ran != 0) {
            fprintf(stderr, "Error in run %d\n", i + 1);
            num_fails++;
            if (num_fails > 10) {
                goto cleanup;
            }
            continue;
        }
        if (run_result.exit_code != 0) {
            fprintf(stderr, "Error in run %d with exit code: %d\n", i, run_result.exit_code);
            num_fails++;
            if (num_fails > 10) {
                goto cleanup;
            }
            continue;
        }

        // calculate means
        stats_realtime.mean += run_result.real_time;
        stats_sys_time.mean += run_result.sys_time;
        stats_user_time.mean += run_result.user_time;
        stats_max_rss.mean += run_result.max_rss;

        // calculate medians
        median_realtime[i] = run_result.real_time;
        median_sys_time[i] = run_result.sys_time;
        median_user_time[i] = run_result.user_time;
        median_max_rss[i] = run_result.max_rss;

        // Print run
        printf("Run %d:\n", i + 1);
    }


    const int valid_runs = cfg.runs - num_fails;
    // calculate means
    stats_realtime.mean /= valid_runs;
    stats_sys_time.mean /= valid_runs;
    stats_user_time.mean /= valid_runs;
    stats_max_rss.mean /= valid_runs;

    // calculate medians
    stats_realtime.median = get_median(median_realtime, valid_runs);
    stats_sys_time.median = get_median(median_sys_time, valid_runs);
    stats_user_time.median = get_median(median_user_time, valid_runs);
    stats_max_rss.median = get_median(median_max_rss, valid_runs);

    // set final stats
    BenchmarkResult result;
    result.real_time_stats = stats_realtime;
    result.user_time_stats = stats_user_time;
    result.sys_time_stats = stats_sys_time;
    result.max_rss_stats = stats_max_rss;
    result.exit_code = ran;

    print_benchmark_result(result, cfg);

cleanup:
    // free
    if (median_realtime) free(median_realtime);
    if (median_sys_time) free(median_sys_time);
    if (median_user_time) free(median_user_time);
    if (median_max_rss) free(median_max_rss);

    return ran == 0 ? 0 : 1;
}
