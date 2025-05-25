
#ifndef CONFIG_H
#define CONFIG_H
#include <stdint.h>

typedef struct config_t {
    int help;
    int version;
    int dependencies;
    int show_realtime;
    int show_cpu_times;
    int show_max_rss;
    int show_exit_code;
    int show_all;
    int runs;
    int dump_csv;
    int dump_json;
    char *target;
    char **target_cmd;
    int target_args_count;
    uint64_t timeout_ms;
    char *comparison;
    char **comparison_cmd;
    int comparison_args_count;
    int visualize;
    char target_dump_file[1024];
    char comparison_dump_file[1024];
    char *visual_rep;
    int warmup_runs;
    int clean_dump;
} config_t;

void init_config(config_t *cfg);
void destroy_config(config_t *cfg);

#endif //CONFIG_H
