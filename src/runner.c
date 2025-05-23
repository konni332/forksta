//
// Created by doepp on 23.05.2025.
//
#include "runner.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#ifdef _WIN32 // Windows
#include <windows.h>

int run_target(char **argv, BenchmarkResult *result) {
    if (!argv || !result) {
        fprintf(stderr, "Invalid arguments in run_target\n");
        return -1;
    }

    LARGE_INTEGER freq, start, end;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&start);

    // build command line
    char cmdln[1024] = {0};
    for (int i = 0; argv[i]; ++i) {
        strcat(cmdln, argv[i]);
        strcat(cmdln, " ");
    }

    STARTUPINFO si = { sizeof(si) };
    PROCESS_INFORMATION pi;

    if (!CreateProcess(NULL, cmdln, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        perror("CreateProcess failed");
        return -1;
    }

    WaitForSingleObject(pi.hProcess, INFINITE);
    DWORD exit_code;
    GetExitCodeProcess(pi.hProcess, &exit_code);

    QueryPerformanceCounter(&end);

    result->real_time = (double) (end.QuadPart - start.QuadPart) / (double) freq.QuadPart;
    result->exit_code = (int)exit_code;

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    return 0;
}

#else // POSIX (macOS/Linux)

#include <unistd.h>
#include <sys/wait.h>
#include <time.h>

int run_target(char **argv, BenchmarkResult *result) {
    if (!argv || !result) {
        fprintf(stderr, "Invalid arguments in run_target\n");
        return -1;
    }

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    pid_t pid = fork();
    if (pid == 0) {
        execvp(argv[0], argv);
        perror("execvp failed");
        exit(1);
    }

    int status;
    waitpid(pid, &status, 0);

    clock_gettime(CLOCK_MONOTONIC, &end);

    result->real_time = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    result->exit_code = WIFEXITED(status) ? WEXITSTATUS(status) : -1;

    return 0;
}

#endif