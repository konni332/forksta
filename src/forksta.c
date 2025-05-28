#include <stdio.h>
#include <stdlib.h>

#include "logger.h"
#include "runner.h"
#include "ui.h"
#include "utils.h"



#ifdef _WIN32
    #define SLEEP_MS(ms) Sleep(ms)
    #include <bemapiset.h>
#else
    #include <unistd.h>
    #define SLEEP_MS(ms) usleep((ms) * 1000)
#endif

int visualize(config_t *cfg) {
    printf("\n\n");
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
        fprintf(stderr, "\n\n");
        fprintf(stderr, "Error while running visualizer: %d\n", rc);
        fprintf(stderr, "Command: %s\n", visual_cmd);
        fprintf(stderr, "Possibly missing dependencies! Try \"forksta --dependencies\" for details");
        fprintf(stderr, "\n");
    }
    if (!cfg->comparison || !cfg->comparison_cmd || strlen(cfg->comparison_dump_file) < 1) return rc;

    snprintf(visual_cmd, sizeof(visual_cmd), "python3 %s/visualize.py --mode \"%s\" --file \"%s\" %s",
        forksta_path, cfg->visual_rep, cfg->comparison_dump_file, flag);
    rc = system(visual_cmd);
    printf("\n\n");
    if (rc != 0) {
        fprintf(stderr, "\n\n");
        fprintf(stderr, "Error while running visualizer: %d\n", rc);
        fprintf(stderr, "Command: %s\n", visual_cmd);
        fprintf(stderr, "Possibly missing dependencies! Try \"forksta --dependencies\" for details");
        fprintf(stderr, "\n");
    }
    printf("\n\n");
    return rc;
}

int main(int argc, char *argv[]) {
    config_t cfg;
    init_config(&cfg);
    // run program
    parse_args(argc, argv, &cfg);
    int rc = run(&cfg);

    if (cfg.visualize) {
        rc = visualize(&cfg);
    }
    if (cfg.visualize && cfg.clean_dump) {
        if (clean_dump_file(cfg.target_dump_file) != 0) {
            fprintf(stderr, "Error cleaning dump file\n");
            rc = 1;
        }
        if (clean_dump_file(cfg.comparison_dump_file) != 0) {
            fprintf(stderr, "Error cleaning dump file\n");
            rc = 1;
        }
    }
    destroy_config(&cfg);
    return rc;
}
