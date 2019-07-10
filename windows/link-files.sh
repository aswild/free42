#!/bin/bash

if [[ "$MSYSTEM" != MINGW* ]]; then
    echo "This script must be run in a MSYS MinGW environment"
    exit 1
fi

files=(
    free42.h
    core_commands1.cc
    core_commands1.h
    core_commands2.cc
    core_commands2.h
    core_commands3.cc
    core_commands3.h
    core_commands4.cc
    core_commands4.h
    core_commands5.cc
    core_commands5.h
    core_commands6.cc
    core_commands6.h
    core_commands7.cc
    core_commands7.h
    core_display.cc
    core_display.h
    core_globals.cc
    core_globals.h
    core_helpers.cc
    core_helpers.h
    core_keydown.cc
    core_keydown.h
    core_linalg1.cc
    core_linalg1.h
    core_linalg2.cc
    core_linalg2.h
    core_math1.cc
    core_math1.h
    core_math2.cc
    core_math2.h
    core_main.cc
    core_main.h
    core_phloat.cc
    core_phloat.h
    core_sto_rcl.cc
    core_sto_rcl.h
    core_tables.cc
    core_tables.h
    core_variables.cc
    core_variables.h
    shell.h
    shell_loadimage.cc
    shell_loadimage.h
    shell_spool.cc
    shell_spool.h
    skin2cc.cc
    skin2cc.conf
    keymap2cc.cc
    bid_conf.h
    bid_functions.h
)

for f in "${files[@]}"; do
    ln -f ../common/$f $f
done
