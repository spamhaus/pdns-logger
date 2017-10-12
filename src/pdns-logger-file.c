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

#include "inih/ini.h"
#include "pdns-logger.h"
#include "dnsmessage.pb-c.h"

static FILE *fp = NULL;
static char *file = NULL;
static int force_flush = 0;
static char rewrites_only = 1;
/* static fifo_t *queue = NULL; */

static int opt_handler(void *user, const char *section, const char *name, const char *value, int lineno) {
    (void) user;

    if (zstr(section) || zstr(name) || zstr(value)) {
        return 1;
    }

    if (!strncmp(section, "logfile", sizeof("logfile"))) {
        if (!strncmp(name, "logfile", sizeof("logfile"))) {
            file = strdup(value);
        } else if (!strncmp(name, "force-flush", sizeof("force-flush"))) {
            force_flush = atoi(value) ? 1 : 0;
        } else if (!strncmp(name, "only-rewrites", sizeof("only-rewrites"))) {
            rewrites_only = atoi(value) ? 1 : 0;
        } else {
            fprintf(stderr, "Unmanaged INI option '%s' at line %d\n", name, lineno);
        }
        return 1;
    }

    return 1;
}

static pdns_status_t logfile_init(const char *inifile) {
    if (zstr(inifile)) {
        return PDNS_NO;
    }

    if (ini_parse(inifile, opt_handler, NULL) != 0) {
        fprintf(stderr, "logfile: Can't read .ini file: '%s'\n", inifile);
        return PDNS_NO;
    }

    if (zstr(file)) {
        fprintf(stderr, "logfile: no log file set. Disabling.\n");
        return PDNS_NO;
    }

    /*
       queue = fifo_init();
       if ( queue == NULL ) {
       return PDNS_NO;
       }
     */

    fp = fopen(file, "a");
    if (fp == NULL) {
        fprintf(stderr, "logfile: cannot open '%s' for writing\n", file);
        return PDNS_NO;
    }

    return PDNS_OK;
}

static pdns_status_t logfile_rotate(void) {
    if (fp != NULL) {
        fp = freopen(file, "a", fp);
        if (fp == NULL) {
            fprintf(stderr, "logfile: cannot open '%s' for writing\n", file);
            return PDNS_NO;
        }
    }

    return PDNS_OK;
}

static pdns_status_t logfile_stop(void) {
    safe_free(file);

    if (fp != NULL) {
        fclose(fp);
    }

    return PDNS_OK;
}

#define write_log() \
    if (fp != NULL) { \
        fprintf(fp, "%s\n", str); \
        if (force_flush) { \
            fflush(fp); \
        } \
    } \
    fprintf(stderr, "%s\n", str);

static pdns_status_t logfile_log(void *rawpb) {
    PBDNSMessage *msg = rawpb;
    PBDNSMessage__DNSQuestion *q;
    PBDNSMessage__DNSResponse *r;
    int sz, pc;
    char str[1024] = "";
    char tmp[1024] = "";

    if (msg == NULL || msg->response == NULL) {
        return PDNS_OK;
    }

    if (rewrites_only != 0) {
        if (msg->response != NULL && zstr(msg->response->appliedpolicy)) {
            return PDNS_OK;
        }
    }

    sz = sizeof(str) - 1;

    if (msg->has_id) {
        pc = snprintf(tmp, sizeof(tmp), "QID: %d ", msg->id);
        strncat(str, tmp, sz);
        sz -= pc;
    }

    if (msg->has_from) {
        if (msg->from.len == 4) {
            char ip[INET6_ADDRSTRLEN];

            inet_ntop(AF_INET, (const void *) msg->from.data, ip, sizeof(ip));

            pc = snprintf(tmp, sizeof(tmp), "from: %s ", ip);
            strncat(str, tmp, sz);
            sz -= pc;
        } else if (msg->from.len == 16) {
            char ip[INET6_ADDRSTRLEN];

            inet_ntop(AF_INET6, (const void *) msg->from.data, ip, sizeof(ip));

            pc = snprintf(tmp, sizeof(tmp), "from: %s ", ip);
            strncat(str, tmp, sz);
            sz -= pc;
        }
    }

    if (msg->has_originalrequestorsubnet) {
        assert(0);
    }

    q = msg->question;
    if (q != NULL) {
        if (q->has_qtype) {
            pc = snprintf(tmp, sizeof(tmp), "qtype: %s ", pdns_logger_type2p(q->qtype));
            strncat(str, tmp, sz);
            sz -= pc;
        }

        if (q->has_qclass) {
            pc = snprintf(tmp, sizeof(tmp), "qclass: %s ", pdns_logger_class2p(q->qclass));
            strncat(str, tmp, sz);
            sz -= pc;
        }

        pc = snprintf(tmp, sizeof(tmp), "qname: %s ", q->qname);
        strncat(str, tmp, sz);
        sz -= pc;
    }

    r = msg->response;
    if (r != NULL) {
        if (r->has_rcode) {
            pc = snprintf(tmp, sizeof(tmp), "rcode: %s ", pdns_logger_rcode2p(r->rcode));
            strncat(str, tmp, sz);
            sz -= pc;
        }

        pc = snprintf(tmp, sizeof(tmp), "rrcount: %zu ", r->n_rrs);
        strncat(str, tmp, sz);
        sz -= pc;

        if (!zstr(r->appliedpolicy)) {
            pc = snprintf(tmp, sizeof(tmp), "policy: '%s' ", r->appliedpolicy);
            strncat(str, tmp, sz);
            sz -= pc;
        }

        if (r->n_rrs > 0) {
            unsigned int t;
            PBDNSMessage__DNSResponse__DNSRR *rr;

            for (t = 1; t <= r->n_rrs; t++) {
                rr = r->rrs[t - 1];

                pc = snprintf(tmp, sizeof(tmp), "rname-%d: %s ", t, rr->name);
                strncat(str, tmp, sz);
                sz -= pc;

                if (rr->has_type) {
                    pc = snprintf(tmp, sizeof(tmp), "rtype-%d: %s ", t, pdns_logger_type2p(rr->type));
                    strncat(str, tmp, sz);
                    sz -= pc;
                }

                if (rr->has_class_) {
                    pc = snprintf(tmp, sizeof(tmp), "rclass-%d: %s ", t, pdns_logger_class2p(rr->class_));
                    strncat(str, tmp, sz);
                    sz -= pc;
                }

                if (rr->has_ttl) {
                    pc = snprintf(tmp, sizeof(tmp), "rttl-%d: %d ", t, rr->ttl);
                    strncat(str, tmp, sz);
                    sz -= pc;
                }

                if (rr->has_rdata) {
                    if (rr->has_type && rr->type == 1 && rr->rdata.len == 4) {
                        char ip[INET6_ADDRSTRLEN];

                        inet_ntop(AF_INET, (const void *) rr->rdata.data, ip, sizeof(ip));

                        pc = snprintf(tmp, sizeof(tmp), "rdata-%d: %s ", t, ip);
                        strncat(str, tmp, sz);
                        sz -= pc;
                    } else if (rr->has_type && rr->type == 28 && rr->rdata.len == 16) {
                        char ip[INET6_ADDRSTRLEN];

                        inet_ntop(AF_INET6, (const void *) rr->rdata.data, ip, sizeof(ip));

                        pc = snprintf(tmp, sizeof(tmp), "rdata-%d: %s ", t, ip);
                        strncat(str, tmp, sz);
                        sz -= pc;
                    } else if (rr->has_type && ((rr->type == 2) || (rr->type == 5) || (rr->type == 6) || (rr->type == 15))) {
                        /* CNAME works */
                        /* NS SOA MX do not seem to pass any data */
                        pc = snprintf(tmp, sizeof(tmp), "rdata-%d: %s ", t, rr->rdata.data);
                        strncat(str, tmp, sz);
                        sz -= pc;
                    } else {
                        pc = snprintf(tmp, sizeof(tmp), "rdata (not supported) ");
                        strncat(str, tmp, sz);
                        sz -= pc;
                    }
                }

                write_log();
            }
        } else {
            write_log();
        }
    } else {
        write_log();
    }

    return PDNS_OK;
}

pdns_logger_t logfile_engine = {
    logfile_init,
    logfile_rotate,
    logfile_stop,
    logfile_log
};
