
#include <ctype.h>
#include <signal.h>
#include "pdns-logger.h"

static int stopped = 0;
static globals_t globals;

static void signal_stop(int sig) {
    (void) sig;

    stopped++;
    globals.running = 0;

    return;
}

static pdns_status_t parse_cli(globals_t *conf, int argc, char **argv) {
    int c;

    while ((c = getopt(argc, argv, "hvc:f:")) != -1) {
        switch (c) {
            case 'h':
                printf("\n");
                printf("Usage: pdns-collector [options]\n");
                printf("Options:\n");
                printf("  -c configfile       Location of the config file (default: %s)\n", DEFAULT_CONFIG_FILE);
                printf("  -f                  Do not fork and stay in foreground\n");
                printf("  -h                  Print this message and exit.\n");
                printf("  -v                  Verbose mode, display config options and stats\n");
                printf("\n");
                return PDNS_OK;
                break;
            case 'c':
                conf->config_file = strdup(optarg);
                if (conf->verbose) {
                    printf("Options file set to %s\n", conf->config_file);
                }
                break;
            case 'f':
                conf->foreground = 1;
                break;

            case 'v':
                conf->verbose = 1;
                break;

            case '?':
                if (optopt == 'c') {
                    printf("Option -%c requires an argument.\n", optopt);
                    return PDNS_NO;
                } else if (isprint(optopt)) {
                    printf("Unknown option `-%c'.\n", optopt);
                    return PDNS_NO;
                } else {
                    printf("Unknown option character `\\x%x'.\n", optopt);
                    return PDNS_NO;
                }
                break;

            default:
                printf("Error parsing command line options\n");
                return PDNS_NO;
        }
    }

    return PDNS_OK;
}

int main(int argc, char **argv) {

    memset(&globals, 0, sizeof(globals_t));

    if ( parse_cli(&globals, argc, argv) != PDNS_OK ) {
        fprintf(stderr, "Error parsing cli options. Exiting.\n");
        return 1;
    }

    if ( parse_config_file(globals.config_file, &globals) != PDNS_OK ) {
        fprintf(stderr, "Error parsing config file. Exiting.\n");
        return 1;
    }

    signal(SIGPIPE, SIG_IGN);

    signal(SIGINT, signal_stop);
    signal(SIGTERM, signal_stop);
    signal(SIGKILL, signal_stop);

    globals.running = 1;
    if ( pdns_socket_run(&globals) != PDNS_OK ) {
        assert(0); // TODO LOG
    }

    safe_free(globals.user);
    safe_free(globals.group);
    safe_free(globals.config_file);

    return 0;
}
