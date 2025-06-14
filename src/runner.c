#include "runner.h"
#include "utils.h"
#include "logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ui.h"


int run_warmup(config_t *cfg, char *target, char **target_cmd) {
    if (cfg->warmup_runs <= 0) {
        return 0;
    }
    if (!target || !target_cmd) {
        fprintf(stderr, "Major error in warm up runs. Target: NULL\nAborting...\n");
        exit(1);
    }

    if (!target_exists(target))
    {
        fprintf(stderr, "Major error in warm up runs. Target: %s does not exist!\nAborting...\n", target);
        exit(1);
    }

    BenchmarkRun warmup_run_result; // discarded after warmups
    printf("Warmup: %d runs\n", cfg->warmup_runs);
    for (int i = 0; i < cfg->warmup_runs; ++i) {
        print_progress_bar(i, cfg->warmup_runs);
        int rc = run_target(target_cmd, &warmup_run_result, cfg->timeout_ms);
        if (rc != 0) {
            fprintf(stderr, "Error in warmup run %d\n", i + 1);
            return rc;
        }
    }
    print_progress_bar(cfg->warmup_runs, cfg->warmup_runs);
    printf("\n\n");
    return 0;
}

int run(config_t *cfg) {
    if (cfg->help) {
        print_help();
        return 0;
    }
    if (cfg->version) {
        print_version();
        return 0;
    }
    if (cfg->dependencies) {
        print_dependencies();
        return 0;
    }
    if (cfg->target == NULL) {
        fprintf(stderr, "Error in target: NULL\n");
        return -1;
    }
    if (cfg->target_cmd == NULL) {
        fprintf(stderr, "Error in target command: NULL\n");
    }

    if (cfg->comparison == NULL) {
        return run_single_benchmark(cfg);
    }

    if (cfg->comparison_cmd == NULL) {
        fprintf(stderr, "Error in comparison command: NULL\n");
        return -1;
    }

    return run_comparison(cfg);
}

int run_single_benchmark(config_t *cfg) {
    Benchmark bm;
    // allocate runs arrays
    init_benchmark(&bm, cfg->runs);

    if (check_target_cmd(cfg->target_cmd, cfg->target_args_count) != 0) {
        fprintf(stderr, "Error in target command\n");
        goto cleanup;
    }

    run_warmup(cfg, cfg->target, cfg->target_cmd);
    run_loop(*cfg, &bm, cfg->target, cfg->target_cmd);

    if (bm.valid_runs <= 0) {
        fprintf(stderr, "Error: No valid runs\n\n");
        goto cleanup;
    }

    if (bm.valid_runs < cfg->runs) {
        fprintf(stderr, ANSI_YELLOW "Warning: %d runs failed!\n\n" ANSI_RESET, bm.num_fails);
    } else {
        printf(ANSI_GREEN "All runs finished successfully\n\n" ANSI_RESET);
    }

    calculate_stats(&bm);

    if (cfg->dump_csv) {
        generate_filename(cfg->target_dump_file, 1024, ".csv", &bm);
        if (dump_csv(cfg->target_dump_file, *cfg, bm.result, bm.runs_array, bm.valid_runs) != 0) {
            fprintf(stderr, "Error in dump_csv\n");
            goto cleanup;
        }
    }

    if (cfg->dump_json) {
        generate_filename(cfg->target_dump_file, 1024, ".json", &bm);
        if (dump_json(cfg->target_dump_file, *cfg, bm.result, bm.runs_array, bm.valid_runs) != 0) {
            fprintf(stderr, "Error in dump_json\n");
            goto cleanup;
        }
    }
    if (!cfg->visualize) {
        if (cfg->show_realtime || cfg->show_all) {
            print_stat(&bm.result.real_time_stats, "Real Time (s)", bm.valid_runs);
        }
        if (cfg->show_cpu_times || cfg->show_all) {
            print_stat(&bm.result.sys_time_stats, "System Time (s)", bm.valid_runs);
            print_stat(&bm.result.user_time_stats, "User Time (s)", bm.valid_runs);
        }
        if (cfg->show_max_rss || cfg->show_all) {
            print_stat(&bm.result.max_rss_stats, "Max RSS (kB)", bm.valid_runs);
        }
    }
cleanup:
    destroy_benchmark(&bm);
    return bm.ran == 0 ? 0 : 1;
}

void print_error_below(const char *msg) {
    printf("\n\033[K");
    printf("Error: %s\n", msg);
    printf("\033[F");
}

int run_comparison(config_t *cfg) {
    Benchmark target_bm;
    Benchmark comparison_bm;

    // allocate runs arrays
    init_benchmark(&target_bm, cfg->runs);
    init_benchmark(&comparison_bm, cfg->runs);

    if (check_target_cmd(cfg->target_cmd, cfg->target_args_count) != 0) {
        fprintf(stderr, "Error in target command\n");
        goto cleanup;
    }

    if (check_target_cmd(cfg->comparison_cmd, cfg->comparison_args_count) != 0) {
        fprintf(stderr, "Error in comparison command\n");
        goto cleanup;
    }

    if (!target_exists(cfg->target)) {
        fprintf(stderr, "Error: Target does not exist\n");
        goto cleanup;
    }

    if (!target_exists(cfg->comparison)) {
        fprintf(stderr, "Error: Comparison does not exist\n");
        goto cleanup;
    }

    run_warmup(cfg, cfg->target, cfg->target_cmd);
    run_loop(*cfg, &target_bm, cfg->target, cfg->target_cmd);
    run_warmup(cfg, cfg->comparison, cfg->comparison_cmd);
    run_loop(*cfg, &comparison_bm, cfg->comparison, cfg->comparison_cmd);

    calculate_stats(&target_bm);
    calculate_stats(&comparison_bm);

    if (target_bm.valid_runs <= 0) {
        fprintf(stderr, "Error: No valid target runs\n\n");
        goto cleanup;
    }
    if (comparison_bm.valid_runs <= 0) {
        fprintf(stderr, "Error: No valid comparison runs\n\n");
        goto cleanup;
    }

    if (target_bm.valid_runs < cfg->runs) {
        printf(ANSI_YELLOW "Warning: %d target runs failed!\n\n" ANSI_RESET, target_bm.num_fails);
    }
    else {
        printf(ANSI_GREEN "All target runs finished successfully\n\n" ANSI_RESET);
    }
    if (comparison_bm.valid_runs < cfg->runs) {
        printf(ANSI_YELLOW "Warning: %d comparison runs failed!\n\n" ANSI_RESET, comparison_bm.num_fails);
    } else {
        printf(ANSI_GREEN "All comparison runs finished successfully\n\n" ANSI_RESET);
    }

    if (cfg->dump_csv) {
        generate_filename(cfg->target_dump_file, 1024, ".csv", &target_bm);
        generate_filename(cfg->comparison_dump_file, 1024, ".csv", &comparison_bm);
        if (dump_csv(cfg->target_dump_file, *cfg, target_bm.result, target_bm.runs_array, target_bm.valid_runs) != 0) {
            fprintf(stderr, "Error in dump_csv\n");
            goto cleanup;
        }
        if (dump_csv(cfg->comparison_dump_file, *cfg, comparison_bm.result, comparison_bm.runs_array, comparison_bm.valid_runs) != 0) {
            fprintf(stderr, "Error in dump_csv\n");
            goto cleanup;
        }
    }

    if (cfg->dump_json) {
        generate_filename(cfg->target_dump_file, 1024, ".json", &target_bm);
        generate_filename(cfg->comparison_dump_file, 1024, ".json", &comparison_bm);
        if (dump_json(cfg->target_dump_file, *cfg, target_bm.result, target_bm.runs_array, target_bm.valid_runs) != 0) {
            fprintf(stderr, "Error in dump_json\n");
            goto cleanup;
        }
        if (dump_json(cfg->comparison_dump_file, *cfg, comparison_bm.result, comparison_bm.runs_array, comparison_bm.valid_runs) != 0) {
            fprintf(stderr, "Error in dump_json\n");
            goto cleanup;
        }
    }
    printf(ANSI_BOLD ANSI_CYAN "Compare: " ANSI_RESET);
    print_args(cfg->target_cmd, cfg->target_args_count);
    printf("\n");
    printf(ANSI_BOLD ANSI_CYAN "With: " ANSI_RESET);
    print_args(cfg->comparison_cmd, cfg->comparison_args_count);
    printf("\n\n");

    if (!cfg->visualize) {
        if (cfg->show_realtime || cfg->show_all) {
            print_stat(&target_bm.result.real_time_stats, "Real Time (s)", target_bm.valid_runs);
            print_stat(&comparison_bm.result.real_time_stats, "Real Time (s)", comparison_bm.valid_runs);
        }
        if (cfg->show_cpu_times || cfg->show_all) {
            print_stat(&target_bm.result.sys_time_stats, "System Time (s)", target_bm.valid_runs);
            print_stat(&comparison_bm.result.sys_time_stats, "System Time (s)", comparison_bm.valid_runs);

            print_stat(&target_bm.result.user_time_stats, "User Time (s)", target_bm.valid_runs);
            print_stat(&comparison_bm.result.user_time_stats, "User Time (s)", comparison_bm.valid_runs);
        }
        if (cfg->show_max_rss || cfg->show_all) {
            print_stat(&target_bm.result.max_rss_stats, "Max RSS (kB)", target_bm.valid_runs);
            print_stat(&comparison_bm.result.max_rss_stats, "Max RSS (kB)", target_bm.valid_runs);
        }
    }
cleanup:
    destroy_benchmark(&target_bm);
    destroy_benchmark(&comparison_bm);
    return (target_bm.ran == 0) && (comparison_bm.ran == 0) ? 0 : 1;
}

#define STATS_SYS bm->result.sys_time_stats
#define STATS_USER bm->result.user_time_stats
#define STATS_MAX_RSS bm->result.max_rss_stats
#define STATS_REALTIME bm->result.real_time_stats
#define RUNS bm->runs_array
#define C_RUN bm->runs_array[i]
#define MAX_FAILS_HARD cfg.failcap_hard
#define MAX_FAILS_SOFT cfg.failcap_soft


static inline int max_fails_soft(config_t cfg) {
    return (int) (cfg.runs * MAX_FAILS_SOFT);
}


int run_loop(config_t cfg, Benchmark *bm, char *target, char **target_cmd) {
    printf("Running %d times\n", cfg.runs);
    printf("Target: %s\n", target);
    for (int i = 0; i < cfg.runs; ++i) {
        print_progress_bar(i, cfg.runs);
        bm->ran = run_target(target_cmd, &bm->runs_array[i], cfg.timeout_ms);

        if (C_RUN.exit_code != 0 || bm->ran != 0) {
            char msg[512];
            snprintf(msg, 512, "Error in run %d with exit code: %d\n", i + 1, C_RUN.exit_code);
            print_error_below(msg);
            bm->num_fails++;
            if (bm->num_fails >= MAX_FAILS_HARD || bm->num_fails >= max_fails_soft(cfg)) {
                fprintf(stderr, "Too many fails: %d (Soft: %d / Hard: %d)!\nAborting...\n",
                    bm->num_fails, max_fails_soft(cfg), MAX_FAILS_HARD);
                return -1;
            }
            continue;
        }

        // calculate min
        if (bm->valid_runs == 0 || C_RUN.real_time < bm->result.real_time_stats.min) {
            STATS_REALTIME.min = C_RUN.real_time;
            STATS_REALTIME.min_run = i + 1;
        }
        if (bm->valid_runs == 0 || C_RUN.sys_time < STATS_SYS.min) {
            STATS_SYS.min = C_RUN.sys_time;
            STATS_SYS.min_run = i + 1;
        }
        if (bm->valid_runs == 0 || C_RUN.user_time < STATS_USER.min) {
            STATS_USER.min = C_RUN.user_time;
            STATS_USER.min_run = i + 1;
        }
        if (bm->valid_runs == 0 || C_RUN.max_rss < STATS_MAX_RSS.min) {
            STATS_MAX_RSS.min = C_RUN.max_rss;
            STATS_MAX_RSS.min_run = i + 1;
        }

        // calculate max
        if (bm->valid_runs == 0 || C_RUN.real_time > STATS_REALTIME.max) {
            STATS_REALTIME.max = C_RUN.real_time;
            STATS_REALTIME.max_run = i + 1;
        }
        if (bm->valid_runs == 0 || C_RUN.sys_time > STATS_SYS.max) {
            STATS_SYS.max = C_RUN.sys_time;
            STATS_SYS.max_run = i + 1;
        }
        if (bm->valid_runs == 0 || C_RUN.user_time > STATS_USER.max) {
            STATS_USER.max = C_RUN.user_time;
            STATS_USER.max_run = i + 1;
        }
        if (bm->valid_runs == 0 || C_RUN.max_rss > STATS_MAX_RSS.max) {
            STATS_MAX_RSS.max = C_RUN.max_rss;
            STATS_MAX_RSS.max_run = i + 1;
        }


        // calculate means
        STATS_REALTIME.mean += bm->runs_array[i].real_time;
        STATS_SYS.mean += bm->runs_array[i].sys_time;
        STATS_USER.mean += bm->runs_array[i].user_time;
        STATS_MAX_RSS.mean += bm->runs_array[i].max_rss;

        STATS_REALTIME.runs[i] = bm->runs_array[i].real_time;
        STATS_SYS.runs[i] = bm->runs_array[i].sys_time;
        STATS_USER.runs[i] = bm->runs_array[i].user_time;
        STATS_MAX_RSS.runs[i] = bm->runs_array[i].max_rss;

        // Print run
        if (C_RUN.exit_code == 0) {
            bm->valid_runs++;
        }
    }
    print_progress_bar(cfg.runs, cfg.runs);
    printf("\n");
    printf("\n");

    return bm->ran == 0 ? 0 : 1;
}

#ifdef _WIN32 // Windows
#include <windows.h>
#include <psapi.h>
#include <stdint.h>
int run_target(char **argv, BenchmarkRun *run_result, uint64_t timeout_ms) {
    if (!argv || !run_result) {
        fprintf(stderr, "Invalid arguments in run_target\n");
        return -1;
    }

    LARGE_INTEGER freq, start, end;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&start);

    // build command line
    char cmdln[1024] = {0};
    for (int i = 0; argv[i]; ++i) {
        strcat(cmdln, "\"");
        strcat(cmdln, argv[i]);
        strcat(cmdln, "\" ");
    }

    STARTUPINFO si = { sizeof(si) };
    PROCESS_INFORMATION pi;

    if (!CreateProcess(NULL, cmdln, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        perror("CreateProcess failed");
        return -1;
    }


    DWORD wait_result = WaitForSingleObject(pi.hProcess, timeout_ms);
    if (wait_result == WAIT_TIMEOUT) {
        TerminateProcess(pi.hProcess, 1);
        char msg[512];
        snprintf(msg, 512, ANSI_RED "Timeout: killed target: %d after: %llums\n" ANSI_RESET, pi.dwProcessId, timeout_ms);
        print_error_below(msg);
        run_result->exit_code = -1;
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        return -1;
    }

    DWORD exit_code;
    GetExitCodeProcess(pi.hProcess, &exit_code);

    QueryPerformanceCounter(&end);

    // calculate real time
    run_result->real_time = (double) (end.QuadPart - start.QuadPart) / (double) freq.QuadPart;

    // calculate cpu times
    FILETIME creation_time, exit_time, kernel_time, user_time;
    if (GetProcessTimes(pi.hProcess, &creation_time, &exit_time, &kernel_time, &user_time)) {
        ULARGE_INTEGER k, u;
        k.LowPart = kernel_time.dwLowDateTime;
        k.HighPart = kernel_time.dwHighDateTime;
        u.LowPart = user_time.dwLowDateTime;
        u.HighPart = user_time.dwHighDateTime;

        // 100-nanoseconds to seconds
        run_result->sys_time = (double) k.QuadPart * 1e-7;
        run_result->user_time = (double) u.QuadPart * 1e-7;
    } else {
        run_result->sys_time = 0;
        run_result->user_time = 0;
    }

    // RAM peak in bytes
    PROCESS_MEMORY_COUNTERS pmc;
    if (GetProcessMemoryInfo(pi.hProcess, &pmc, sizeof(pmc))) {
        run_result->max_rss = pmc.PeakWorkingSetSize / 1024;
    } else {
        run_result->max_rss = 0;
    }

    // EXIT code
    run_result->exit_code = (int)exit_code;

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    return 0;
}
#else // POSIX (macOS/Linux)
#include <sys/resource.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>

int run_target(char **argv, BenchmarkRun *result, uint64_t timeout_ms) {
    if (!argv || !result) {
        fprintf(stderr, "Invalid arguments in run_target\n");
        return -1;
    }

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    struct rusage usage;
    int status = 0;
    pid_t pid = fork();

    if (pid < 0) {
        perror("fork failed");
        return -1;
    }

    if (pid == 0) {
        // child
        execvp(argv[0], argv);
        perror("execvp failed");
        exit(1);
    }

    // Timeout loop
    const useconds_t interval_us = 10 * 1000; // 10 ms
    uint64_t waited_ms = 0;

    while (1) {
        pid_t done = waitpid(pid, &status, WNOHANG);
        if (done == -1) {
            perror("waitpid failed");
            return -1;
        } else if (done > 0) {
            break; // process exited
        }

        usleep(interval_us);
        waited_ms += 10;

        if (waited_ms >= timeout_ms) {
            char msg[512];
            snprintf(msg, sizeof(msg), ANSI_RED "Timeout: killed target %d after %llums\n" ANSI_RESET, pid, (unsigned long long)timeout_ms);
            print_error_below(msg);
            kill(pid, SIGKILL);
            waitpid(pid, &status, 0);
            result->exit_code = -1;
            clock_gettime(CLOCK_MONOTONIC, &end);
            result->real_time = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
            result->user_time = 0;
            result->sys_time = 0;
            result->max_rss = 0;
            return -1;
        }
    }

    clock_gettime(CLOCK_MONOTONIC, &end);
    getrusage(RUSAGE_CHILDREN, &usage);

    result->real_time = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    result->user_time = usage.ru_utime.tv_sec + usage.ru_utime.tv_usec / 1e6;
    result->sys_time = usage.ru_stime.tv_sec + usage.ru_stime.tv_usec / 1e6;
    result->max_rss = usage.ru_maxrss; // already in KB on Linux
    result->exit_code = WIFEXITED(status) ? WEXITSTATUS(status) : -1;

    return 0;
}



#endif