#!/bin/bash
set -e
make prof
echo Profiling... It is going to take about a minute...
./prof < ../travelling-salesman/real_data/data_300.txt > /dev/null
gprof prof gmon.out > analysis.txt
rm gmon.out prof
less -S analysis.txt
