#!/bin/bash

CURRENT_DIR=`pwd`
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null && pwd )"

cd $SCRIPT_DIR
printf `git log -1 --format="%at" | xargs -I{} date -d @{} +%Y-%m-%d` && printf "+" && git describe --tags 2>/dev/null || git rev-parse --short HEAD 2>/dev/null
cd $CURRENT_DIR
