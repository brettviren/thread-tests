#!/bin/bash
for compiler in clang++
do
    for program in bigarray maparray
    do
	exe="${program}.${compiler}"
	$compiler -O3 -std=c++14 -o "$exe" ${program}.cpp -lrt
	echo "$exe"
	./$exe
    done
done

