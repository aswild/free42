#!/bin/bash

set -eo pipefail
set -x

cd $(dirname $0)
pushd app/src/main/cpp
./link-files-release.sh
./build-intel-lib.sh
popd

sed -e "s/versionCode .*/versionCode $(<version.code)/" \
    -e "s/versionName .*/versionName \"$(<../VERSION)\"/" \
    -i app/build.gradle

./gradlew assemble
