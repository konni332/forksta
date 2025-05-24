cmake_minimum_required(VERSION 3.31)
project(forksta C)

set(CMAKE_C_STANDARD 11)

add_executable(forksta src/forksta.c src/logger.c src/metrics.c src/runner.c src/utils.c src/ui.c)

target_include_directories(forksta PRIVATE src/include/)
