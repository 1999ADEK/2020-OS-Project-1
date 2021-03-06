#!/bin/bash

touch "output/TIME_MEASUREMENT_stdout.txt" "output/TIME_MEASUREMENT_dmesg.txt"
sudo dmesg -c
sudo stdbuf -oL ./main < "testset/TIME_MEASUREMENT.txt" > "output/TIME_MEASUREMENT_stdout.txt"
dmesg | grep Project1 > "output/TIME_MEASUREMENT_dmesg.txt"

for policy in FIFO PSJF RR SJF
do
    for i in {1..5}
    do
        touch "output/${policy}_${i}_stdout.txt" "output/${policy}_${i}_dmesg.txt"
	sudo dmesg -c
	sudo stdbuf -oL ./main < "testset/${policy}_${i}.txt" > "output/${policy}_${i}_stdout.txt"
	dmesg | grep Project1 > "output/${policy}_${i}_dmesg.txt"
    done
done
