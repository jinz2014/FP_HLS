#!/bin/sh

ROOT=../My_Tool
XPROJ=../My_Tool/Xilinx
#BENCHMARK="pos sop fir16"
BENCHMARK="ordbbr"
SCHEDULE="ASAP ALAP List"

for benchmark in $BENCHMARK; do
  n=1
  while [ $n -le 8 ]; do
   for schedule in $SCHEDULE; do
     echo info: $benchmark $schedule$n
     cp proj.tcl $XPROJ/$benchmark/"$schedule""$n"_"$benchmark"/
     sed -i "s/Schedule/$schedule$n/g" \
     $XPROJ/$benchmark/"$schedule""$n"_"$benchmark"/proj.tcl
     sed -i "s/Benchmark/$benchmark/g" \
     $XPROJ/$benchmark/"$schedule""$n"_"$benchmark"/proj.tcl

     echo checking rom and RegisterFile 
     grep -q "rom" $ROOT/Modelsim/"$schedule""$n"_"$benchmark"/"$schedule""$n"_"$benchmark"_HDL.v
     if [ "$?" -ne "0" ]; then
      sed -i "/_ROM.v/ d" \
      $XPROJ/$benchmark/"$schedule""$n"_"$benchmark"/proj.tcl
     fi

     grep -q "RegisterFile" $ROOT/Modelsim/"$schedule""$n"_"$benchmark"/"$schedule""$n"_"$benchmark"_HDL.v
     if [ "$?" -ne "0" ]; then
      sed -i "/RegFile.v/ d" \
      $XPROJ/$benchmark/"$schedule""$n"_"$benchmark"/proj.tcl
     fi
   done
   let n++
  done
done
