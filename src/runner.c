#include "runner.h"
#include "utils.h"
#include "logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#ifdef _WIN32 // Windows
#include <windows.h>
#include <psapi.h>
#include <stdint.h>

int run(int argc, char **argv) {
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

    BenchmarkRun runs[cfg.runs];

    if (cfg.help == 1) {
        print_help();
        return 0;
    }

    if (cfg.version == 1) {
        print_version();
        return 0;
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
        runs[i] = run_result;
    }
    print_progress_bar(cfg.runs, cfg.runs);
    printf("\n");
    printf("\n");

    if (valid_runs == 0) {
        fprintf(stderr, "Error: No valid runs\n\n");
        goto cleanup;
    }

    if (valid_runs < cfg.runs) {
        fprintf(stderr, ANSI_YELLOW "Warning: %d runs failed!\n\n" ANSI_RESET, (cfg.runs - valid_runs));
    } else {
        printf(ANSI_GREEN "All runs finished successfully\n\n" ANSI_RESET);
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

    if (cfg.dump_csv) {
        if (dump_csv(cfg, result, runs, valid_runs) != 0) {
            fprintf(stderr, "Error in dump_csv\n");
            goto cleanup;
        }
    }
    if (cfg.dump_json) {
        if (dump_json(cfg, result, runs, valid_runs) != 0) {
            fprintf(stderr, "Error in dump_json\n");
            goto cleanup;
        }
    }

cleanup:
    // free
    if (runs_realtime) free(runs_realtime);
    if (runs_sys_time) free(runs_sys_time);
    if (runs_user_time) free(runs_user_time);
    if (runs_max_rss) free(runs_max_rss);

    return ran == 0 ? 0 : 1;
}

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
        fprintf(stderr, "Timeout: killed target: %d after: %llums\n", pi.dwProcessId, timeout_ms);
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
            fprintf(stderr, "Timeout: killed target %d after %llums\n", pid, (unsigned long long)timeout_ms);
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