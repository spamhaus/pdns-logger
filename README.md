PDNS-LOGGER
===========

** pdns-logger ** is a small daemon that leverages the protobufServer feature of Powerdns Recursor
to log the queries to syslog, to a file (optional) and to an sqlite3 database.

Use cases
---------

The program was written to debug RPZ rewrites in an easy way.
This code is not meant to parse heavy query streams. However, the code is threaded and all the major
building blocks to support big numbers of queries per second are already available. Feel free to send a merge request.

Installation
------------

To compile pdns-logger, you need [cmake](https://cmake.org/).

Other required libraries are:
- [protobuf-c](https://github.com/protobuf-c/protobuf-c)
- [sqlite3](https://www.sqlite.org/)

Checkout the Github code and then
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

PACKAGE DEPENDENCIES
--------------------
cmake
pkg-config
libprotobuf-c-dev and libprotobuf-c1
libsqlite3-0 libsqlite3-dev

HOW TO BUILD
------------

Developement
------------
- Source hosted at [GitHub](https://github.com/spamhays/pdns-logger)
- Report issues, questions, feature requests on [GitHub Issues](https://github.com/spamhaus/pdns-logger/issues)

Authors
-------
[Massimo Cetra](http://www.ctrix.it/)
