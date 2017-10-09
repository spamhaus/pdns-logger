
/* *INDENT-OFF* */
enum dns_section_e{
    DNS_S_NONE = 0,
    DNS_S_ANSWERS,
    DNS_S_AUTHORITY,
    DNS_S_ADDITIONAL
};

enum dns_class_e {
  DNS_C_INVALID = 0,    /* invalid class */
  DNS_C_IN      = 1,    /* Internet */
  DNS_C_CH      = 3,    /* CHAOS */
  DNS_C_HS      = 4,    /* HESIOD */
  DNS_C_ANY     = 255   /* wildcard */
};

enum dns_type_e {
  DNS_T_INVALID         = 0,    /* Cookie. */
  DNS_T_A               = 1,    /* Host address. */
  DNS_T_NS              = 2,    /* Authoritative server. */
  DNS_T_MD              = 3,    /* Mail destination. */
  DNS_T_MF              = 4,    /* Mail forwarder. */
  DNS_T_CNAME           = 5,    /* Canonical name. */
  DNS_T_SOA             = 6,    /* Start of authority zone. */
  DNS_T_MB              = 7,    /* Mailbox domain name. */
  DNS_T_MG              = 8,    /* Mail group member. */
  DNS_T_MR              = 9,    /* Mail rename name. */
  DNS_T_NULL            = 10,   /* Null resource record. */
  DNS_T_WKS             = 11,   /* Well known service. */
  DNS_T_PTR             = 12,   /* Domain name pointer. */
  DNS_T_HINFO           = 13,   /* Host information. */
  DNS_T_MINFO           = 14,   /* Mailbox information. */
  DNS_T_MX              = 15,   /* Mail routing information. */
  DNS_T_TXT             = 16,   /* Text strings. */
  DNS_T_RP              = 17,   /* Responsible person. */
  DNS_T_AFSDB           = 18,   /* AFS cell database. */
  DNS_T_X25             = 19,   /* X_25 calling address. */
  DNS_T_ISDN            = 20,   /* ISDN calling address. */
  DNS_T_RT              = 21,   /* Router. */
  DNS_T_NSAP            = 22,   /* NSAP address. */
  DNS_T_NSAP_PTR        = 23,   /* Reverse NSAP lookup (deprecated). */
  DNS_T_SIG             = 24,   /* Security signature. */
  DNS_T_KEY             = 25,   /* Security key. */
  DNS_T_PX              = 26,   /* X.400 mail mapping. */
  DNS_T_GPOS            = 27,   /* Geographical position (withdrawn). */
  DNS_T_AAAA            = 28,   /* Ip6 Address. */
  DNS_T_LOC             = 29,   /* Location Information. */
  DNS_T_NXT             = 30,   /* Next domain (security). */
  DNS_T_EID             = 31,   /* Endpoint identifier. */
  DNS_T_NIMLOC          = 32,   /* Nimrod Locator. */
  DNS_T_SRV             = 33,   /* Server Selection. */
  DNS_T_ATMA            = 34,   /* ATM Address */
  DNS_T_NAPTR           = 35,   /* Naming Authority PoinTeR */
  DNS_T_KX              = 36,   /* Key Exchange */
  DNS_T_CERT            = 37,   /* Certification record */
  DNS_T_A6              = 38,   /* IPv6 address (deprecates AAAA) */
  DNS_T_DNAME           = 39,   /* Non-terminal DNAME (for IPv6) */
  DNS_T_SINK            = 40,   /* Kitchen sink (experimentatl) */
  DNS_T_OPT             = 41,   /* EDNS0 option (meta-RR) */
  DNS_T_TSIG            = 250,  /* Transaction signature. */
  DNS_T_IXFR            = 251,  /* Incremental zone transfer. */
  DNS_T_AXFR            = 252,  /* Transfer zone of authority. */
  DNS_T_MAILB           = 253,  /* Transfer mailbox records. */
  DNS_T_MAILA           = 254,  /* Transfer mail agent records. */
  DNS_T_ANY             = 255,  /* Wildcard match. */
  DNS_T_ZXFR            = 256,  /* BIND-specific, nonstandard. */
  DNS_T_MAX             = 65536
};

enum dns_rcode_e {        /* reply code */
  DNS_R_NOERROR         = 0,    /* ok, no error */
  DNS_R_FORMERR         = 1,    /* format error */
  DNS_R_SERVFAIL        = 2,    /* server failed */
  DNS_R_NXDOMAIN        = 3,    /* domain does not exists */
  DNS_R_NOTIMPL         = 4,    /* not implemented */
  DNS_R_REFUSED         = 5,    /* query refused */
                                /* these are for BIND_UPDATE */
  DNS_R_YXDOMAIN        = 6,    /* Name exists */
  DNS_R_YXRRSET         = 7,    /* RRset exists */
  DNS_R_NXRRSET         = 8,    /* RRset does not exist */
  DNS_R_NOTAUTH         = 9,    /* Not authoritative for zone */
  DNS_R_NOTZONE         = 10,   /* Zone of record different from zone section */
                                /*ns_r_max = 11,*/
                                /* The following are TSIG extended errors */
  DNS_R_BADSIG          = 16,
  DNS_R_BADKEY          = 17,
  DNS_R_BADTIME         = 18
};
/* *INDENT-ON* */

const char *pdns_logger_rcode2p(enum dns_rcode_e);
const char *pdns_logger_type2p(enum dns_type_e);
const char *pdns_logger_class2p(enum dns_class_e);
