#!/usr/bin/bash

rm -f a.out
gcc -g test.c -lSDL2 -lGLEW -lGLU -lGL -lm && ./a.out
