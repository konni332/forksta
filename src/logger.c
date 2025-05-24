//
// Created by doepp on 23.05.2025.
//

#include "logger.h"
#include <stdio.h>
#include "utils.h"
#include <inttypes.h>


int dump_csv(const char *filename, config_t cfg, BenchmarkResult result, const BenchmarkRun *runs, int num_runs) {
    FILE *file = fopen(filename, "w");
    if (!file) {
        perror("fopen");
        return -1;
    }

    fprintf(file,
        "run,real_time,sys_time,user_time,max_rss,exit_code\n"
    );

    for (int i = 0; i < num_runs; i++) {
        fprintf(file, "%d,%.6f,%.6f,%.6f,%.0f,%d\n",
            i + 1,
            runs[i].real_time,
            runs[i].sys_time,
            runs[i].user_time,
            runs[i].max_rss,
            runs[i].exit_code
        );
    }

    fprintf(file, "\n");

    fprintf(file, "statistic,mean,median,min,min_run,max,max_run,stddev\n");

    #define WRITE_STATS_ROW(label, stats) \
        fprintf(file, "%s,%.6f,%.6f,%.6f,%d,%.6f,%d,%.6f\n", \
            label, \
            stats.mean, \
            stats.median, \
            stats.min, \
            stats.min_run + 1, \
            stats.max, \
            stats.max_run + 1, \
            stats.stddev \
        );

    WRITE_STATS_ROW("real_time", result.real_time_stats);
    WRITE_STATS_ROW("sys_time", result.sys_time_stats);
    WRITE_STATS_ROW("user_time", result.user_time_stats);
    WRITE_STATS_ROW("max_rss", result.max_rss_stats);

    fprintf(file, "\nsummary_exit_code,%d\n", result.exit_code);

    fclose(file);
    return 0;
}


int dump_json(const char *filename, config_t cfg, BenchmarkResult result, const BenchmarkRun *runs, int num_runs) {
    FILE *file = fopen(filename, "w");
    if (!file) {
        perror("fopen");
        return -1;
    }

    // Header: "meta"
    fprintf(file, "{\n");
    fprintf(file, "  \"meta\": {\n");
    fprintf(file, "    \"target\": \"%s\",\n", cfg.target ? cfg.target : "unknown");

    // "args": Array aus target_cmd
    fprintf(file, "    \"args\": [");
    for (int i = 0; i < cfg.target_args_count; i++) {
        fprintf(file, "\"%s\"%s", cfg.target_cmd[i], (i < cfg.target_args_count - 1) ? ", " : "");
    }
    fprintf(file, "],\n");

    fprintf(file, "    \"runs\": %d,\n", cfg.runs);
    fprintf(file, "    \"timeout_ms\": %" PRIu64 "\n", cfg.timeout_ms);
    fprintf(file, "  },\n");

    // Einzelne runs
    for (int i = 0; i < num_runs; i++) {
        fprintf(file, "  \"run_%d\": {\n", i + 1);
        fprintf(file, "    \"real_time\": %.6f,\n", runs[i].real_time);
        fprintf(file, "    \"sys_time\": %.6f,\n", runs[i].sys_time);
        fprintf(file, "    \"user_time\": %.6f,\n", runs[i].user_time);
        fprintf(file, "    \"max_rss\": %.0f,\n", runs[i].max_rss);
        fprintf(file, "    \"exit_code\": %d\n", runs[i].exit_code);
        fprintf(file, "  },\n");
    }

    // Zusammenfassende Statistik
    fprintf(file, "  \"results\": {\n");

    #define WRITE_STATS(field, name) do { \
        fprintf(file, "    \"%s\": {\n", name); \
        fprintf(file, "      \"mean\": %.6f,\n", result.field.mean); \
        fprintf(file, "      \"median\": %.6f,\n", result.field.median); \
        fprintf(file, "      \"min\": %.6f,\n", result.field.min); \
        fprintf(file, "      \"min_run\": %d,\n", result.field.min_run); \
        fprintf(file, "      \"max\": %.6f,\n", result.field.max); \
        fprintf(file, "      \"max_run\": %d,\n", result.field.max_run); \
        fprintf(file, "      \"stddev\": %.6f\n", result.field.stddev); \
        fprintf(file, "    },\n"); \
    } while(0)

    WRITE_STATS(real_time_stats, "real_time");
    WRITE_STATS(sys_time_stats, "sys_time");
    WRITE_STATS(user_time_stats, "user_time");
    WRITE_STATS(max_rss_stats, "max_rss");

    fprintf(file, "    \"exit_code\": %d\n", result.exit_code);
    fprintf(file, "  }\n");
    fprintf(file, "}\n");

    fclose(file);
    return 0;
}