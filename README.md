PDNS-LOGGER
===========

** pdns-logger ** is a small daemon that leverages the protobufServer feature of Powerdns Recursor
to log the queries to syslog, to a file and to an sqlite3 database.

Use cases
---------

The program was written to debug RPZ rewrites in an easy way because unfortunately, debugging RPZ when used together with powerdns-recursor
is an ugly task.

The initial version of this program was logging to file, only.
Syslog and sqlite3 came later to add more features and increase the use cases.

In particular, the sqlite3 database can be easily used to log queries and show them on a nice web interface.

DEBIAN OR UBUNTU PACKAGE
------------------------

If you need a debian or an ubuntu package, the repository includes everything you need.

First, you need to install the build environment and the required developent libraries.
```bash
apt-get install cdbs debhelper devscripts cmake build-essential pkg-config libprotobuf-c-dev libsqlite3-dev
```

Then you need to start the configuration, compilation and the build of the packages. Don't worry, it's a single command:
```bash
dpkg-buildpackage -uc -us -b
```

After the process, you will have a nice debian/ubuntu package ready to be installed:

```bash
dpkg -i ../pdns-logger*.deb
```

MANUAL INSTALLATION
-------------------

To compile pdns-logger, you need [cmake](https://cmake.org/).

Other required libraries are:
- [protobuf-c](https://github.com/protobuf-c/protobuf-c)
- [sqlite3](https://www.sqlite.org/)

Checkout the code from Git and then
```bash
mkdir -p  build
cd build
cmake ../ \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX="/usr/" \
    -DSYSCONF_INSTALL_DIR="/etc"

make
make install
```

After installation, you will have 
* /etc/pdns-logger/pdns-logger.ini - The configuration file
* /usr/sbin/pdns-logger - the daemon

Command line options are:
```bash
Usage: pdns-collector [options]
Options:
  -c configfile       Location of the config file (default: /etc/pdns-logger/pdns-logger.ini)
  -f                  Do not fork and stay in foreground
  -h                  Print this message and exit.

```

In the debian/ directory there is an init script (`pdns-logger.init`) that you can adapt and use for your purposes.

POWERDNS CONFIGURATION
----------------------

To effectively use the daemon, you need to configure powerdns.
If you're using RPZ, you already know that you need to add the following configuration to recursor.conf:
```
lua-config-file=/etc/powerdns/recursor.conf.lua
```

What you're probably missing is that in the recursor.conf.lua you need to add:
```lua
protobufServer("127.0.0.1:4242")
```

This will instruct powerdns-recursor to send the logs to our daemon.
If you're using pdns-recursor 4.1.0 or higher, you may want to use this command instead. With this, you will be able to log only the RPZ rewrites and not all the queries sent to the logger.:
```lua
protobufServer("127.0.0.1:4242", 2, 100, 1, 32, 128, false, true)
```

Please notice that the IP address and the port must match what is configured in the .ini file of the daemon.

Restart the recursor. If you're on debian or ubuntu, then

```bash
service pdns-resolver restart
```

will be enough.

PDNS-LOGGER CONFIGURATION
-------------------------
Don't forget to have a look and eventually modify the configuration of the logger.
The configuration INI file is usually in "/etc/pdns-logger/pdns-logger.ini" and contains very few directives.

```ini
[globals]
allow-root=0
bind-ip=127.0.0.1
bind-port=4242
;foreground=0            ; If set, this will override the CLI -f option


[syslog]
disabled = 0            ; Should we disable the syslog backend ?
only-rewrites=0         ; log only RPZ rewrites
ident=pdns-logger       ; the syslog ident
facility=syslog         ; the log facility


[logfile]
disabled = 0            ; should we disable the log-to-file backend ?
only-rewrites=0         ; log only RPZ rewrites
                        ; what is the path of our log file ?
logfile=/var/log/pdns-logger/pdns.log
force-flush=1           ; flush buffers to disk at each query


[sqlite3]
disabled = 0;           ; should we disable the sqlite3 backend ?
only-rewrites=0         ; log only RPZ rewrites
                        ; what is the path to our sqlite3 database ?
dbfile=/var/lib/pdns-logger/queries.db
```

LOG FORMAT
----------

The logfile and the syslog backends share the same format.
Each line will have the following format:
```
Nov  2 18:01:19 NS0 pdns-logger[12718]: QID: 20542 from: 127.0.0.1 qtype: A qclass: IN qname: dbltest.com. rcode: NXDOMAIN rrcount: 0 policy: 'DBL'
```

The sinble bits in the line are:
* QID: the query ID, as sent from the client to the DNS server
* FROM: the querier IP address
* QTYPE: the query type (A, NS, MX, ...)
* QCLASS: the query class. Usually it's always 'IN'
* QNAME: the query name, that is the main thing that you need to know.
* RCODE: the response code sent back to the client (NXDOMAIN, NOERROR etcc...)
* RRCOUNT: the number of RR sent back in the response packet
* POLICY: it's the policy, if present, applied to the response packet, according to an RPZ zone. This bit may be missing if the query was not rewritten.

When the response packet contains more than one RR, then additional fields (or tuples, if RRCOUNT is greater than one) will apply:
* RNAME: the domain name sent back
* RTYPE: the response type
* RCLASS: same as QCLASS
* RTTL: the TTL of the record
* RDATA: the valie of the RR - Remember that PDNS does log all the rewrites and additional queries (but not all RTYPEs)


The sqlite backend stores the same data in a table with the following schema:

```sql
CREATE TABLE IF NOT EXISTS logs ( 
   ts      INTEGER NOT NULL, 
   querier VARCHAR(48), 
   id      INTEGER NOT NULL, 
   qtype   VARCHAR(10), 
   qclass  VARCHAR(10), 
   qname   VARCHAR(256), 
   rcode   VARCHAR(16), 
   rcount  INTEGER, 
   rname   VARCHAR(256), 
   rtype   VARCHAR(10), 
   rclass  VARCHAR(10), 
   rttl    INTEGER, 
   rdata   VARCHAR(256), 
   policy  VARCHAR(100)
)"
```
The meaning of the columns are identical to the corresponding text version.


DEVELOPEMENT
------------
- Source hosted at [GitHub](https://github.com/spamhays/pdns-logger)
- Report issues, questions, feature requests on [GitHub Issues](https://github.com/spamhaus/pdns-logger/issues)

Authors
-------
[Massimo Cetra] (http://www.ctrix.it/)


Enjoy!
