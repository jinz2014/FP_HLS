#-------------------------------------------------------------------
# args:
# $1: schedule name
# $2: px num
# $3: pk num
# $4: benchmark name uuci
# $5: benchmark name UUCI
# $6: which regsiter allocation method to run
# $7: schedule verbose name
# $8: test mode: simple or exhaustive
#-------------------------------------------------------------------

case $6 in
  wbm)
      #valgrind  --tool=memcheck --leak-check=full --show-reachable=yes \
      ./Test_Schedule$1_$5_WBM_$8 $2 $3 
      #| tee Log/$7_wbm_$4_$8.log
  ;;
  lea)
      #valgrind  --tool=memcheck --leak-check=full --show-reachable=yes \
      ./Test_Schedule$1_$5_LEA_$8 $2 $3 | tee Log/$7_lea_$4_$8.log
  ;;
  max)
      #valgrind  --tool=memcheck --leak-check=full --show-reachable=yes \
      ./Test_Schedule$1_$5_MAX_$8 $2 $3 | tee Log/$7_max_$4_$8.log
  ;;
  all)
      #valgrind  --tool=memcheck --leak-check=full --show-reachable=yes \
      ./Test_Schedule$1_$5_WBM_$8 $2 $3 | tee Log/$7_wbm_$4_$8.log

      #valgrind  --tool=memcheck --leak-check=full --show-reachable=yes \
      ./Test_Schedule$1_$5_LEA_$8 $2 $3 | tee Log/$7_lea_$4_$8.log

      #valgrind  --tool=memcheck --leak-check=full --show-reachable=yes \
      ./Test_Schedule$1_$5_MAX_$8 $2 $3 | tee Log/$7_max_$4_$8.log

  ;;
  *)
  echo "Please specify a register allocation method"
  exit
esac

