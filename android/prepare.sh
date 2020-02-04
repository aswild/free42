#!/bin/bash

set -eo pipefail
set -x

cd $(dirname $0)
pushd app/src/main/cpp
./link-files.sh
./build-intel-lib.sh
popd
