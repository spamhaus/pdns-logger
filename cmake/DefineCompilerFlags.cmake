# define system dependent compiler flags

INCLUDE(CheckCCompilerFlag)

IF (UNIX AND NOT WIN32)

    ADD_DEFINITIONS(-D_GNU_SOURCE)

    CHECK_C_COMPILER_FLAG("-fstack-protector" WITH_STACK_PROTECTOR)
    IF (WITH_STACK_PROTECTOR)
        ADD_DEFINITIONS(-fstack-protector)
    ENDIF (WITH_STACK_PROTECTOR)

    CHECK_C_COMPILER_FLAG("-D_FORTIFY_SOURCE=2" WITH_FORTIFY_SOURCE)
    IF (WITH_FORTIFY_SOURCE)
        ADD_DEFINITIONS(-D_FORTIFY_SOURCE=2)
    ENDIF (WITH_FORTIFY_SOURCE)


    SET(CMAKE_INCLUDE_PATH "/usr/include/ /usr/local/include" )

    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -W -Wreturn-type  -Wstrict-prototypes")
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -W -Wmissing-prototypes -Wmissing-declarations -Wpointer-arith -Wchar-subscripts -Wformat=2 -Wbad-function-cast -Wno-strict-aliasing -Wshadow")
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wdeclaration-after-statement -Wuninitialized -Wno-format-nonliteral" )
    #SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-omit-frame-pointer -Wstrict-aliasing=2")
    #SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-unused -Wno-unused-parameter" )
    #SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fvisibility=hidden")
    #SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fstack-protector-all")
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fstack-protector-strong")
    #SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=undefined -fno-sanitize-recover")

    # Mandatory for loadable modules
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC" )

    IF ("${CMAKE_BUILD_TYPE}" STREQUAL "Release")
        SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -Wno-strict-aliasing")
    ENDIF ("${CMAKE_BUILD_TYPE}" STREQUAL "Release")

    IF ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
        SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O0 -Wstrict-aliasing=2 -fno-omit-frame-pointer")
    ENDIF ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")


ENDIF (UNIX AND NOT WIN32)
