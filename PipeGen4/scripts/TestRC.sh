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
  ucti) 
        echo "Testing UCTI"
        Benchmark="UCTI"
        XPconfig=( 2 )
        KPconfig=( 3 )
        ;;
  uaii) 
        echo "Testing UAII"
        Benchmark="UAII"
        XPconfig=( 2 )
        KPconfig=( 3 )
        ;;
  uar) 
        echo "Testing UAR"
        Benchmark="UAR"
        XPconfig=( 3 )
        KPconfig=( 5 )
        ;;
  uucr) 
        echo "Testing UUCR"
        Benchmark="UUCR"
        XPconfig=( 3 )
        KPconfig=( 5 )
        ;;
  umi) 
        echo "Testing UMI"
        Benchmark="UMI"
        XPconfig=( 2 )
        KPconfig=( 4 )
        ;;
  umr) 
        echo "Testing UMR"
        Benchmark="UMR"
        XPconfig=( 3 )
        KPconfig=( 6 )
        ;;
  uctr) 
        echo "Testing UCTR"
        Benchmark="UCTR"
        XPconfig=( 3 )
        KPconfig=( 5 )
        ;;
  umai) 
        echo "Testing UMAI"
        Benchmark="UMAI"
        XPconfig=( 2 )
        KPconfig=( 4 )
        ;;
  ordbur)
        echo "Testing ORDBUR"
        Benchmark="ORDBUR"
        XPconfig=( 3 )
        KPconfig=( 7 )
        ;;
  ordubr)
        echo "Testing ORDUBR"
        Benchmark="ORDUBR"
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
  dct)
        echo "Testing DCT"
        Benchmark="DCT"
        XPconfig=( 32 )
        KPconfig=( 32 )
        ;;
  random2)
        echo "Testing RANDOM2"
        Benchmark="RANDOM2"
        XPconfig=( 2 )
        KPconfig=( 2 )
        ;;
  fir16)
        echo "Testing FIR16"
        Benchmark="FIR16"
        XPconfig=( 8 )
        KPconfig=( 8 )
        ;;
  pos)
        echo "Testing POS"
        Benchmark="POS"
        XPconfig=( 8 )
        KPconfig=( 8 )
        ;;
  power)
        echo "Testing POWER"
        Benchmark="POWER"
        XPconfig=( 1 )
        KPconfig=( 0 )
        ;;
  mux4)
        echo "Testing MX4"
        Benchmark="MX4"
        XPconfig=( 1 )
        KPconfig=( 0 )
        ;;
  mux2)
        echo "Testing MX2"
        Benchmark="MX2"
        XPconfig=( 2 )
        KPconfig=( 2 )
        ;;
  tmax)
        echo "Testing TMAX"
        Benchmark="TMAX"
        XPconfig=( 4 )
        KPconfig=( 4 )
        ;;
  log)
        echo "Testing LOG"
        Benchmark="LOG"
        XPconfig=( 1 )
        KPconfig=( 0 )
        ;;
  f2d)
        echo "Testing F2D"
        Benchmark="F2D"
        XPconfig=( 2 )
        KPconfig=( 2 )
        ;;
  mrbay)

        echo "Testing MRBAY"
        Benchmark="MRBAY"
        #XPconfig=( 32 )
        #KPconfig=( 10 )
        XPconfig=( 5 )
        KPconfig=( 5 )
        ;;
  plf)
        echo "Testing PLF"
        Benchmark="PLF"
        XPconfig=( 4 )
        KPconfig=( 4 )
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
n="rc";

for bck in $Benchmark; do

	sed -i "s/BENCHMARK/$bck/g" Include/Schedule.h

  make TestSchedule2 arg1="$bck" arg2="$2" arg3="$test"

	if [ $? != 0 ]; then 
	 sed -i "s/$bck/BENCHMARK/g" Include/Schedule.h
   exit 1
  fi

	sed -i "s/$bck/BENCHMARK/g" Include/Schedule.h

  # upper->lower case
  name=`echo "$bck" | tr "A-Z" "a-z"`

  # get logical port numbers
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
  #  done
  #done

  # generate HDL 
  echo "--------------------------------------------------"
  echo "*   Generating circuit $bck ..."
  echo "--------------------------------------------------"

  # call TestSchedule.sh
  #./TestSchedule.sh 2 $x $k $name $bck $2 "rc" "$test" 
  ./scripts/TestSchedule.sh 2 $x $k $name $bck $2 "rc" "$test" 

  echo "--------------------------------------------------"
  echo "*   Finish generating and testing circuit $bck    "
  echo "--------------------------------------------------"
  let i++
done

