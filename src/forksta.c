#include <stdio.h>
#include <stdlib.h>

#include "runner.h"
#include "metrics.h"
#include "utils.h"



#ifdef _WIN32
    #include <windows.h>
    #define SLEEP_MS(ms) Sleep(ms)
#else
    #include <unistd.h>
    #define SLEEP_MS(ms) usleep((ms) * 1000)
#endif


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
    double *runs_realtime = malloc(sizeof(double) * cfg.runs);
    double *runs_sys_time = malloc(sizeof(double) * cfg.runs);
    double *runs_user_time = malloc(sizeof(double) * cfg.runs);
    double *runs_max_rss = malloc(sizeof(double) * cfg.runs);

    int ran = 1;
    int num_fails = 0;
    int valid_runs = 0;

    printf("Running %d times\n", cfg.runs);
    printf("Target: %s\n\n", cfg.target);

    for (int i = 0; i < cfg.runs; ++i) {
        print_progress_bar(i, cfg.runs);
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
            fprintf(stderr, "Error in run %d with exit code: %d\n", i + 1, run_result.exit_code);
            num_fails++;
            if (num_fails > 10) {
                goto cleanup;
            }
            continue;
        }

        // calculate min
        if (valid_runs == 0 || run_result.real_time < stats_realtime.min) {
            stats_realtime.min = run_result.real_time;
            stats_realtime.min_run = i + 1;
        }
        if (valid_runs == 0 || run_result.sys_time < stats_sys_time.min) {
            stats_sys_time.min = run_result.sys_time;
            stats_sys_time.min_run = i + 1;
        }
        if (valid_runs == 0 || run_result.user_time < stats_user_time.min) {
            stats_user_time.min = run_result.user_time;
            stats_user_time.min_run = i + 1;
        }
        if (valid_runs == 0 || run_result.max_rss < stats_max_rss.min) {
            stats_max_rss.min = run_result.max_rss;
            stats_max_rss.min_run = i + 1;
        }

        // calculate max
        if (valid_runs == 0 || run_result.real_time > stats_realtime.max) {
            stats_realtime.max = run_result.real_time;
            stats_realtime.max_run = i + 1;
        }
        if (valid_runs == 0 || run_result.sys_time > stats_sys_time.max) {
            stats_sys_time.max = run_result.sys_time;
            stats_sys_time.max_run = i + 1;
        }
        if (valid_runs == 0 || run_result.user_time > stats_user_time.max) {
            stats_user_time.max = run_result.user_time;
            stats_user_time.max_run = i + 1;
        }
        if (valid_runs == 0 || run_result.max_rss > stats_max_rss.max) {
            stats_max_rss.max = run_result.max_rss;
            stats_max_rss.max_run = i + 1;
        }


        // calculate means
        stats_realtime.mean += run_result.real_time;
        stats_sys_time.mean += run_result.sys_time;
        stats_user_time.mean += run_result.user_time;
        stats_max_rss.mean += run_result.max_rss;

        // calculate medians
        runs_realtime[i] = run_result.real_time;
        runs_sys_time[i] = run_result.sys_time;
        runs_user_time[i] = run_result.user_time;
        runs_max_rss[i] = run_result.max_rss;

        // Print run
        if (run_result.exit_code == 0) {
            valid_runs++;
        }
        else if ((i - valid_runs) < 50) printf("Run %d: finished with exit code %d\n", i + 1, run_result.exit_code);
    }
    print_progress_bar(cfg.runs, cfg.runs);
    printf("\n");
    printf("\n");

    if (valid_runs == 0) {
        fprintf(stderr, "Error: No valid runs\n\n");
        goto cleanup;
    }

    if (valid_runs < cfg.runs) {
        fprintf(stderr, "Warning: %d runs failed!\n\n", (cfg.runs - valid_runs));
    } else {
        printf("All runs finished successfully\n\n");
    }

    // calculate means
    stats_realtime.mean /= valid_runs;
    stats_sys_time.mean /= valid_runs;
    stats_user_time.mean /= valid_runs;
    stats_max_rss.mean /= valid_runs;

    // calculate stddev
    stats_realtime.stddev = calculate_stddev(runs_realtime, valid_runs, stats_realtime.mean);
    stats_sys_time.stddev = calculate_stddev(runs_sys_time, valid_runs, stats_sys_time.mean);
    stats_user_time.stddev = calculate_stddev(runs_user_time, valid_runs, stats_user_time.mean);
    stats_max_rss.stddev = calculate_stddev(runs_max_rss, valid_runs, stats_max_rss.mean);

    // calculate medians
    stats_realtime.median = get_median(runs_realtime, valid_runs);
    stats_sys_time.median = get_median(runs_sys_time, valid_runs);
    stats_user_time.median = get_median(runs_user_time, valid_runs);
    stats_max_rss.median = get_median(runs_max_rss, valid_runs);

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
    if (runs_realtime) free(runs_realtime);
    if (runs_sys_time) free(runs_sys_time);
    if (runs_user_time) free(runs_user_time);
    if (runs_max_rss) free(runs_max_rss);

    return ran == 0 ? 0 : 1;
}
