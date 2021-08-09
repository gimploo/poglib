#!/usr/bin/bash

rm -f a.out
#gcc -g test.c -lSDL2 -lGLEW -lGLU -lGL -lm -E
gcc -g test.c -lSDL2 -lGLEW -lGLU -lGL -lm && ./a.out

if [ $? -eq 139 ] 
then 
    echo run | gdb a.out --silent
fi
