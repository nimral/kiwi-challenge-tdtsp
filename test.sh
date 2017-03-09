#!/bin/bash

date
hostname
for a in ../travelling-salesman/real_data/*txt ; do
    basename $a
    ( time ./main < $a ) 2>&1 | grep -E 'real|^[0-9]'
done
