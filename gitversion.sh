#!/bin/sh
cd "`dirname $0`"
rev="`git rev-parse --short HEAD`"
dirty=
if [ -n "`git status --porcelain --untracked=no`" ]; then
    dirty=+
fi
if [ "$1" = "--full" ]; then
    mainversion="`git describe --tags --abbrev=0 | sed s/^v//`-"
fi
echo "${mainversion}${rev}${dirty}"
