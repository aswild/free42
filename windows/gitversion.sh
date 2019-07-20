#!/bin/sh
rev="`git rev-parse --short HEAD`"
dirty=
if [ -n "`git status --porcelain --untracked=no`" ]; then
    dirty=+
fi
echo "${rev}${dirty}"
