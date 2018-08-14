#!/bin/bash
# Lee Alima 313467441
if [ "$#" != 1 ]
	then
		echo "error: only one argument is allowed"
else
	if [ ! -e "$1" ]
		then
			echo "error: there is no such file"
	else
		mkdir -p safe_rm_dir
		cp "$1" safe_rm_dir
		rm "$1"
		echo "done!"
	fi
fi

