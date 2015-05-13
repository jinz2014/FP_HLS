#!/bin/sh

#Benchmark="uuci ordbur ordbbr ppbr"
#Benchmark="ordbur ordbbr ppbr"
Benchmark="ordbbr"

lapse=30
time=-30

for b in $Benchmark; do
  echo "./run.sh  xilinx $b min_dly"  | at now + `expr $time + 1 \* $lapse` minutes
  echo "./run_1.sh xilinx $b min_dly" | at now + `expr $time + 2 \* $lapse` minutes
  echo "./run1.sh xilinx $b min_dly"  | at now + `expr $time + 3 \* $lapse` minutes
  echo "./run1_1.sh xilinx $b min_dly"| at now + `expr $time + 4 \* $lapse` minutes
  time=`expr $time + 4 \* $lapse`
done
