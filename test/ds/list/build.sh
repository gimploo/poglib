#!/bin/bash

POGLIB_ROOT=$(dirname "$0")/../../..
POGLIB_DIR=$(cd "$POGLIB_ROOT" && pwd)
gcc -I"$POGLIB_DIR"/.. -I"$POGLIB_DIR" -std=c11 -g -g3 -O0 -Wall -Wextra -Wno-unused-parameter -Wno-unused-function -Wno-unused-variable -Wno-unused-but-set-variable -Wno-sign-compare -Wno-type-limits -Wno-format -pthread main.c -o a.out && echo "Compiled successfully"
