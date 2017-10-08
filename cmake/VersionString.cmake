#
# Create a tag with 'git tag <name>' and 'git push --tags'
#

FIND_PACKAGE(Git)

IF(NOT GIT_FOUND)
    # cmake 2.8.1 (in Lucid) does not have a git Cmake interface
    # cmake 2.8.5 (in Oneiric) does have it
    MESSAGE(STATUS "Couldn't find CMAKE interface to git, old version of Cmake?  looking by hand...")
    EXECUTE_PROCESS(
        COMMAND git --version
        RESULT_VARIABLE RC
    )
    IF(${RC} EQUAL 0)
        SET(GIT_FOUND 1)
        SET(GIT_EXECUTABLE "git")
    ELSE()
        MESSAGE(ERROR "couldn't run git executable!")
    ENDIF()
ENDIF()

IF(GIT_FOUND)
    EXECUTE_PROCESS(
        COMMAND ${GIT_EXECUTABLE} describe --tags
        RESULT_VARIABLE res_var
        OUTPUT_VARIABLE GIT_COM_ID
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    IF( NOT ${res_var} EQUAL 0 )
        MESSAGE(WARNING "Git failed (not a repo, or no tags)." )
        FILE(READ "git-tag.txt" GIT_COMMIT_ID)
        MESSAGE(STATUS "GIT_COMMIT_ID: " ${GIT_COMMIT_ID} " (from file)")
    ELSE()
        STRING(REPLACE "\n" "" GIT_COMMIT_ID ${GIT_COM_ID} )
        MESSAGE(STATUS "GIT_COMMIT_ID: " ${GIT_COMMIT_ID})
    ENDIF()
ELSE()
    # if we don't have git, try to read git-tag from file instead
    FILE(READ "git-tag.txt" GIT_COMMIT_ID)
    MESSAGE(WARNING "Git not found. Reading tag from git-tag.txt instead: " ${GIT_COMMIT_ID})
    #MESSAGE(STATUS "GIT_COMMIT_ID: " ${GIT_COMMIT_ID} " (from file)")
ENDIF()

SET( vstring "//version_string.hpp - written by cmake. changes will be lost!\n"
             "#ifndef VERSION_STRING\n"
             "#define VERSION_STRING \"${GIT_COMMIT_ID}\"\n"
             "#endif\n"
)

FILE(WRITE ${CMAKE_CURRENT_BINARY_DIR}/version_string.hpp ${vstring} )
SET_SOURCE_FILES_PROPERTIES(
    ${CMAKE_CURRENT_BINARY_DIR}/version_string.hpp
    PROPERTIES GENERATED TRUE
    HEADER_FILE_ONLY TRUE
)

# copy the file to the final header only if the version changes
# reduces needless rebuilds
#EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E copy_if_different
#                        version_string.hpp.txt /version_string.hpp)

