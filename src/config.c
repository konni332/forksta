//
// Created by doepp on 25.05.2025.
//
#include "config.h"

#include <stdlib.h>
#include "utils.h"

void free_cmd_line(char **cmd_line) {
    if (!cmd_line) return;
    for (int i = 0; cmd_line[i] != NULL; i++) {
        free(cmd_line[i]);
    }
    free(cmd_line);
}

void destroy_config(config_t *cfg) {
    if (!cfg) return;
    if (cfg->target) free(cfg->target);
    if (cfg->comparison) free(cfg->comparison);
    free_cmd_line(cfg->target_cmd);
    free_cmd_line(cfg->comparison_cmd);
}

void init_config(config_t *cfg) {
    if (!cfg) return;
    // Default Werte
    cfg->help = 0;
    cfg->version = 0;
    cfg->show_realtime = 1;
    cfg->show_cpu_times = 0;
    cfg->show_max_rss = 0;
    cfg->show_exit_code = 0;
    cfg->show_all = 0;
    cfg->runs = 1;
    cfg->dump_csv = 0;
    cfg->dump_json = 0;
    cfg->target = NULL;
    cfg->target_cmd = NULL;
    cfg->target_args_count = 0;
    cfg->comparison = NULL;
    cfg->comparison_cmd = NULL;
    cfg->comparison_args_count = 0;
    cfg->timeout_ms = DEFAULT_TIMEOUT_MS;
    cfg->visualize = 0;
    cfg->visual_rep = NULL;
    cfg->warmup_runs = 5;
}


