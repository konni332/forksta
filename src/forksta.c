#include <stdio.h>
#include <stdlib.h>

#include "logger.h"
#include "runner.h"
#include "metrics.h"
#include "utils.h"



#ifdef _WIN32
    #include <windows.h>
    #define SLEEP_MS(ms) Sleep(ms)
#else
    #include <unistd.h>
    #define SLEEP_MS(ms) usleep((ms) * 1000)
#endif


int main(int argc, char *argv[]) {
    return run(argc, argv);
}
