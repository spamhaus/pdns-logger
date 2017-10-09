
CONFIGURE_FILE(
    ${CMAKE_CURRENT_SOURCE_DIR}/pdns-logger.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/pdns-logger.h
)

SET (INCLUDES "${PROTOBUFC_INCLUDEDIR}")

IF(SYSTEMD_FOUND)
    SET (INCLUDES "${INCLUDES} ${SYSTEMD_INCLUDE_DIR}")
ENDIF(SYSTEMD_FOUND)

INCLUDE_DIRECTORIES(
    ${INCLUDES}
)

SET(SOURCES
    dnsmessage.pb-c.c
    inih/ini.c

    pdns-logger-file.c
    #pdns-logger-syslog.c

    pdns-logger-protobuf.c
    pdns-logger-queue.c
    pdns-logger-dns.c
    pdns-logger-ini.c
    pdns-logger.c
)


SET (LINKLIBS
    ${PROTOBUFC_LIBRARIES}
    pthread
)

IF(SQLITE3_FOUND)
    SET (SOURCES
        ${SOURCES}
            pdns-logger-sqlite.c
    )
    SET (LINKLIBS
        ${LINKLIBS}
            ${SQLITE3_LIBRARIES}
    )
ENDIF(SQLITE3_FOUND)

ADD_EXECUTABLE (
    pdns-logger
        ${SOURCES}
)

TARGET_LINK_LIBRARIES (
    pdns-logger
        ${LINKLIBS}
)

SET_TARGET_PROPERTIES(
    pdns-logger
    PROPERTIES
        PROJECT_LABEL "pdns-logger"
        OUTPUT_NAME pdns-logger
)

INSTALL(
    TARGETS
        pdns-logger
    RUNTIME DESTINATION "${SBIN_INSTALL_DIR}"
    LIBRARY DESTINATION "${BIN_INSTALL_DIR}"
    ARCHIVE DESTINATION "${BIN_INSTALL_DIR}"
)

INSTALL(DIRECTORY DESTINATION ${PROJECT_STATE_DIR})
