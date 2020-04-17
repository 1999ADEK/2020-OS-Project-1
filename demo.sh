#!/bin/bash

for filename in TIME_MEASUREMENT FIFO_1 PSJF_2 RR_3 SJF_4
do
    sudo dmesg -c
    sudo stdbuf -oL ./main < "testset/${filename}.txt" > "demo/${filename}_stdout.txt"
    dmesg | grep Project1 > "demo/${filename}_dmesg.txt"
done
