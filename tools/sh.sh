#! /bin/bash

if [[ $# -lt 2  ]]; then
	echo "Usage: ./sh.sh <label> <data_file> [opts]"
	exit
fi

label=$1
data=$2
opts=$3
fout="${label}_output.txt"
:> $fout && ../bin/construct_index $data ${label}.index $opts 2>&1 | tee $fout
