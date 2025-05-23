#include "runner.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


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