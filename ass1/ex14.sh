#!/bin/bash
# Lee Alima 313467441
number=0
while read line
do
	if echo $line | grep -q "$1"
		then
		echo $line
		stringArray=($line)
		((number+=${stringArray[2]}))
	fi
done < "$2"
echo Total balance: $number

