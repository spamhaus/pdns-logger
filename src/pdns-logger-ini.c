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

#include "pdns-logger.h"
#include "inih/ini.h"

static int opt_handler(void *user, const char *section, const char *name, const char *value, int lineno) {
    globals_t *globals = user;

    if (zstr(section) || zstr(name) || zstr(value)) {
        return 1;
    }

    if (!strncmp(section, "globals", sizeof("globals"))) {
        if (!strncmp(name, "bind-ip", sizeof("bind-ip"))) {
            globals->bind_ip = strdup(value);
        } else if (!strncmp(name, "bind-port", sizeof("bind-port"))) {
            globals->bind_port = atoi(value);
        } else if (!strncmp(name, "allow-root", sizeof("allow-root"))) {
            globals->allow_root = atoi(value) ? 1 : 0;
        } else if (!strncmp(name, "foreground", sizeof("foreground"))) {
            globals->foreground = atoi(value) ? 1 : 0;
        /*
        } else if (!strncmp(name, "user", sizeof("user"))) {
            globals->user = strdup(value);
        } else if (!strncmp(name, "group", sizeof("group"))) {
            globals->group = strdup(value);
        */
        } else {
            fprintf(stderr, "Unmanaged INI option '%s' at line %d\n", name, lineno);
        }

        return 1;
    }

    return 1;
}

pdns_status_t parse_config_file(const char *cf, globals_t * globals) {

    if (zstr(cf)) {
        return PDNS_NO;
    }

    if (ini_parse(cf, opt_handler, globals) != 0) {
        fprintf(stderr, "Can't read options file: '%s'\n", cf);
        return PDNS_NO;
    }

    return PDNS_OK;
}
