#!/bin/sh

PLF_1pp=../PLF_ALL/PLF_1pp/export_hw/hw
PLF_2pp=../PLF_ALL/PLF_2pp/export_hw/hw
PLF_3pp=../PLF_ALL/PLF_3pp/export_hw/hw
PLF_4pp=../PLF_ALL/PLF_4pp/export_hw/hw
PLF_5pp=../PLF_ALL/PLF_5pp/export_hw/hw
PLF_6pp=../PLF_ALL/PLF_6pp/export_hw/hw
PLF_7pp=../PLF_ALL/PLF_7pp/export_hw/hw
PLF_8pp=../PLF_ALL/PLF_8pp/export_hw/hw

IP_1pp=../IP_ALL/PLF_1pp/export_hw/hw
IP_2pp=../IP_ALL/PLF_2pp/export_hw/hw
IP_3pp=../IP_ALL/PLF_3pp/export_hw/hw
IP_4pp=../IP_ALL/PLF_4pp/export_hw/hw
IP_5pp=../IP_ALL/PLF_5pp/export_hw/hw
IP_6pp=../IP_ALL/PLF_6pp/export_hw/hw
IP_7pp=../IP_ALL/PLF_7pp/export_hw/hw
IP_8pp=../IP_ALL/PLF_8pp/export_hw/hw

FIR_1pp=../FIR_ALL/PLF_1pp/export_hw/hw
FIR_2pp=../FIR_ALL/PLF_2pp/export_hw/hw
FIR_3pp=../FIR_ALL/PLF_3pp/export_hw/hw
FIR_4pp=../FIR_ALL/PLF_4pp/export_hw/hw
FIR_5pp=../FIR_ALL/PLF_5pp/export_hw/hw
FIR_6pp=../FIR_ALL/PLF_6pp/export_hw/hw
FIR_7pp=../FIR_ALL/PLF_7pp/export_hw/hw
FIR_8pp=../FIR_ALL/PLF_8pp/export_hw/hw


sed -i "s/addrwidth => 7/addrwidth => 1/g" $PLF_1pp/PLF_top.vhd
sed -i "s/addrwidth => 7/addrwidth => 1/g" $PLF_2pp/PLF_top.vhd
sed -i "s/addrwidth => 7/addrwidth => 1/g" $PLF_3pp/PLF_top.vhd
sed -i "s/addrwidth => 4/addrwidth => 1/g" $PLF_4pp/PLF_top.vhd
sed -i "s/addrwidth => 4/addrwidth => 1/g" $PLF_5pp/PLF_top.vhd
sed -i "s/addrwidth => 4/addrwidth => 1/g" $PLF_6pp/PLF_top.vhd
sed -i "s/addrwidth => 4/addrwidth => 1/g" $PLF_7pp/PLF_top.vhd
sed -i "s/addrwidth => 4/addrwidth => 1/g" $PLF_8pp/PLF_top.vhd

sed -i "s/addrwidth => 7/addrwidth => 1/g" $IP_1pp/PLF_top.vhd
sed -i "s/addrwidth => 7/addrwidth => 1/g" $IP_2pp/PLF_top.vhd
sed -i "s/addrwidth => 7/addrwidth => 1/g" $IP_3pp/PLF_top.vhd
sed -i "s/addrwidth => 4/addrwidth => 1/g" $IP_4pp/PLF_top.vhd
sed -i "s/addrwidth => 4/addrwidth => 1/g" $IP_5pp/PLF_top.vhd
sed -i "s/addrwidth => 4/addrwidth => 1/g" $IP_6pp/PLF_top.vhd
sed -i "s/addrwidth => 4/addrwidth => 1/g" $IP_7pp/PLF_top.vhd
sed -i "s/addrwidth => 4/addrwidth => 1/g" $IP_8pp/PLF_top.vhd

sed -i "s/addrwidth => 7/addrwidth => 1/g" $FIR_1pp/PLF_top.vhd
sed -i "s/addrwidth => 7/addrwidth => 1/g" $FIR_2pp/PLF_top.vhd
sed -i "s/addrwidth => 7/addrwidth => 1/g" $FIR_3pp/PLF_top.vhd
sed -i "s/addrwidth => 4/addrwidth => 1/g" $FIR_4pp/PLF_top.vhd
sed -i "s/addrwidth => 4/addrwidth => 1/g" $FIR_5pp/PLF_top.vhd
sed -i "s/addrwidth => 4/addrwidth => 1/g" $FIR_6pp/PLF_top.vhd
sed -i "s/addrwidth => 4/addrwidth => 1/g" $FIR_7pp/PLF_top.vhd
sed -i "s/addrwidth => 4/addrwidth => 1/g" $FIR_8pp/PLF_top.vhd
