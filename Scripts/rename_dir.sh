#!/bin/sh

PLF_1pp=../PLF_ALL/PLF_1pp
PLF_2pp=../PLF_ALL/PLF_2pp
PLF_3pp=../PLF_ALL/PLF_3pp
PLF_4pp=../PLF_ALL/PLF_4pp
PLF_5pp=../PLF_ALL/PLF_5pp
PLF_6pp=../PLF_ALL/PLF_6pp
PLF_7pp=../PLF_ALL/PLF_7pp
PLF_8pp=../PLF_ALL/PLF_8pp

IP_1pp=../IP_ALL/PLF_1pp
IP_2pp=../IP_ALL/PLF_2pp
IP_3pp=../IP_ALL/PLF_3pp
IP_4pp=../IP_ALL/PLF_4pp
IP_5pp=../IP_ALL/PLF_5pp
IP_6pp=../IP_ALL/PLF_6pp
IP_7pp=../IP_ALL/PLF_7pp
IP_8pp=../IP_ALL/PLF_8pp

FIR_1pp=../FIR_ALL/PLF_1pp
FIR_2pp=../FIR_ALL/PLF_2pp
FIR_3pp=../FIR_ALL/PLF_3pp
FIR_4pp=../FIR_ALL/PLF_4pp
FIR_5pp=../FIR_ALL/PLF_5pp
FIR_6pp=../FIR_ALL/PLF_6pp
FIR_7pp=../FIR_ALL/PLF_7pp
FIR_8pp=../FIR_ALL/PLF_8pp

mv $PLF_1pp/export_hw  $PLF_1pp/export_hw_pipe
mv $PLF_2pp/export_hw  $PLF_2pp/export_hw_pipe
mv $PLF_3pp/export_hw  $PLF_3pp/export_hw_pipe
mv $PLF_4pp/export_hw  $PLF_4pp/export_hw_pipe
mv $PLF_5pp/export_hw  $PLF_5pp/export_hw_pipe
mv $PLF_6pp/export_hw  $PLF_6pp/export_hw_pipe
mv $PLF_7pp/export_hw  $PLF_7pp/export_hw_pipe
mv $PLF_8pp/export_hw  $PLF_8pp/export_hw_pipe

mv $IP_1pp/export_hw  $IP_1pp/export_hw_pipe
mv $IP_2pp/export_hw  $IP_2pp/export_hw_pipe
mv $IP_3pp/export_hw  $IP_3pp/export_hw_pipe
mv $IP_4pp/export_hw  $IP_4pp/export_hw_pipe
mv $IP_5pp/export_hw  $IP_5pp/export_hw_pipe
mv $IP_6pp/export_hw  $IP_6pp/export_hw_pipe
mv $IP_7pp/export_hw  $IP_7pp/export_hw_pipe
mv $IP_8pp/export_hw  $IP_8pp/export_hw_pipe

mv $FIR_1pp/export_hw  $FIR_1pp/export_hw_pipe
mv $FIR_2pp/export_hw  $FIR_2pp/export_hw_pipe
mv $FIR_3pp/export_hw  $FIR_3pp/export_hw_pipe
mv $FIR_4pp/export_hw  $FIR_4pp/export_hw_pipe
mv $FIR_5pp/export_hw  $FIR_5pp/export_hw_pipe
mv $FIR_6pp/export_hw  $FIR_6pp/export_hw_pipe
mv $FIR_7pp/export_hw  $FIR_7pp/export_hw_pipe
mv $FIR_8pp/export_hw  $FIR_8pp/export_hw_pipe

sed -i "s/export_hw/export_hw_pipe/g" $PLF_1pp/export_hw_pipe/xilinx/plf.tcl
sed -i "s/export_hw/export_hw_pipe/g" $PLF_2pp/export_hw_pipe/xilinx/plf.tcl
sed -i "s/export_hw/export_hw_pipe/g" $PLF_3pp/export_hw_pipe/xilinx/plf.tcl
sed -i "s/export_hw/export_hw_pipe/g" $PLF_4pp/export_hw_pipe/xilinx/plf.tcl
sed -i "s/export_hw/export_hw_pipe/g" $PLF_5pp/export_hw_pipe/xilinx/plf.tcl
sed -i "s/export_hw/export_hw_pipe/g" $PLF_6pp/export_hw_pipe/xilinx/plf.tcl
sed -i "s/export_hw/export_hw_pipe/g" $PLF_7pp/export_hw_pipe/xilinx/plf.tcl
sed -i "s/export_hw/export_hw_pipe/g" $PLF_8pp/export_hw_pipe/xilinx/plf.tcl

sed -i "s/export_hw/export_hw_pipe/g" $IP_1pp/export_hw_pipe/xilinx/plf.tcl
sed -i "s/export_hw/export_hw_pipe/g" $IP_2pp/export_hw_pipe/xilinx/plf.tcl
sed -i "s/export_hw/export_hw_pipe/g" $IP_3pp/export_hw_pipe/xilinx/plf.tcl
sed -i "s/export_hw/export_hw_pipe/g" $IP_4pp/export_hw_pipe/xilinx/plf.tcl
sed -i "s/export_hw/export_hw_pipe/g" $IP_5pp/export_hw_pipe/xilinx/plf.tcl
sed -i "s/export_hw/export_hw_pipe/g" $IP_6pp/export_hw_pipe/xilinx/plf.tcl
sed -i "s/export_hw/export_hw_pipe/g" $IP_7pp/export_hw_pipe/xilinx/plf.tcl
sed -i "s/export_hw/export_hw_pipe/g" $IP_8pp/export_hw_pipe/xilinx/plf.tcl

sed -i "s/export_hw/export_hw_pipe/g" $FIR_1pp/export_hw_pipe/xilinx/plf.tcl
sed -i "s/export_hw/export_hw_pipe/g" $FIR_2pp/export_hw_pipe/xilinx/plf.tcl
sed -i "s/export_hw/export_hw_pipe/g" $FIR_3pp/export_hw_pipe/xilinx/plf.tcl
sed -i "s/export_hw/export_hw_pipe/g" $FIR_4pp/export_hw_pipe/xilinx/plf.tcl
sed -i "s/export_hw/export_hw_pipe/g" $FIR_5pp/export_hw_pipe/xilinx/plf.tcl
sed -i "s/export_hw/export_hw_pipe/g" $FIR_6pp/export_hw_pipe/xilinx/plf.tcl
sed -i "s/export_hw/export_hw_pipe/g" $FIR_7pp/export_hw_pipe/xilinx/plf.tcl
sed -i "s/export_hw/export_hw_pipe/g" $FIR_8pp/export_hw_pipe/xilinx/plf.tcl


