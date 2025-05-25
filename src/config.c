//
// Created by doepp on 25.05.2025.
//
#include "config.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils.h"

#ifdef _WIN32
#include <windows.h>
    void get_config_path(char *path, size_t size) {
        if (!path || size < 1) return;
        get_executable_path(path, size);
        strcat(path, "/forksta.conf");
    }
#elif __linux__
#include <sys/stat.h>
#include <unistd.h>
char* expand_tilde(const char* path) {
    if (path[0] != '~') {
        return strdup(path);
    }

    const char* home = getenv("HOME");
    if (!home) {
        return NULL;
    }

    size_t len = strlen(home) + strlen(path);
    char* fullpath = malloc(len);
    if (!fullpath) {
        return NULL;
    }

    sprintf(fullpath, "%s%s", home, path + 1);
    return fullpath;
}

int dir_exists(const char *path) {
        struct stat st;
        return stat(path, &st) == 0 && S_ISDIR(st.st_mode);
    }

void get_config_path(char *path, size_t size) {
    if (!path || size < 1) return;
    char *dir = expand_tilde("~/.config/forksta");
    if (!dir_exists(dir)) mkdir(dir, 0755);
    free(dir);

    char *fullpath = expand_tilde("~/.config/forksta/forksta.conf")
    snprintf(path, size, "%s", fullpath);
    free(fullpath);
}

#elif __APPLE__
char* expand_tilde(const char* path) {
    if (path[0] != '~') {
        return strdup(path);
    }

    const char* home = getenv("HOME");
    if (!home) {
        return NULL;
    }

    size_t len = strlen(home) + strlen(path); // +1 fehlt nicht, da '/' schon in Pfad enthalten ist
    char* fullpath = malloc(len); // besser: +1 für null terminator
    if (!fullpath) {
        return NULL;
    }

    sprintf(fullpath, "%s%s", home, path + 1); // path+1: überspringt das '~'
    return fullpath;
}

int dir_exists(const char *path) {
    struct stat st;
    return stat(path, &st) == 0 && S_ISDIR(st.st_mode);
}

void get_config_path(char *path, size_t size) {
    if (!path || size < 1) return;

    char *dir = expand_tilde("~/Library/Application Support/forksta");
    if (dir == NULL) {
        fprintf(stderr, "Error expanding tilde for config directory\n");
        exit(1);
    }

    if (!dir_exists(dir)) {
        if (mkdir(dir, 0755) != 0) {
            perror("mkdir failed");
            free(dir);
            exit(1);
        }
    }

    char *fullpath = expand_tilde("~/Library/Application Support/forksta/forksta.conf");
    if (!fullpath) {
        fprintf(stderr, "Error expanding tilde for config file path\n");
        free(dir);
        exit(1);
    }

    snprintf(path, size, "%s", fullpath);

    free(dir);
    free(fullpath);
}

#else
    #error "Unsuported OS"
#endif
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



int config_exists(const char *config_path) {
    FILE *fp = fopen(config_path, "r");
    if (fp == NULL) {
        return 0;
    }
    fclose(fp);
    return 1;
}

void init_config_ini(const char *config_path) {
    if (!config_path) return;
    FILE *fp = fopen(config_path,"w");
    if (!fp) {
        fprintf(stderr, "Error creating config file\n");
        exit(1);
    }
    fprintf(fp, "[DEFAULT]\nruns=1\nwarmup=5\noutput=REAL_TIME\nvisual=TABLE\n");
    fflush(fp);
    fclose(fp);
}

void trim(char *str) {
    if (!str) return;

    char *start = str;
    while (isspace((unsigned char)*start)) start++;

    char *end = start + strlen(start) - 1;
    while (end > start && isspace((unsigned char)*end)) end--;
    *(end + 1) = '\0';

    memmove(str, start, end - start + 2);  // inklusive Nullterminator
}


void parse_config_ini(config_t *cfg) {
    char ini_path[1024];
    get_config_path(ini_path, sizeof(ini_path));
    int exists = config_exists(ini_path);
    if (!exists) {
        init_config_ini(ini_path);
    }
    FILE *fp = fopen(ini_path, "r");
    if (fp == NULL) {
        fprintf(stderr, "Error opening config file\n");
        exit(1);
    }

    char line[1024];
    char section[64] = "";
    while (fgets(line, sizeof(line), fp)) {
        trim(line);
        if (line[0] == '\0' || line[0] == '#' || line[0] == ';') continue;

        if (line[0] == '[') {
            sscanf(line, "[%63[^]]", section);
        } else {
            char key[64], value[128];
            if (sscanf(line, "%63[^=]=%127[^\n]", key, value) == 2) {
                trim(key);
                trim(value);
                if (strcmp(key, "runs") == 0) {
                    cfg->runs = atoi(value);
                    if (cfg->runs < 1) {
                        fprintf(stderr, "Invalid runs value in .conf: %s\n", value);
                        cfg->runs = 1;
                    }
                } else if (strcmp(key, "warmup") == 0) {
                    cfg->warmup_runs = atoi(value);
                    if (cfg->warmup_runs < 1) {
                        fprintf(stderr, "Invalid warmup value in .conf: %s\n", value);
                        cfg->warmup_runs = 1;
                    }
                } else if (strcmp(key, "output") == 0) {
                    if (strcmp(value, "REAL_TIME") == 0) {
                        cfg->show_realtime = 1;
                        cfg->show_cpu_times = 0;
                        cfg->show_max_rss = 0;
                        cfg->show_all = 0;
                    } else if (strcmp(value, "CPU_TIMES") == 0) {
                        cfg->show_realtime = 0;
                        cfg->show_cpu_times = 1;
                        cfg->show_max_rss = 0;
                        cfg->show_all = 0;
                    } else if (strcmp(value, "MAX_RSS") == 0) {
                        cfg->show_realtime = 0;
                        cfg->show_cpu_times = 0;
                        cfg->show_max_rss = 1;
                        cfg->show_all = 0;
                    } else if (strcmp(value, "ALL") == 0) {
                        cfg->show_realtime = 0;
                        cfg->show_cpu_times = 0;
                        cfg->show_max_rss = 0;
                        cfg->show_all = 1;
                    } else {
                        fprintf(stderr, "Invalid output value in .conf: %s\n", value);
                        exit(1);
                    }
                } else if (strcmp(key, "visual") == 0) {
                  if (strcmp(value, "TABLE") == 0) {
                      cfg->visual_rep = "table";
                  } else if (strcmp(value, "PLOT") == 0) {
                      cfg->visual_rep = "plot";
                  } else if (strcmp(value, "HEAT_MAP") == 0) {
                      cfg->visual_rep = "heatmap";
                  } else if (strcmp(value, "LIST") == 0) {
                      cfg->visual_rep = "list";
                  } else {
                      fprintf(stderr, "Invalid visual value in .conf: %s\n", value);
                      exit(1);
                  }
                } else {
                    fprintf(stderr, "Unknown key in .conf: %s\n", key);
                    exit(1);
                }
            }
        }
    }

    fclose(fp);
}

void init_config(config_t *cfg) {
    if (!cfg) return;
    // Default Werte
    cfg->help = 0;
    cfg->version = 0;
    cfg->dependencies = 0;
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
    parse_config_ini(cfg);
}
