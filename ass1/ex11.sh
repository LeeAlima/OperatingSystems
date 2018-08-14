#!/bin/bash
# Lee Alima 313467441
if [ "$#" != 0 ]
then
cd "$1"
fi
num=$(find . -maxdepth 1 -name "*.txt" -type f | wc -l)
echo Number of files in the directory that end with .txt is $num
