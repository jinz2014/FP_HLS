#!/bin/sh

PLF=../PLF_ALL
IP=../IP_ALL
FIR=../FIR_ALL

# rename export_hw to export_hw_pipe
n=1
while [ $n -le 8]; do
  mv $PLF/PLF_"$n"pp/export_hw  $PLF/PLF_"$n"pp/export_hw_pipe
  mv $IP/IP_"$n"pp/export_hw    $IP/IP_"$n"pp/export_hw_pipe
  mv $FIR/FIR_"$n"pp/export_hw  $FIR/FIR_"$n"pp/export_hw_pipe
  sed -i "s/export_hw/export_hw_pipe/g" $PLF/PLF_"$n"pp/export_hw_pipe/xilinx/plf.tcl
  sed -i "s/export_hw/export_hw_pipe/g" $IP/IP_"$n"pp/export_hw_pipe/xilinx/plf.tcl
  sed -i "s/export_hw/export_hw_pipe/g" $FIR/FIR_"$n"pp/export_hw_pipe/xilinx/plf.tcl
  let n++
done

