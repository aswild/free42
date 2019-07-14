#!/bin/bash

set -eo pipefail
set -x

cd $(dirname $0)
./prepare.sh
./gradlew assemble
