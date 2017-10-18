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

#include <syslog.h>
#include "inih/ini.h"
#include "pdns-logger.h"
#include "dnsmessage.pb-c.h"

static struct {
    int facility;
    const char *name;
} facility_names[] = {
    {
    LOG_AUTH, "auth"}, {
    LOG_AUTHPRIV, "authpriv"}, {
    LOG_CRON, "cron"}, {
    LOG_DAEMON, "daemon"}, {
    LOG_FTP, "ftp"}, {
    LOG_KERN, "kern"}, {
    LOG_LOCAL0, "local0"}, {
    LOG_LOCAL1, "local1"}, {
    LOG_LOCAL2, "local2"}, {
    LOG_LOCAL3, "local3"}, {
    LOG_LOCAL4, "local4"}, {
    LOG_LOCAL5, "local5"}, {
    LOG_LOCAL6, "local6"}, {
    LOG_LOCAL7, "local7"}, {
    LOG_LPR, "lpr"}, {
    LOG_MAIL, "mail"}, {
    LOG_NEWS, "news"}, {
    LOG_SYSLOG, "syslog"}, {
    LOG_USER, "user"}, {
LOG_UUCP, "uucp"},};

static char *ident = "pdns-logger";
static char *facility = "daemon";
static char rewrites_only = 1;
static char disabled = 0;

static int logfacility_lookup(const char *nfacility, int *logfacility) {
    unsigned int t;

    if (logfacility == NULL) {
        return 0;
    }

    for (t = 0; t < sizeof(facility_names) / sizeof(facility_names[0]); t++) {
        if (!strncmp(facility_names[t].name, nfacility, strlen(facility_names[t].name) + 1)) {
            *logfacility = facility_names[t].facility;
            return 1;
        }
    }

    *logfacility = LOG_DAEMON;

    return 0;
}

static int opt_handler(void *user, const char *section, const char *name, const char *value, int lineno) {
    (void) user;

    if (zstr(section) || zstr(name) || zstr(value)) {
        return 1;
    }

    if (!strncmp(section, "syslog", sizeof("syslog"))) {
        if (!strncmp(name, "ident", sizeof("ident"))) {
        } else if (!strncmp(name, "facility", sizeof("facility"))) {
        } else if (!strncmp(name, "level", sizeof("level"))) {
        } else if (!strncmp(name, "only-rewrites", sizeof("only-rewrites"))) {
            rewrites_only = atoi(value) ? 1 : 0;
        } else if (!strncmp(name, "disabled", sizeof("disabled"))) {
            disabled = atoi(value) ? 1 : 0;
        } else {
            fprintf(stderr, "Unmanaged INI option '%s' at line %d\n", name, lineno);
        }
        return 1;
    }

    return 1;
}

static pdns_status_t syslog_init(const char *inifile) {
    int logf;

    if (zstr(inifile)) {
        return PDNS_NO;
    }

    if (ini_parse(inifile, opt_handler, NULL) != 0) {
        fprintf(stderr, "syslog: Can't read .ini file: '%s'\n", inifile);
        return PDNS_NO;
    }

    if (disabled) {
        fprintf(stderr, "syslog: Disabled according to configuration\n");
        return PDNS_OK;
    }

    logfacility_lookup(facility, &logf);

    openlog(!zstr(ident) ? ident : "pdns-logger", LOG_NDELAY | LOG_PID, logf);

    return PDNS_OK;
}

static pdns_status_t syslog_rotate(void) {
    return PDNS_OK;
}

static pdns_status_t syslog_stop(void) {
    closelog();
    return PDNS_OK;
}

#define write_log() \
    syslog(LOG_NOTICE, "%s", str);

    //fprintf(stderr, "%s\n", str);

static pdns_status_t syslog_log(void *rawpb) {
    PBDNSMessage *msg = rawpb;
    PBDNSMessage__DNSQuestion *q;
    PBDNSMessage__DNSResponse *r;
    int sz, pc;
    char str[1024] = "";
    char tmp[1024] = "";

    if (disabled) {
        return PDNS_OK;
    }

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

pdns_logger_t syslog_engine = {
    syslog_init,
    syslog_rotate,
    syslog_stop,
    syslog_log
};
