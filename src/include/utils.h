
#ifndef UTILS_H
#define UTILS_H
#include <stdint.h>
#include "metrics.h"
#include "config.h"

#define DEFAULT_TIMEOUT_MS 3000
#define VERSION "1.0.0"
#define ANSI_RESET   "\x1b[0m"
#define ANSI_BOLD    "\x1b[1m"
#define ANSI_CYAN    "\x1b[36m"
#define ANSI_GREEN   "\x1b[32m"
#define ANSI_YELLOW  "\x1b[33m"
#define ANSI_RED     "\x1b[31m"


/**
 * @brief parses forksta arguments and save those into configurations
 * @param argc number of arguments
 * @param argv program arguments
 * @param cfg configurations
 */
void parse_args(int argc, char **argv, config_t *cfg);
/**
 * @brief checks target cmd for NULL termination
 * @param target_cmd target command line
 * @param argc number of arguments
 * @return
 */
int check_target_cmd(char **target_cmd, int argc);
/**
 *
 * @param seconds seconds to be converted
 * @return ms
 */
uint64_t seconds_to_ms(char *seconds);
/**
 *
 * @param minutes minutes to be converted
 * @return ms
 */
uint64_t minutes_to_ms(char *minutes);
/**
 *
 * @param buffer buffer
 * @param size buffer size
 * @param suffix file suffix: .json, .csv
 * @param bm Benchmark struct
 * @return pointer to buffer
 */
char *generate_filename(char *buffer, size_t size, const char *suffix, Benchmark *bm);
/**
 * @brief checks weather target program exists
 * @param target target name
 * @return 1 if it exists else 0
 */
int target_exists(const char *target);
/**
 * @brief saves path to executable into buffer
 * @param buffer buffer
 * @param size buffer size
 */
void get_executable_path(char *buffer, size_t size);
#endif //UTILS_H
