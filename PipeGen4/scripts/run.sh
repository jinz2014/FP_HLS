#!/bin/sh

grep "BENCHMARK" ./Include/Schedule.h
if [ "$?" -ne "0" ]; then
  echo BENCHMARK not found in Include/Schedule.h
  exit 1
fi

case $1 in
  xilinx) 
  echo "Run test and prepare xilinx compile file"
  setup=1
  ;;
  sim)  
  echo "Run test only"
  setup=0
  ;;
  *)  
  echo "Please specify xilinx or sim"
  exit
esac

case $2 in
  uuci) 
        echo "testing uuci"
        Benchmark="uuci"
        ;;
  uctr) 
        echo "testing uctr"
        Benchmark="uctr"
        ;;
  ucti) 
        echo "testing ucti"
        Benchmark="ucti"
        ;;
  uaii) 
        echo "testing uaii"
        Benchmark="uaii"
        ;;
  uar) 
        echo "testing uar"
        Benchmark="uar"
        ;;
  uucr) 
        echo "testing uucr"
        Benchmark="uucr"
        ;;
  umi) 
        echo "testing umi"
        Benchmark="umi"
        ;;
  umr) 
        echo "testing umr"
        Benchmark="umr"
        ;;
  umai) 
        echo "testing umai"
        Benchmark="umai"
        ;;
  unii) 
        echo "testing unii"
        Benchmark="unii"
        ;;
  ordbur)
        echo "testing ordbur"
        Benchmark="ordbur"
        ;;
  ordubr)
        echo "testing ordubr"
        Benchmark="ordubr"
        ;;
  ordbbr)
        echo "testing ordbbr"
        Benchmark="ordbbr"
        ;;
  ppbr)
        echo "testing ppbr"
        Benchmark="ppbr"
        ;;
  dct)
        echo "testing dct"
        Benchmark="dct"
        ;;
  random2)
        echo "testing random2"
        Benchmark="random2"
        ;;
  fir16)
        echo "testing fir16"
        Benchmark="fir16"
        ;;
  pos)
        echo "testing pos"
        Benchmark="pos"
        ;;
  power)
        echo "testing power"
        Benchmark="power"
        ;;
  mux4)
        echo "testing mux4"
        Benchmark="mux4"
        ;;
  mux2)
        echo "testing mux2"
        Benchmark="mux2"
        ;;
  tmax)
        echo "testing tmax"
        Benchmark="tmax"
        ;;
  log)
        echo "testing log"
        Benchmark="log"
        ;;
  f2d)
        echo "testing f2d"
        Benchmark="f2d"
        ;;
  mrbay)
        echo "testing mrbay"
        Benchmark="mrbay"
        ;;
  plf)
        echo "testing plf"
        Benchmark="plf"
        ;;
  all)
        echo "testing all"
        Benchmark="uuci ordbur ordbbr ppbr"
       ;;
   *)
        echo "Please specify a benchmark"
        exit
esac

case $3 in
  min_res)
    echo "minimum resource schedule"
    cmd=TestRC.sh
    m="min_res"
    ;;
  min_dly)
    echo "minimum delay schedule"
    cmd=TestMC.sh
    m="min_dly"
    ;;
  *)
  echo "Please specify a scheduler min_res or min_dly"
  exit
esac


#RegAlloc="lea"
#RegAlloc="max"
RegAlloc="wbm"
#RegAlloc="max lea"

PortPriority="simple"
#PortPriority="full"
#PortPriority="simple full"

#--------------------------------------------------------------------
# Modelsim simulation doens't need it as the port number is specified
# in RC_Simple_Schedule.c or MC_Simple_Schedule.c
#--------------------------------------------------------------------
Schedule="ASAP4"
#Schedule="ASAP1 ASAP2"
#Schedule="ASAP1 ASAP2 ASAP3 ASAP4"
#Schedule="ALAP1 ALAP2"
#Schedule="ALAP3 ALAP4"


PROJ_PATH=`pwd`

for b in $Benchmark; do
  for r in $RegAlloc; do
    for p in $PortPriority; do

      # all the files will be generated after executing the command
      #./$cmd $b $r $p
      ./scripts/$cmd $b $r $p

      if [ $setup -eq 1 ]; then
        #-------------------------------------------------------
        # Xilinx Setting begins
        #-------------------------------------------------------
        for s in $Schedule; do

          mkdir -p $PROJ_PATH/Xilinx/$b/$m/$s/$r/$p

          # set xilinx project directory
          XPROJ_PATH=$PROJ_PATH/Xilinx/$b/$m/$s/$r/$p

          # set source HDL file path
          HDL_SRC_PATH=$PROJ_PATH/Modelsim/$b/"$s"_"$b"

          # set HDL file name
          FILE="$s"_"$b"_HDL_"$r"_"$p"
          echo "debug: HDL file name is $FILE"

          #rename ASAP1_uuci_HDL.v to ASAP1_uuci_HDL_max_simple.v
          cp $HDL_SRC_PATH/"$s"_"$b"_HDL.v $XPROJ_PATH/$FILE.v

          # get a copy of proj.tcl 
          cp $PROJ_PATH/Xilinx/proj.tcl $PROJ_PATH/
          echo "debug: copy project script"

          # uuci
          sed -i "s/Benchmark/$b/g" $PROJ_PATH/proj.tcl

          # ASAP1
          #sed -i "s/Schedule/$s/g" $PROJ_PATH/proj.tcl

          # register allocation
          #sed -i "s/RegAlloc/$r/g" $PROJ_PATH/proj.tcl

          # port priority simple/full
          #sed -i "s/Priority/$p/g" $PROJ_PATH/proj.tcl

          # ASAP1_uuci_HDL_max_simple.v
          sed -i "s/Circuit/$FILE/g" $PROJ_PATH/proj.tcl

          # Full XST path to the xilinx project directory
          sed -i "s?WORK_DIR?$XPROJ_PATH?g" $PROJ_PATH/proj.tcl

          #move it to the xilinx project directory
          mv $PROJ_PATH/proj.tcl $XPROJ_PATH/

          echo "debug: start xtclsh..."
          cd $XPROJ_PATH
          pwd
          xtclsh proj.tcl rebuild_project &

          cd $PROJ_PATH

        #-------------------------------------------------------
        # Xilinx Setting ends
        #-------------------------------------------------------
        done
      fi
    done
  done
done
