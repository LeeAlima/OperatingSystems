#!/bin/bash
# Lee Alima 313467441
if [ "$#" != 0 ]
then
cd "$1"
fi
for i in *
do
if [[ -d $i ]]; then
echo "$i is a directory"
elif [[ -f $i ]]; then
echo "$i is a file"
fi
done
