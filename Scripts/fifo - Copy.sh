#!/bin/sh

list="PLF_ALL IP_ALL FIR_ALL"
for benchmark in $list; do
  echo benchmark is $benchmark
  sed -i "s/addrwidth => 7/addrwidth => 1/g" ../$benchmark/PLF_1pp/export_hw/hw/PLF_top.vhd
  sed -i "s/addrwidth => 7/addrwidth => 1/g" ../$benchmark/PLF_2pp/export_hw/hw/PLF_top.vhd
  sed -i "s/addrwidth => 7/addrwidth => 1/g" ../$benchmark/PLF_3pp/export_hw/hw/PLF_top.vhd
  sed -i "s/addrwidth => 4/addrwidth => 1/g" ../$benchmark/PLF_4pp/export_hw/hw/PLF_top.vhd
  sed -i "s/addrwidth => 4/addrwidth => 1/g" ../$benchmark/PLF_5pp/export_hw/hw/PLF_top.vhd
  sed -i "s/addrwidth => 4/addrwidth => 1/g" ../$benchmark/PLF_6pp/export_hw/hw/PLF_top.vhd
  sed -i "s/addrwidth => 4/addrwidth => 1/g" ../$benchmark/PLF_7pp/export_hw/hw/PLF_top.vhd
  sed -i "s/addrwidth => 4/addrwidth => 1/g" ../$benchmark/PLF_8pp/export_hw/hw/PLF_top.vhd

  sed -i "s/addrwidth => 7/addrwidth => 1/g" ../$benchmark/PLF_1pp/export_hw_pipe/hw/PLF_top.vhd
  sed -i "s/addrwidth => 7/addrwidth => 1/g" ../$benchmark/PLF_2pp/export_hw_pipe/hw/PLF_top.vhd
  sed -i "s/addrwidth => 7/addrwidth => 1/g" ../$benchmark/PLF_3pp/export_hw_pipe/hw/PLF_top.vhd
  sed -i "s/addrwidth => 4/addrwidth => 1/g" ../$benchmark/PLF_4pp/export_hw_pipe/hw/PLF_top.vhd
  sed -i "s/addrwidth => 4/addrwidth => 1/g" ../$benchmark/PLF_5pp/export_hw_pipe/hw/PLF_top.vhd
  sed -i "s/addrwidth => 4/addrwidth => 1/g" ../$benchmark/PLF_6pp/export_hw_pipe/hw/PLF_top.vhd
  sed -i "s/addrwidth => 4/addrwidth => 1/g" ../$benchmark/PLF_7pp/export_hw_pipe/hw/PLF_top.vhd
  sed -i "s/addrwidth => 4/addrwidth => 1/g" ../$benchmark/PLF_8pp/export_hw_pipe/hw/PLF_top.vhd
done
