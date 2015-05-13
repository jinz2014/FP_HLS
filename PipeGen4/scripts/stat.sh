#!/bin/sh

# Specify a search sequence
Benchmark="uuci ordbur ordbbr ppbr random2"
#Benchmark="random2"

case $1 in
  min_res)
    echo "Collecting stats of minimum resource schedule"
    ;;
  min_dly)
    echo "Collecting stats of minimum delay schedule"
    ;;
  *)
  echo "Please specify a scheduler min_res or min_dly"
  exit
esac

# Get Frequency info
grep -r -m 1 "MHz" --include="*.syr" Xilinx/* > freq.txt

# Get Registers/LUTs info
grep -r -m 2 "out of" --include="*.mrp" Xilinx/*  > resource.txt


for b in $Benchmark; do
  rm Xilinx/"$b"_freq.txt Xilinx/"$b"_resource.txt
  for (( i = 1 ; i <= 4; i++ )); do
    #grep "$b/$1/ASAP$i" freq.txt >> Xilinx/"$b"_freq.txt
    #grep "$b/$1/ASAP$i" resource.txt >> Xilinx/"$b"_resource.txt
    grep "$b/$1/$2$i" freq.txt >> Xilinx/"$b"_freq.txt
    grep "$b/$1/$2$i" resource.txt >> Xilinx/"$b"_resource.txt
  done
done

#rm freq.txt resource.txt
