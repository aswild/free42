#!/bin/sh
cd "`dirname $0`"
rev="`git rev-parse --short HEAD`"
dirty=
if [ -n "`git status --porcelain --untracked=no`" ]; then
    dirty=+
fi
if [ "$1" = "--full" ]; then
    mainversion="`cat VERSION`-"
fi
echo "${mainversion}${rev}${dirty}"
