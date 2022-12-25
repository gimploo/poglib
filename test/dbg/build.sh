#!/bin/bash
# =============================================================================================
#                            -- LINUX BUILD SCRIPT FOR C PROJECTS --
# =============================================================================================


SRC_PATH="./src/main.c"
EXE_NAME="test"

CC="gcc"
FLAGS="-std=c11 -g -DDEBUG -W -Wall -Wextra -Wno-missing-braces -Wno-variadic-macros -rdynamic"
LINKERS="-lfreetype -lglfw -lSDL2 -lGLEW -lGLU -lGL -lm"
INCLUDES="-I/usr/include/freetype2 -I./lib/"



# =============================================================================================
#                            -- IMPLEMENTATION --
# =============================================================================================

red=$(tput setaf 1)
green=$(tput bold; tput setaf 2)
blue=$(tput bold; tput setaf 4)
reset=$(tput sgr0)

function setup_envirnoment {

    rm -rf core
    ulimit -c unlimited

}

function cleanup_envirnoment {

    ulimit -c 0
}


function compile_in_linux {

    local FILE_PATH="$1"

    $CC $FILE_PATH $FLAGS $INCLUDES $LINKERS -o ./bin/$EXE_NAME

}

function gdb_debug {

    if [ -f "core" ] 
    then
        echo -e "[*] ${blue}Core dump found, running with core dump ... ${reset}"
        gdb --core=core --silent --tui ./bin/"$EXE_NAME"
    else 
        echo -e "[*] ${blue}Core Dump not found, running without core dump ... ${reset}"
        gdb --silent --tui ./bin/"$EXE_NAME"
    fi

    return 0
}

function run_profiler {

    time ./bin/$EXE_NAME
}

function main {

    if [ "$1" == "help" ] 
    then
        echo -e "

        Usage: ./build.sh <tag>

        Builds the project using gcc or specified compiler

        Types of tags
        -------------
            help    - prints the usage
            run     - compiles and runs the program
            debug   - runs the executable in a debugger after compilation

        "
        exit 0
    fi


    echo " "

    local BIN_DIR="./bin"
    local LIB_DIR="./lib"

    # Cleaning bin directory
    if [ "$1" == "clean" ]
    then
        echo -e "[!] ${green}Cleaning $BIN_DIR directory${reset}"
        rm -rf $BIN_DIR 2> /dev/null
        echo -e "[!] ${green}Removing coredumps${reset}"
        rm -f core

        echo " "
        exit 0
    fi

    # Set environment
    echo -e "[!] ${green}Setting up environment${reset}"
    setup_envirnoment

    # Checking if bin directory is made
    if [ ! -d "$BIN_DIR" ] 
    then
        echo -e "[!] ${green}Creating directory ${reset}\`$BIN_DIR\`"
        mkdir bin/
    else 
        echo -e "[!] ${green}Found directory ${reset}\`$BIN_DIR\`" 
    fi

    # Checking if lib directory is made
    if [ ! -d "$LIB_DIR" ] 
    then
        echo -e "[!] ${green}Creating directory ${reset}\`$LIB_DIR\` (poglib)"
        if [ $(whoami) == "gokul" ]
        then
            ln -s /mnt/c/Users/User/OneDrive/Documents/projects/dev-libs lib
        else
            mkdir "$LIB_DIR"
            git clone https://github.com/gimploo/poglib.git $LIB_DIR/poglib > /dev/null
        fi
    else 
        echo -e "[!] ${green}Found directory ${reset}\`$LIB_DIR\`" 
    fi

    # Compiling source files
    echo -e "[*] ${blue}Compiling source file ...${reset}"

    if ! compile_in_linux $SRC_PATH ;
    then 
        echo -e "[!] ${red}Compilation Failed ${reset}"
        exit $LINENO
    else 
        echo -e "[!] ${green}Compilation Successfull ${reset}"
    fi

    # Running it through the debugger
    if [ "$1" == "debug" ]
    then
        echo -e "\n[*] ${blue}Running executable through debugger ...${reset}"
        gdb_debug
        echo -e "[!] ${green}Exiting debugger ${reset}"
        exit 0
    fi

    # Running executable
    if [ "$1" == "run" ]
    then
        echo -e "[*] ${blue}Running executable ...\n${reset}"
        run_profiler 
        if [ $? -eq 139 ]
        then
            echo -e "\n[!] ${red} Segmentation Fault Occurred ${reset}"
            echo -e "\n[*] ${blue}Running executable through debugger ...${reset}"
            gdb_debug
            echo -e "[!] ${green}Exiting debugger ${reset}"
        fi
    fi



    # Reseting environment
    echo -e "[!] ${green}Cleaning up environment ${reset}"
    cleanup_envirnoment

    echo " "
    exit 0
}

main "$1"
