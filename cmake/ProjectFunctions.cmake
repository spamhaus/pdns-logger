#
# Usage:
# install_if_not_exists(source_file destination_directory)
#

FUNCTION(install_if_not_exists src dest)
    IF(NOT IS_ABSOLUTE "${src}")
        SET(src "${CMAKE_CURRENT_SOURCE_DIR}/${src}")
    ENDIF()

    GET_FILENAME_COMPONENT(src_name "${src}" NAME)

    IF(NOT IS_ABSOLUTE "${dest}")
        SET(dest "${CMAKE_INSTALL_PREFIX}/${dest}")
    ENDIF()

    INSTALL(CODE "
        IF(NOT EXISTS \"\$ENV{DESTDIR}${dest}/${src_name}\")
            #file(INSTALL \"${src}\" DESTINATION \"${dest}\")
            MESSAGE(STATUS \"Installing: \$ENV{DESTDIR}${dest}/${src_name}\")
            EXECUTE_PROCESS(COMMAND \${CMAKE_COMMAND} -E copy \"${src}\"
                      \"\$ENV{DESTDIR}${dest}/${src_name}\"
                      RESULT_VARIABLE copy_result
                      ERROR_VARIABLE error_output)
            IF(copy_result)
                MESSAGE(FATAL_ERROR \${error_output})
            ENDIF()
        ELSE()
            message(STATUS \"Skipping  : \$ENV{DESTDIR}${dest}/${src_name}\")
        ENDIF()
    ")
ENDFUNCTION(install_if_not_exists)
