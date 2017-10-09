
#include "inih/ini.h"
#include "pdns-logger.h"
#include "dnsmessage.pb-c.h"

static FILE *fp = NULL;
static char *file = NULL;
static fifo_t *queue = NULL;

static int opt_handler(void *user, const char *section, const char *name, const char *value, int lineno) {
    (void) user;

    if ( zstr(section) || zstr(name) || zstr(value) ) {
        return 1;
    }

    if ( !strncmp(section, "logfile", sizeof("logfile")) ) {
        if ( !strncmp(name, "logfile", sizeof("logfile")) ) {
            file = strdup(value);
        }
        else {
            fprintf(stderr, "Unmanaged INI option '%s' at line %d\n", name, lineno);
        }

        //fprintf(stderr, "%d * %s -> %s -> %s\n", lineno, section, name, value);
        return 1;
    }


    return 1;
}

static pdns_status_t logfile_init(const char *inifile) {
    if ( zstr(inifile) ) {
        return PDNS_NO;
    }

    if (ini_parse(inifile, opt_handler, NULL) != 0) {
        fprintf(stderr, "logfile: Can't read .ini file: '%s'\n", inifile);
        return PDNS_NO;
    }

    queue = fifo_init();
    if ( queue == NULL ) {
        return PDNS_NO;
    }

    return PDNS_OK;
}

static pdns_status_t logfile_rotate(void) {

}

static pdns_status_t logfile_stop(void) {
    safe_free(file);
    if ( fp != NULL ) {
        fclose(fp);
    }

    return PDNS_OK;
}

static pdns_status_t logfile_log(void *rawpb) {
    PBDNSMessage *msg = rawpb;
    PBDNSMessage__DNSQuestion *q;
    PBDNSMessage__DNSResponse *r;
    int sz, pc;
    char str[1024] = "";
    char tmp[1024] = "";

    if ( msg == NULL || msg->response == NULL ) {
        return PDNS_OK;
    }

    sz = sizeof(str) - 1;

    pc = snprintf(tmp, sizeof(tmp), "type: %d ", msg->type);
    strncat(str, tmp, sz);
    sz -= pc;

    if (msg->has_messageid) {
        pc = snprintf(tmp, sizeof(tmp), "msgid: %d ", msg->type);
        strncat(str, tmp, sz);
        sz -= pc;
    }

    q = msg->question;
    if ( q != NULL ) {
        if ( q->has_qtype ) {
            pc = snprintf(tmp, sizeof(tmp), "qt: %d ", q->qtype);
            strncat(str, tmp, sz);
            sz -= pc;
        }

        if ( q->has_qclass ) {
            pc = snprintf(tmp, sizeof(tmp), "qc: %d ", q->qclass);
            strncat(str, tmp, sz);
            sz -= pc;
        }

        pc = snprintf(tmp, sizeof(tmp), "qn: %s ", q->qname);
        strncat(str, tmp, sz);
        sz -= pc;
    }

    r = msg->response;
    if ( r != NULL ) {
        if ( r->has_rcode ) {
            pc = snprintf(tmp, sizeof(tmp), "rc: %d ", r->rcode);
            strncat(str, tmp, sz);
            sz -= pc;
        }

        if (r->n_rrs > 0) {
            unsigned int t;
            PBDNSMessage__DNSResponse__DNSRR *rr;

            pc = snprintf(tmp, sizeof(tmp), "rr#: %zu ", r->n_rrs);
            strncat(str, tmp, sz);
            sz -= pc;

            for (t = 0; t < r->n_rrs; t++) {
                rr = r->rrs[t];

                if (rr->has_type) {
                    pc = snprintf(tmp, sizeof(tmp), "rtype: %d ", rr->type);
                    strncat(str, tmp, sz);
                    sz -= pc;
                }

                if (rr->has_class_) {
                    pc = snprintf(tmp, sizeof(tmp), "rclass: %d ", rr->class_);
                    strncat(str, tmp, sz);
                    sz -= pc;
                }

                if (rr->has_ttl) {
                    pc = snprintf(tmp, sizeof(tmp), "rttl: %d ", rr->ttl);
                    strncat(str, tmp, sz);
                    sz -= pc;
                }

                if (rr->has_rdata) {
                    if (rr->has_type && rr->type == 1 && rr->rdata.len == 4) {
                        char ip[INET6_ADDRSTRLEN];

                        inet_ntop(AF_INET, (const void*) rr->rdata.data, ip, sizeof(ip));

                        pc = snprintf(tmp, sizeof(tmp), "rdata: %s ", ip);
                        strncat(str, tmp, sz);
                        sz -= pc;
                    } else if (rr->has_type && rr->type == 28 && rr->rdata.len == 16) {
                        char ip[INET6_ADDRSTRLEN];

                        inet_ntop(AF_INET6, (const void*) rr->rdata.data, ip, sizeof(ip));

                        pc = snprintf(tmp, sizeof(tmp), "rdata: %s ", ip);
                        strncat(str, tmp, sz);
                        sz -= pc;
                    } else if (rr->has_type && ((rr->type == 2) || (rr->type == 5) || (rr->type == 6) || (rr->type == 15))) {
                        /* CNAME works */
                        /* NS SOA MX do not seem to pass any data */
                        pc = snprintf(tmp, sizeof(tmp), "rdata: %s ", rr->rdata.data);
                        strncat(str, tmp, sz);
                        sz -= pc;
                    }
                    else {
                        pc = snprintf(tmp, sizeof(tmp), "rdata (not supported) ");
                        strncat(str, tmp, sz);
                        sz -= pc;
                    }
                }
            }

        }

        if (!zstr(r->appliedpolicy)) {
            pc = snprintf(tmp, sizeof(tmp), "policy: %s ", r->appliedpolicy);
            strncat(str, tmp, sz);
            sz -= pc;
        }
    }

    fprintf(stderr, "'%s'\n", str);
    return PDNS_OK;
}

pdns_logger_t logfile_engine = {
    logfile_init,
    logfile_rotate,
    logfile_stop,
    logfile_log
};
