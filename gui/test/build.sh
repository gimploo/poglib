#!/usr/bin/bash

rm -f a.out

#gcc -g test.c -lSDL2 -lGLEW -lGLU -lGL -lm -E 
gcc -g "$1" -lSDL2 -lGLEW -lGLU -lGL -lm && ./a.out 

if [ $? -eq 139 ] # opens gdb if seg faults
then
    echo run | gdb --silent ./a.out 
fi
