#!/bin/bash
SRC_PATH="./src/main.c"
EXE_NAME="ecs_save_load_test"

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
POGLIB_DIR="$(cd "$SCRIPT_DIR/../../.." && pwd)"
PARENT_DIR="$(dirname "$POGLIB_DIR")"
JOLT_DIR="$POGLIB_DIR/external/joltc/lib/linux/debug"

CC="clang"
FLAGS="-std=c11 -g -O0 -DDEBUG -W -Wall -Wextra -Wno-missing-braces -Wincompatible-pointer-types-discards-qualifiers -Wno-variadic-macros"
LINKERS="-lfreetype -lSDL2 -lGLEW -lGLU -lGL -lm -lassimp -pthread -ldl -L$JOLT_DIR -ljoltcd -lJoltd -lstdc++"
INCLUDES="-I/usr/include/freetype2 -I$PARENT_DIR -I/usr/include/SDL2"

red=$(tput setaf 1)
green=$(tput bold; tput setaf 2)
blue=$(tput bold; tput setaf 4)
reset=$(tput sgr0)

mkdir -p bin
echo -e "[*] ${blue}Compiling test...${reset}"
if $CC $SRC_PATH $FLAGS $INCLUDES $LINKERS -o ./bin/$EXE_NAME 2>&1; then
    echo -e "[!] ${green}Compilation Success${reset}"
    if [ "$1" != "compile" ]; then
        echo -e "[*] ${blue}Running test...${reset}"
        ./bin/$EXE_NAME
    fi
else
    echo -e "[!] ${red}Compilation Failed${reset}"
    exit 1
fi
