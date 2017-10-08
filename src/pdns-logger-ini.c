
#include "pdns-logger.h"
#include "inih/ini.h"

static int opt_handler(void *user, const char *section, const char *name, const char *value, int lineno) {
    globals_t *globals = user;

    if ( zstr(section) || zstr(name) || zstr(value) ) {
        return 1;
    }

    if ( !strncmp(section, "globals", sizeof("globals")) ) {
        if ( !strncmp(name, "user", sizeof("user")) ) {
            globals->user = strdup(value);
        }
        else if ( !strncmp(name, "group", sizeof("group")) ) {
            globals->group = strdup(value);
        }
        else if ( !strncmp(name, "bind-ip", sizeof("bind-ip")) ) {
            globals->bind_ip = strdup(value);
        }
        else if ( !strncmp(name, "bind-port", sizeof("bind-port")) ) {
            globals->bind_port = atoi(value);
        }
        else {
            fprintf(stderr, "Unmanaged INI option '%s'\n", name);
        }

        return 1;
    }

    fprintf(stderr, "%d * %s -> %s -> %s\n", lineno, section, name, value);

    return 1;
}

pdns_status_t parse_config_file(const char *cf, globals_t *globals) {

    if ( zstr(cf) ) {
        return PDNS_NO;
    }

    if (ini_parse(cf, opt_handler, globals) != 0) {
        fprintf(stderr, "Can't read options file: '%s'\n", cf);
        return PDNS_NO;
    }

    return PDNS_OK;
}
