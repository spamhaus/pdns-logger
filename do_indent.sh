#!/bin/bash

#
# This file is used to conform all code to the same style.
# Please use indent, if you need to submit patches or contribute
# your code.
#

function do_indent() {
    local FROM=$1
    local TO=$2


    /usr/bin/indent \
        --braces-on-func-def-line \
        \
        --no-blank-lines-after-declarations \
        --no-blank-lines-after-procedures \
        --no-blank-lines-after-commas \
        --break-before-boolean-operator \
        --braces-on-struct-decl-line \
        --honour-newlines \
        --braces-on-if-line \
        --comment-indentation33 \
        --declaration-comment-column33 \
        --no-comment-delimiters-on-blank-lines \
        --cuddle-else \
        --continuation-indentation4 \
        --case-indentation0 \
        --line-comments-indentation0 \
        --declaration-indentation1 \
        --dont-format-first-column-comments \
        --parameter-indentation0 \
        --continue-at-parentheses \
        --no-space-after-function-call-names \
        --dont-break-procedure-type \
        --space-after-if \
        --space-after-for \
        --space-after-while \
        --no-space-after-parentheses \
        --dont-star-comments \
        --swallow-optional-blank-lines \
        --dont-format-comments \
        --else-endif-column33 \
        --space-special-semicolon \
        --indent-label1 \
        --case-indentation4 \
        \
        --tab-size4 \
        -i4 \
        -l200 \
        --no-tabs \
        $FROM \
        -o $TO
}


SOURCES=`find src/ -name *.[c,h] -print;`

for SOURCE in $SOURCES; do
    if $( echo $SOURCE | grep -v --quiet 'libmbedtls' )
    then
        echo $SOURCE
        do_indent $SOURCE $SOURCE
    fi
done;

