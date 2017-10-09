PROJECT (PDNS-LOGGER)

SET(APPLICATION_NAME "pdns-logger")

CMAKE_MINIMUM_REQUIRED(VERSION 2.8.0)

SET(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

IF(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
    MESSAGE(FATAL_ERROR "Prevented in-tree built. This is not a good practice. Please read README.md to find a suitable example.")
ENDIF()

IF (WIN32)
    MESSAGE(FATAL_ERROR "This software does not work on Windows, sorry!")
ENDIF (WIN32)

INCLUDE(DefineCMakeDefaults)
INCLUDE(DefineCompilerFlags)
INCLUDE(HeadersChecks)
INCLUDE(TestInline)

INCLUDE(DefineInstallationPaths)
INCLUDE(ProjectDefaults)
INCLUDE(ProjectFunctions)

INCLUDE(TDDHelpers)
INCLUDE(ModuleHelpers)

# **********************************************************

SET(SYSTEMD_FIND_REQUIRED OFF)
SET(SYSTEMD_FIND_QUIETLY OFF)
INCLUDE(FindSystemd)
IF(SYSTEMD_FOUND)
    ADD_DEFINITIONS(-DSYSTEMD_FOUND)
ENDIF(SYSTEMD_FOUND)

SET(PROTOBUFC_FIND_REQUIRED ON)
SET(PROTOBUFC_FIND_QUIETLY ON)
INCLUDE(FindProtobufC)

SET(THREADS_PREFER_PTHREAD_FLAG ON)
FIND_PACKAGE(Threads REQUIRED)
IF(NOT CMAKE_USE_PTHREADS_INIT)
    MESSAGE(ERROR "Cannot find PTHREAD library")
ENDIF(NOT CMAKE_USE_PTHREADS_INIT)

SET(SQLITE3_FIND_REQUIRED OFF)
SET(SQLITE3_FIND_QUIETLY ON)
INCLUDE(FindSqlite3)

# *******************************************************************************

ADD_SUBDIRECTORY(config)
ADD_SUBDIRECTORY(src)

# *******************************************************************************
# *** TESTS
# *******************************************************************************

SET(_USE_TESTS   false CACHE BOOL  "Compile the tests")

IF ( ${_USE_TESTS} )
    ENABLE_TESTING()
    MESSAGE(STATUS "ENABLING TESTS")
    ADD_SUBDIRECTORY(tests)
ELSE( ${NMAIL_USE_TESTS} )
    MESSAGE(STATUS "TESTS NOT ENABLED")
ENDIF( ${_USE_TESTS} )

# *******************************************************************************
# *******************************************************************************
# *******************************************************************************

IF(SYSTEMD_FOUND)
    MESSAGE(STATUS "SYSTEMD SUPPORT:                    Enabled")
ELSE(SYSTEMD_FOUND)
    MESSAGE(STATUS "SYSTEMD SUPPORT:                    NOT Enabled")
ENDIF(SYSTEMD_FOUND)

IF(PROTOBUFC_FOUND)
    MESSAGE(STATUS "PROTOBUFC SUPPORT:                  Enabled")
ELSE(PROTOBUFC_FOUND)
    MESSAGE(STATUS "PROTOBUFC SUPPORT:                  NOT Enabled")
ENDIF(PROTOBUFC_FOUND)

IF(SQLITE3_FOUND)
    MESSAGE(STATUS "SQLITE3 SUPPORT:                    Enabled")
ELSE(SQLITE3_FOUND)
    MESSAGE(STATUS "SQLITE3 SUPPORT:                    NOT Enabled")
ENDIF(SQLITE3_FOUND)
