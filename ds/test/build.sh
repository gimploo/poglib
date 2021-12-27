#!/bin/bash


gcc -Wno-int-conversion $1 -g && ./a.out

if [ $? -eq 139 ]
then
    echo "run" | gdb ./a.out 
fi

