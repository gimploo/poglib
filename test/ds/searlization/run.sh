#!/bin/bash

gcc -g loader.c -o loader &&
gcc -g saver.c -o saver &&
valgrind ./saver &&
valgrind ./loader
