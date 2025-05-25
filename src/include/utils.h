
#ifndef UTILS_H
#define UTILS_H
#include <stdint.h>
#include "metrics.h"
#include "config.h"

#define DEFAULT_TIMEOUT_MS 3000
#define VERSION "1.4.0"
#define ANSI_RESET   "\x1b[0m"
#define ANSI_BOLD    "\x1b[1m"
#define ANSI_CYAN    "\x1b[36m"
#define ANSI_GREEN   "\x1b[32m"
#define ANSI_YELLOW  "\x1b[33m"
#define ANSI_RED     "\x1b[31m"




void parse_args(int argc, char **argv, config_t *cfg);
int check_target_cmd(char **target_cmd, int argc);
uint64_t seconds_to_ms(char *seconds);
uint64_t minutes_to_ms(char *minutes);
char *generate_filename(char *buffer, size_t size, const char *suffix, Benchmark *bm);
int target_exists(const char *target);
void get_executable_path(char *buffer, size_t size);
#endif //UTILS_H
