
#include <sqlite3.h>

#include "inih/ini.h"
#include "pdns-logger.h"
#include "dnsmessage.pb-c.h"

static int opt_handler(void *user, const char *section, const char *name, const char *value, int lineno) {
    (void) user;

    if (zstr(section) || zstr(name) || zstr(value)) {
        return 1;
    }

    if (!strncmp(section, "sqlite3", sizeof("sqlite3"))) {
        if (!strncmp(name, "dbfile", sizeof("dbfile"))) {
        } else if (!strncmp(name, "force-flush", sizeof("force-flush"))) {
        } else {
            fprintf(stderr, "Unmanaged INI option '%s' at line %d\n", name, lineno);
        }
        return 1;
    }
    return 1;
}

static pdns_status_t logsqlite_init(const char *inifile) {
    if (zstr(inifile)) {
        return PDNS_NO;
    }

    if (ini_parse(inifile, opt_handler, NULL) != 0) {
        fprintf(stderr, "logsqlite: Can't read .ini file: '%s'\n", inifile);
        return PDNS_NO;
    }

    return PDNS_OK;
}

static pdns_status_t logsqlite_rotate(void) {
    return PDNS_OK;
}

static pdns_status_t logsqlite_stop(void) {
    return PDNS_OK;
}

static pdns_status_t logsqlite_log(void *rawpb) {
#if 0
/*
    PBDNSMessage *msg = rawpb;
    PBDNSMessage__DNSQuestion *q;
    PBDNSMessage__DNSResponse *r;
    int sz, pc;
    char str[1024] = "";
    char tmp[1024] = "";

    if (msg == NULL || msg->response == NULL) {
        return PDNS_OK;
    }

    sz = sizeof(str) - 1;

    if (msg->has_id) {
        pc = snprintf(tmp, sizeof(tmp), "QID: %d ", msg->id);
        strncat(str, tmp, sz);
        sz -= pc;
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

        if (r->n_rrs > 0) {
            unsigned int t;
            PBDNSMessage__DNSResponse__DNSRR *rr;

            pc = snprintf(tmp, sizeof(tmp), "rrcount: %zu ", r->n_rrs);
            strncat(str, tmp, sz);
            sz -= pc;

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
            }

        }

        if (!zstr(r->appliedpolicy)) {
            pc = snprintf(tmp, sizeof(tmp), "policy: '%s' ", r->appliedpolicy);
            strncat(str, tmp, sz);
            sz -= pc;
        }
    }
*/
#endif
    return PDNS_OK;
}

pdns_logger_t logsqlite_engine = {
    logsqlite_init,
    logsqlite_rotate,
    logsqlite_stop,
    logsqlite_log
};
