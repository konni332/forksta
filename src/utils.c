#include <stdlib.h>
#include "utils.h"
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

char* portable_strndup(const char* s, size_t n) {
    if (!s) return NULL;

    size_t len = strnlen(s, n);
    char* result = (char*)malloc(len + 1);
    if (!result) return NULL;

    memcpy(result, s, len);
    result[len] = '\0';

    return result;
}

char** cpy_cmd_line(char** cmd, int args_count) {
    char** cmd_line = (char**)malloc(sizeof(char*) * (args_count + 1));
    if (cmd_line == NULL) {
        fprintf(stderr, "Error in cpy_cmd_line\n");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < args_count; i++) {
        cmd_line[i] = portable_strndup(cmd[i], strlen(cmd[i]));
        if (!cmd_line[i]) {
            for (int j = 0; j < i; j++) free(cmd_line[j]);
            free(cmd_line);
            fprintf(stderr, "Memory allocation failed in cpy_cmd_line\n");
            exit(EXIT_FAILURE);
        }
    }

    cmd_line[args_count] = NULL; // NULL-terminieren
    return cmd_line;
}

int check_target_cmd(char **target_cmd, int argc) {
    if (target_cmd == NULL || target_cmd[0] == NULL) {
        return -1;
    }

    int j = 0;
    while (target_cmd[j] != NULL) {
        j++;
        if (j > (argc)) {
            fprintf(stderr, "Error: target_cmd array not NULL-terminated\n");
            return -1;
        }
    }
    return 0;
}

void parse_comparison(int start, int argc, char **argv, config_t *cfg) {
    int with_index = -1;

    // Suche nach --with & ersetze -py
    for (int i = start; i < argc; i++) {
        if (strcmp(argv[i], "--with") == 0) {
            if (with_index == -1) {
                with_index = i;
            } else {
                fprintf(stderr, "Invalid usage: only one --with allowed\n");
                exit(1);
            }
        }
        if (strcmp(argv[i], "-py") == 0) {
            argv[i] = "python3";
        }
    }

    if (with_index == -1) {
        fprintf(stderr, "Missing --with for comparison\n");
        exit(1);
    }
    if (with_index + 1 >= argc) {
        fprintf(stderr, "No comparison target after --with\n");
        exit(1);
    }

    // Erstes Programm (target)
    if (strcmp(argv[start], "python3") == 0) {
        cfg->target = portable_strndup(argv[start + 1], strlen(argv[start + 1]));
    }
    else {
        cfg->target = portable_strndup(argv[start], strlen(argv[start]));
    }
    cfg->target_cmd = cpy_cmd_line(&argv[start], with_index - start);
    cfg->target_args_count = with_index - start;

    // Zweites Programm (comparison)
    if (strcmp(argv[with_index + 1], "python3") == 0) {
        cfg->comparison = portable_strndup(argv[with_index + 2], strlen(argv[with_index + 2]));
    }
    else {
        cfg->comparison = portable_strndup(argv[with_index + 1], strlen(argv[with_index + 1]));
    }
    cfg->comparison_cmd = cpy_cmd_line(&argv[with_index + 1], argc - (with_index + 1));
    cfg->comparison_args_count = argc - (with_index + 1);
}

void parse_args(int argc, char **argv, config_t *cfg) {
    if (argc < 2) {
        fprintf(stderr, "Missing target\n");
        exit(1);
    }

    int i = 1;
    // Optionen parsen
    for (; i < argc; i++) {
        if (argv[i][0] != '-') {
            // break when a no flag item is found!
            break;
        }

        // Flags auswerten
        if (strcmp(argv[i], "-r") == 0) {
            cfg->show_realtime = 1;
            cfg->show_all = 0;
        } else if (strcmp(argv[i], "-c") == 0) {
            cfg->show_cpu_times = 1;
            cfg->show_all = 0;
        } else if (strcmp(argv[i], "-m") == 0) {
            cfg->show_max_rss = 1;
            cfg->show_all = 0;
        } else if (strcmp(argv[i], "-e") == 0) {
            cfg->show_exit_code = 1;
            cfg->show_all = 0;
        } else if (strcmp(argv[i], "-a") == 0) {
            cfg->show_all = 1;
            cfg->show_realtime = 0;
            cfg->show_cpu_times = 0;
            cfg->show_max_rss = 0;
            cfg->show_exit_code = 0;
        }

        // --dump
        else if (strncmp(argv[i], "--dump", 6) == 0) {
            if (i + 1 < argc) {
                if (strcmp(argv[i + 1], "csv") == 0) {
                    cfg->dump_csv = 1;
                    i++;
                } else if (strcmp(argv[i + 1], "json") == 0) {
                    cfg->dump_json = 1;
                    i++;
                } else {
                    fprintf(stderr, "Invalid dump format: %s\n", argv[i + 1]);
                    exit(1);
                }
            } else {
                fprintf(stderr, "Missing dump format\n");
                exit(1);
            }
        }

        // --runs
        else if (strcmp(argv[i], "--runs") == 0) {
            if (i + 1 < argc) {
                cfg->runs = atoi(argv[i + 1]);
                if (cfg->runs < 1) cfg->runs = 1;
                i++;
            } else {
                fprintf(stderr, "Missing number of runs\n");
                exit(1);
            }
        }

        // --timeout
        else if (strcmp(argv[i], "--timeout") == 0) {
            if (i + 1 < argc) {
                cfg->timeout_ms = seconds_to_ms(argv[i + 1]);
                i++;
            } else {
                fprintf(stderr, "Missing timeout\n");
                exit(1);
            }
        }

        // --timeout-m
        else if (strcmp(argv[i], "--timeout-m") == 0) {
            if (i + 1 < argc) {
                cfg->timeout_ms = minutes_to_ms(argv[i + 1]);
                i++;
            } else {
                fprintf(stderr, "Missing timeout\n");
                exit(1);
            }
        }

        // --help
        else if (strcmp(argv[i], "--help") == 0) {
            cfg->help = 1;
            break;
        }

        // --version
        else if (strcmp(argv[i], "--version") == 0) {
            cfg->version = 1;
            break;
        }

        // -py
        else if (strcmp(argv[i], "-py") == 0) {
            if (i + 1 >= argc || strstr(argv[i + 1], ".py") == NULL) {
                fprintf(stderr, "Missing python file\n");
                exit(1);
            }
            argv[i] = "python3";
            cfg->target = portable_strndup(argv[i + 1], strlen(argv[i + 1])); // Programm
            cfg->target_cmd = cpy_cmd_line(&argv[i], argc - i); // Programm with arguments
            cfg->target_args_count = argc - i;
            return;
        }

        // --compare
        else if (strcmp(argv[i], "--compare") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "Missing comparison\n");
                exit(1);
            }
            return parse_comparison(i + 1, argc, argv, cfg);
        }
        // --visual
        else if (strcmp(argv[i], "--visual") == 0) {
            cfg->visualize = 1;
            cfg->dump_json = 1;
            if (i + 1 >= argc || argv[i + 1][0] == '-') {
                continue;
            }
            cfg->visual_rep = argv[i + 1];
            i++;
        }
        else {
            fprintf(stderr, "Unknown option: %s\n", argv[i]);
            exit(1);
        }
    }

    if (i >= argc) {
        fprintf(stderr, "Missing target program\n");
        exit(1);
    }
    if (strcmp(argv[i], "python3") == 0) {
        cfg->target = portable_strndup(argv[i + 1], strlen(argv[i + 1]));
    }
    else {
        cfg->target = portable_strndup(argv[i], strlen(argv[i]));
    } // Programm
    cfg->target_cmd = cpy_cmd_line(&argv[i], argc - i); // Programm with arguments
    cfg->target_args_count = argc - i;
}


uint64_t seconds_to_ms(char *seconds) {
    if (!seconds) {
        fprintf(stderr, "Invalid seconds value: %s\n", seconds);
        exit(1);
    }
    char *endptr;
    const uint64_t val = strtol(seconds, &endptr, 10);
    if (*endptr != '\0') {
        fprintf(stderr, "Invalid seconds value: %s\n", seconds);
        exit(1);
    }
    return val * 1000;
}

uint64_t minutes_to_ms(char *minutes) {
    if (!minutes) {
        fprintf(stderr, "Invalid minutes value: %s\n", minutes);
        exit(1);
    }
    char *endptr;
    const uint64_t val = strtol(minutes, &endptr, 10);
    if (*endptr != '\0') {
        fprintf(stderr, "Invalid minutes value: %s\n", minutes);
        exit(1);
    }
    return val * 60 * 1000;
}

void seed_rng_once() {
    static int seeded = 0;
    if (!seeded) {
        srand(time(NULL));
        seeded = 1;
    }
}

char *generate_filename(char *buffer, size_t size, const char *suffix, Benchmark *bm) {
    seed_rng_once();
    if (!buffer || !bm) {
        fprintf(stderr, "Invalid arguments\n");
        exit(1);
    }
    if (size < 1) {
        fprintf(stderr, "Invalid size\n");
        exit(1);
    }
    if (!suffix) {
        fprintf(stderr, "Invalid suffix\n");
        exit(1);
    }
    int rand_num = rand() % 0xFFFF;

    snprintf(buffer, size, "benchmark_0x%03x%s", rand_num, suffix);
    return buffer;
}

int target_exists(const char *target) {
    FILE *fp = fopen(target, "r");
    if (fp == NULL) {
        return 0;
    }
    fclose(fp);
    return 1;
}

#ifdef _WIN32
#include <windows.h>
void get_executable_path(char *buffer, size_t size) {
    char fullpath[MAX_PATH];
    GetModuleFileNameA(NULL, fullpath, MAX_PATH);
    char *p = strrchr(fullpath, '\\');
    if (p) {
        *p = '\0';
    }
    strncpy(buffer, fullpath, size - 1);
    buffer[size - 1] = '\0';
}
#elif __linux__
#include <unistd.h>
#include <limits.h>
#include <libgen.h>
void get_executable_path(char *buffer, size_t size) {
    char path[PATH_MAX];
    size_t len = readlink("/proc/self/exe", path, sizeof(path) - 1);
    if (len != -1) {
        path[len - 1] = '\0';
        char *dir = dirname(path);
        strncpy(buffer, dir, size - 1);
        buffer[size - 1] = '\0';
    } else {
        fprintf(stderr, "Path to forksta executable could not be found");
        buffer[0] = '\0';
    }
}
#elif __APPLE__
#include <mach-o/dyld.h>
void get_executable_path(char *buffer, size_t size) {
    char path[PATH_MAX];
    uint32_t len = sizeof(path);
    if (_NSGetExecutablePath(path, &len) == 0) {
        char *dir = dirname(path);
        strncpy(buffer, dir, size - 1);
        buffer[size - 1] = '\0';
    } else {
        fprintf(stderr, "Path to long for buffer");
        buffer[0] = '\0';
    }
}
#else
    #error "Unsupported platform"
#endif
