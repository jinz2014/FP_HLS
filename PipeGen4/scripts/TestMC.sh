#!/bin/sh

######################################################
# A test script to test all the benchmarks listed
#
# minimum resource
# 3 port (1K + 2X)
# different combinations of port ordering
######################################################

case $1 in
  uuci) 
        echo "Testing UUCI"
        Benchmark="UUCI"
        XPconfig=( 2 )
        KPconfig=( 3 )
        ;;
  ordbur)
        echo "Testing ORDBUR"
        Benchmark="ORDBUR"
        XPconfig=( 3 )
        KPconfig=( 7 )
        ;;
  ordbbr)
        echo "Testing ORDBBR"
        Benchmark="ORDBBR"
        XPconfig=( 4 )
        KPconfig=( 10 )
        ;;
  ppbr)
        echo "Testing PPBR"
        Benchmark="PPBR"
        XPconfig=( 4 )
        KPconfig=( 9 )
        ;;
  random2)
        echo "Testing RANDOM2"
        Benchmark="RANDOM2"
        XPconfig=( 2 )
        KPconfig=( 2 )
        ;;
  all)
        echo "Testing ALL"
        Benchmark="UUCI ORDBUR ORDBBR PPBR"
        XPconfig=( 2 3 4  4 )
        KPconfig=( 3 7 10 9 )
       ;;
   *)  
        echo "Please specify a benchmark"
        exit
esac

case $2 in
  wbm)
  RegAlloc="wbm"
  ;;
  lea)
  RegAlloc="lea"
  ;;
  max)
  RegAlloc="max"
  ;;
  all)
  RegAlloc="wbm lea max"
  ;;
  *)
  echo "Please specify a register allocation method"
  exit
esac

case $3 in
  simple)
  test="PORT_PRIORITY"
  ;;
  full)
  test="PORT_ALL_PRIORITY"
  ;;
  *)
  echo "Please specify simple or full test"
  exit
esac

PROJ_PATH=`pwd`

# start from the first test circuit
i=0;
n="mc";

for bck in $Benchmark; do

	sed -i "s/BENCHMARK/$bck/g" Include/Schedule.h

  make TestSchedule1 arg1="$bck" arg2="$2" arg3="$test"

	if [ $? != 0 ]; then 
	 sed -i "s/$bck/BENCHMARK/g" Include/Schedule.h
   exit 1
  fi

	sed -i "s/$bck/BENCHMARK/g" Include/Schedule.h

  # upper->lower case
  name=`echo "$bck" | tr "A-Z" "a-z"`

  x=${XPconfig[$i]}
  k=${KPconfig[$i]}
  N=`expr $x + $k`

  # setup directory for benchmark $name
  mkdir -p $PROJ_PATH/Stat
  mkdir -p $PROJ_PATH/Log

  #for m in $RegAlloc; do
  #  for (( p = 1; p <= N; p++ )); do
  #    mkdir -p $PROJ_PATH/CSV/"$n"_"$m"/asap/"$name"_simple/Port"$p"
  #    mkdir -p $PROJ_PATH/CSV/"$n"_"$m"/asap/"$name"_all/Port"$p"
  #    mkdir -p $PROJ_PATH/CSV/"$n"_"$m"/alap/"$name"_simple/Port"$p"
  #    mkdir -p $PROJ_PATH/CSV/"$n"_"$m"/alap/"$name"_all/Port"$p"
  #    mkdir -p $PROJ_PATH/CSV/"$n"_"$m"/iter/"$name"_simple/Port"$p"
  #    mkdir -p $PROJ_PATH/CSV/"$n"_"$m"/iter/"$name"_all/Port"$p"
  #  done
  #done

  # generate HDL 
  echo "--------------------------------------------------"
  echo "*   Generating circuit $bck ..."
  echo "--------------------------------------------------"

  # call a shell script TestSchedule.sh
  ./TestSchedule.sh 1 $x $k $name $bck $2 "mc" "$test"

  echo "--------------------------------------------------"
  echo "*   Finish generating and testing circuit $bck    "
  echo "--------------------------------------------------"
  let i++
done

