
MACRO(TDD_CAR var)
    SET(${var} ${ARGV1})
ENDMACRO(TDD_CAR)


MACRO(TDD_CDR var junk)
    SET(${var} ${ARGN})
ENDMACRO(TDD_CDR)


MACRO(TDD_PARSE_ARGUMENTS prefix arg_names option_names)
    SET(DEFAULT_ARGS)
    FOREACH(arg_name ${arg_names})
        SET(${prefix}_${arg_name})
    ENDFOREACH(arg_name)
    FOREACH(option ${option_names})
        SET(${prefix}_${option} FALSE)
    ENDFOREACH(option)

    SET(current_arg_name DEFAULT_ARGS)
    SET(current_arg_list)

    FOREACH(arg ${ARGN})
        SET(larg_names ${arg_names})
        LIST(FIND larg_names "${arg}" is_arg_name)
        IF (is_arg_name GREATER -1)
            SET(${prefix}_${current_arg_name} ${current_arg_list})
            SET(current_arg_name ${arg})
           SET(current_arg_list)
        ELSE (is_arg_name GREATER -1)
            SET(loption_names ${option_names})
            LIST(FIND loption_names "${arg}" is_option)
            IF (is_option GREATER -1)
                SET(${prefix}_${arg} TRUE)
            ELSE (is_option GREATER -1)
                SET(current_arg_list ${current_arg_list} ${arg})
            ENDIF (is_option GREATER -1)
        ENDIF (is_arg_name GREATER -1)
    ENDFOREACH(arg)
    SET(${prefix}_${current_arg_name} ${current_arg_list})
ENDMACRO(TDD_PARSE_ARGUMENTS)


MACRO(PROJECT_ADD_TEST)

    TDD_PARSE_ARGUMENTS(
        TESTUNIT
        "SOURCE;LIBS;VALGRIND"
        ""
        ${ARGN}
    )
    TDD_CAR(TESTUNIT_NAME    ${TESTUNIT_DEFAULT_ARGS})
    TDD_CDR(TESTUNIT_SOURCES ${TESTUNIT_DEFAULT_ARGS})

    #MESSAGE("*** Arguments for  : ${TESTUNIT_NAME}")
    #MESSAGE("    Sources        : ${TESTUNIT_SOURCE}")
    #MESSAGE("    Link libraries : ${TESTUNIT_LIBS}")
    #MESSAGE("    Use Valgrind   : ${TESTUNIT_VALGRIND}")

    IF   ("${TESTUNIT_SOURCE}" STREQUAL "")
        MESSAGE(FATAL_ERROR "No source files for unit ${TESTUNIT_NAME}")
    ENDIF("${TESTUNIT_SOURCE}" STREQUAL "")

    ADD_EXECUTABLE(
        ${TESTUNIT_NAME} ${TESTUNIT_SOURCE}
    )

    TARGET_LINK_LIBRARIES(
        ${TESTUNIT_NAME}
            ${TESTUNIT_LIBS}
    )

    SET_TARGET_PROPERTIES(
        ${TESTUNIT_NAME}
            PROPERTIES
                PREFIX ""
                PROJECT_LABEL "Module ${TESTUNIT_NAME}"
    )

    IF (NOT APPLE AND NOT WIN32)
    SET_TARGET_PROPERTIES(
        ${TESTUNIT_NAME}
        PROPERTIES
            LINK_FLAGS "${MODULES_LINKER_OPTIONS}"
    )
    ENDIF(NOT APPLE AND NOT WIN32)

    IF( ${TESTUNIT_VALGRIND} STREQUAL "1")
        ADD_TEST(${TESTUNIT_NAME}_vg
                valgrind
                --error-exitcode=1
                --read-var-info=yes
                --leak-check=full
                --show-leak-kinds=all
                ./${TESTUNIT_NAME}
    )
    ELSE( ${TESTUNIT_VALGRIND} STREQUAL "1")
        ADD_TEST(${TESTUNIT_NAME} ${TESTUNIT_NAME} )
    ENDIF( ${TESTUNIT_VALGRIND} STREQUAL "1")

ENDMACRO(PROJECT_ADD_TEST)
