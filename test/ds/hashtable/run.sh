#!/bin/bash

bear -- gcc -std=c11 -g3 -DDEBUG -O0 -Wall main.c -o a.out && 
./a.out

