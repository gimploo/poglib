#!/bin/bash

red=$(tput setaf 1)
green=$(tput bold; tput setaf 2)
blue=$(tput bold; tput setaf 4)
reset=$(tput sgr0)

# =================================================================
# CHANGE THIS _
#              |
#              v

SRC_PATH="main.c"
EXE_PATH="./test"
BIN_DIR="./"

#=====================================================================


function setup_envirnoment {

    rm -rf core
    ulimit -c unlimited

}

function cleanup_envirnoment {

    ulimit -c 0
}


function compile_in_linux {

    local CC="gcc"
    local FLAGS="-std=c11 -g -W -Wall -Wextra -Wno-missing-braces -Wno-variadic-macros"
    local LINKERS="-lSDL2 -lGLEW -lGLU -lGL -lm"

    local FILE_PATH="$1"

    $CC $FILE_PATH $FLAGS $LINKERS -o $EXE_PATH

}

function gdb_debug {

    local TMP=gdb_debug_txt_file

    if [ -f "core" ] 
    then
        echo -e "[*] ${blue}Core dump found, running with core dump ... ${reset}"
        gdb --core=core --silent "$EXE_PATH"
    else 
        echo -e "[*] ${blue}Core Dump not found, running without core dump ... ${reset}"
        gdb --silent "$EXE_PATH"
    fi

    return 0
}

function run_profiler {

    time $EXE_PATH
}

function main {

    # Cleaning bin directory
    if [ "$1" == "clean" ]
    then
        echo -e "[!] ${green}Cleaning bin/ directory${reset}"
        rm -rf $EXE_PATH
        echo -e "[!] ${green}Removing coredumps${reset}\n"
        rm -f core
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

    # Compiling source files
    echo -e "[*] ${blue}Compiling source file ...${reset}\n"

    if ! compile_in_linux $SRC_PATH ;
    then 
        echo -e "[!] ${red}Compilation Failed ${reset}"
        exit $LINENO
    else 
        echo -e "\n[!] ${green}Compilation Successfull ${reset}"
    fi


    # Running executable
    echo -e "[*] ${blue}Running executable ...\n${reset}"
    run_profiler 


    # If seg faults run it throught debugger
    if [ $? -ne 0 ]
    then
        echo -e "\n[!] ${red} Segmentation Fault Occurred ${reset}"
        echo -e "\n[*] ${blue}Running executable through debugger ...${reset}"
        gdb_debug
        echo -e "[!] ${green}Exiting debugger ${reset}"
    fi


    # Reseting environment
    echo -e "[!] ${green}Cleaning up environment ${reset}"
    cleanup_envirnoment


    echo -e ""
    exit 0
}

main "$1"
