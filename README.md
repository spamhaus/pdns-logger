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

Installation
------------

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

DEBIAN OR UBUNTU PACKAGE
------------------------

If you need a debian or an ubuntu package, the repository includes everything you need.

```bash
apt-get install cdbs debhelper devscripts cmake build-essential pkg-config libprotobuf-c-dev libsqlite3-dev
```

```bash
dpkg-buildpackage -uc -us -b
```

After the process, you will have a nice debian/ubuntu package ready to be installed:

```bash
dpkg -i ../pdns-logger*.deb
```

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
Please notice that the IP address and the port must match what is configured in the .ini file of the daemon.

Developement
------------
- Source hosted at [GitHub](https://github.com/spamhays/pdns-logger)
- Report issues, questions, feature requests on [GitHub Issues](https://github.com/spamhaus/pdns-logger/issues)

Authors
-------
[Massimo Cetra](http://www.ctrix.it/)


Enjoy!
