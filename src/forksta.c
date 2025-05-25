#include <stdio.h>
#include <stdlib.h>

#include "runner.h"
#include "utils.h"



#ifdef _WIN32
    #define SLEEP_MS(ms) Sleep(ms)
    #include <bemapiset.h>
#else
    #include <unistd.h>
    #define SLEEP_MS(ms) usleep((ms) * 1000)
#endif

int visualize(config_t *cfg) {
    char visual_cmd[2048];
    char forksta_path[512];
    get_executable_path(forksta_path, sizeof(forksta_path));

    char *flag = NULL;
    if (cfg->show_realtime) {
        flag = "-r";
    } else if (cfg->show_cpu_times) {
        flag = "-c";
    } else if (cfg->show_max_rss) {
        flag = "-m";
    } else if (cfg->show_all) {
        flag = "-a";
    } else {
        fprintf(stderr, "Error: cannot visualize only exit codes");
        return 1;
    }
    snprintf(visual_cmd, sizeof(visual_cmd), "python3 %s/visualize.py --mode \"%s\" --file \"%s\" %s",
        forksta_path, cfg->visual_rep, cfg->target_dump_file, flag);
    int rc = system(visual_cmd);
    if (rc != 0) {
        fprintf(stderr, "Error while running visualizer: %d\n", rc);
    }
    return rc;
}


int main(int argc, char *argv[]) {
    config_t cfg;
    init_config(&cfg);
    // run program
    parse_args(argc, argv, &cfg);
    int rc = run(&cfg);
    printf("ran with exit code %d\n\n", rc);

    if (cfg.visualize) {
        rc = visualize(&cfg);
    }
    destroy_config(&cfg);
    return rc;
}
