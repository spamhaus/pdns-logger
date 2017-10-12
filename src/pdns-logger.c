/*
 * Powerdns logger daemon
 * ----------------------
 *
 * This Source Code Form is subject to the terms of the
 * Mozilla Public License, v. 2.0.
 * If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (C) 2017, Spamhaus Technology Ltd, London
 *
 * The Initial developer of the Original code is:
 * Massimo Cetra
 *
 */

#include <ctype.h>
#include <signal.h>
#include "pdns-logger.h"

static globals_t globals;

/* ************************************************************************ */
/* ************************************************************************ */
/* ************************************************************************ */

static int engines_count = 0;
static pdns_logger_t engines[3];

pdns_status_t pdns_loggers_log(void *pbm) {
    int t;

    for (t = 0; t < engines_count; t++) {
        if (engines[t].log != NULL) {
            engines[t].log(pbm);
        }
    }

    return PDNS_OK;
}

static pdns_status_t pdns_loggers_rotate(void) {
    int t;

    for (t = 0; t < engines_count; t++) {
        if (engines[t].rotate != NULL) {
            engines[t].rotate();
        }
    }

    return PDNS_OK;
}

static pdns_status_t loggers_initialize(const char *conf) {
    int t;
    memset(engines, 0, sizeof(engines));

    engines[engines_count++] = logfile_engine;
    engines[engines_count++] = sqlite_engine;

    for (t = 0; t < engines_count; t++) {
        if (engines[t].start != NULL) {
            engines[t].start(conf);
        }
    }

    return PDNS_OK;
}

/* ************************************************************************ */
/* ************************************************************************ */
/* ************************************************************************ */

/*
static void signal_rotate(int sig) {
    (void) sig;
    fprintf(stderr, "Rotating logfiles...\n");
    pdns_loggers_rotate();
    return;
}
*/

static void signal_stop(int sig) {
    (void) sig;

    globals.running = 0;

    return;
}

static pdns_status_t parse_cli(globals_t * conf, int argc, char **argv) {
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
    sighandler_t sret;

    memset(&globals, 0, sizeof(globals_t));

    if (parse_cli(&globals, argc, argv) != PDNS_OK) {
        fprintf(stderr, "Error parsing cli options. Exiting.\n");
        exit(EXIT_FAILURE);
    }

    if (parse_config_file(globals.config_file, &globals) != PDNS_OK) {
        fprintf(stderr, "Error parsing config file. Exiting.\n");
        exit(EXIT_FAILURE);
    }

    if ( !globals.allow_root ) {
        uid_t current_uid = getuid();

        if (current_uid == 0) {
            fprintf(stderr, "Please don't start this program as root.\n");
            exit(EXIT_FAILURE);
        }
    }


    loggers_initialize(globals.config_file);

    sret = signal(SIGINT, signal_stop);
    assert(sret != SIG_ERR);
/*
    sret = signal(SIGHUP, SIG_IGN);
    assert(sret != SIG_ERR);
    sret = signal(SIGPIPE, SIG_IGN);
    assert(sret != SIG_ERR);
    sret = signal(SIGTERM, signal_stop);
    assert(sret != SIG_ERR);
    sret = signal(SIGHUP, signal_rotate);
    assert(sret != SIG_ERR);
*/

    globals.running = 1;

    if (pdns_socket_run(&globals) != PDNS_OK) {
        fprintf(stderr, "Cannot start the socket process. Is there another daemon already listening ?\n");
        exit(EXIT_FAILURE);
    }

    fprintf(stderr, "exiting\n");

    safe_free(globals.user);
    safe_free(globals.group);
    safe_free(globals.config_file);

    return 0;
}
