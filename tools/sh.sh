#! /bin/bash

if [[ $# -lt 2  ]]; then
	echo "Usage: ./sh.sh <label> <data_file>"
	exit
fi

label=$1
data=$2
fout="${label}_output.txt"
:> $fout && ../bin/construct_index $data ${label}.index 2>&1 | tee $fout
