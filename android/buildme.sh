#!/bin/bash

set -eo pipefail
set -x

cd $(dirname $0)
pushd app/src/main/cpp
./link-files-release.sh
./build-intel-lib.sh
popd

./gradlew assembleRelease
