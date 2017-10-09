
#include <sqlite3.h>

#include "inih/ini.h"
#include "pdns-logger.h"
#include "dnsmessage.pb-c.h"

static char *dbfile = NULL;
static struct sqlite3 *db = NULL;

/* *************************************************************************** */
/* *************************************************************************** */
/* *************************************************************************** */

/* *INDENT-OFF* */

#define SQL_CREATE_TABLE \
    "CREATE TABLE IF NOT EXISTS logs ( " \
    "   ts      INTEGER NOT NULL, " \
    "   id      INTEGER NOT NULL, " \
    "   qtype   VARCHAR(10), " \
    "   qclass  VARCHAR(10), " \
    "   qname   VARCHAR(256), " \
    "   rcode   VARCHAR(16), " \
    "   rcount  INTEGER, " \
    "   rname   VARCHAR(256), " \
    "   rtype   VARCHAR(10), " \
    "   rclass  VARCHAR(10), " \
    "   rttl    INTEGER, " \
    "   rdata   VARCHAR(256), " \
    "   policy  VARCHAR(100)" \
    ")"

// EDNS
// CLIENT

#define SQL_INSERT \
    "INSERT INTO logs (" \
    "  ts, id, qtype, qclass, qname, rcode, rcount, rname, rtype, rclass, rttl, rdata, policy " \
    ") VALUES (" \
    "  %ld, %d, '%q', '%q', '%q', '%q', %d, '%q', '%q', '%q', %ld, '%q', '%q'  " \
    ")"

#define SQL_CREATE_INDEX \
    "CREATE INDEX IF NOT EXISTS logs_ts_idx    ON  logs(ts);" \
    "CREATE INDEX IF NOT EXISTS logs_qname_idx ON  logs(qname);" \
    "CREATE INDEX IF NOT EXISTS logs_policy_idx ON logs(policy);"

/* *INDENT-ON* */

/* *************************************************************************** */
/* *************************************************************************** */
/* *************************************************************************** */

static pdns_status_t db_exec(const char *sql, char log) {
    int res;
    char *zErr = NULL;

    res = sqlite3_exec(db, sql, NULL, NULL, &zErr);
    if (res != 0) {
        if (zErr != NULL) {
            if (log != 0) {
                fprintf(stderr, "Error executing query: %s (%s)\n", sql, zErr);
            }
            sqlite3_free(zErr);
        }
        return PDNS_NO;
    }

    if (zErr != NULL) {
        sqlite3_free(zErr);
    }

    return PDNS_OK;
}

static pdns_status_t db_open(const char *file) {
    int err;

    err = sqlite3_open_v2(file, &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
    if (err != SQLITE_OK) {
        fprintf(stderr, "sqlite: cannot open sqlite database file '%s'\n", file);
        return PDNS_NO;
    }

    db_exec(SQL_CREATE_TABLE, 1);
    db_exec(SQL_CREATE_INDEX, 1);

    return PDNS_OK;
}

static pdns_status_t db_close(void) {
    sqlite3_close(db);
    return PDNS_OK;
}

/* *************************************************************************** */
/* *************************************************************************** */
/* *************************************************************************** */

static int opt_handler(void *user, const char *section, const char *name, const char *value, int lineno) {
    (void) user;

    if (zstr(section) || zstr(name) || zstr(value)) {
        return 1;
    }

    if (!strncmp(section, "sqlite3", sizeof("sqlite3"))) {
        if (!strncmp(name, "dbfile", sizeof("dbfile"))) {
            dbfile = strdup(value);
        } else {
            fprintf(stderr, "Unmanaged INI option '%s' at line %d\n", name, lineno);
        }
        return 1;
    }
    return 1;
}

static pdns_status_t logsqlite_init(const char *inifile) {
    if (zstr(inifile)) {
        fprintf(stderr, "logsqlite: No inifile to read\n");
        return PDNS_NO;
    }

    if (ini_parse(inifile, opt_handler, NULL) != 0) {
        fprintf(stderr, "logsqlite: Can't read .ini file: '%s'\n", inifile);
        return PDNS_NO;
    }

    if (zstr(dbfile)) {
        fprintf(stderr, "logsqlite: DB file is not set\n");
        return PDNS_NO;
    }

    return db_open(dbfile);
}

static pdns_status_t logsqlite_rotate(void) {
    if (db != NULL) {
        db_close();
        db_open(dbfile);
    }
    return PDNS_OK;
}

static pdns_status_t logsqlite_stop(void) {
    return db_close();
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

pdns_logger_t sqlite_engine = {
    logsqlite_init,
    logsqlite_rotate,
    logsqlite_stop,
    logsqlite_log
};
