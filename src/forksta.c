#include <stdio.h>
#include "runner.h"
#include <getopt.h>

#include "utils.h"

int main(int argc, char *argv[]) {
    config_t cfg;
    BenchmarkResult result;

    parse_args(argc, argv, &cfg);
    int ran = 1;
    int num_fails = 0;

    // check target command for validity
    if (check_target_cmd(cfg.target_cmd, cfg.target_args_count) != 0) {
        fprintf(stderr, "Error in target command\n");
        return -1;
    }

    for (int i = 0; i < cfg.runs; ++i) {

        ran = run_target(cfg.target_cmd, &result, cfg.timeout_ms);
        if (ran != 0) {
            fprintf(stderr, "Error in run %d\n", i + 1);
            num_fails++;
            if (num_fails > 10) {
                return -1;
            }
            continue;
        }
        if (result.exit_code != 0) {
            fprintf(stderr, "Error in run %d: exit code %d\n", i, result.exit_code);
            num_fails++;
            if (num_fails > 10) {
                return -1;
            }
        }

        printf("Run %d:\n", i + 1);
        if (cfg.show_realtime || cfg.show_all) {
            printf("    real time: %f\n", result.real_time);
        }
        if (cfg.show_cpu_times || cfg.show_all) {
            printf("    user time: %f\n", result.user_time);
            printf("    sys time: %f\n", result.sys_time);
        }
        if (cfg.show_max_rss || cfg.show_all) {
            printf("    max rss: %llu\n", result.max_rss);
        }
        if ((cfg.show_exit_code || cfg.show_all) && result.exit_code == 0) {
            printf("    exit code: %d\n", result.exit_code);
        }
    }
    return ran == 0 ? 0 : 1;
}
