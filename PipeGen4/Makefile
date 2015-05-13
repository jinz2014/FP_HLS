PROJ_PATH=/share/jinz/Xilinx_synthesis/MyResearch/PipeGen4

SIM_PATH=$(PROJ_PATH)/Modelsim
LIB_PATH=$(PROJ_PATH)/vlib
TEST_DATA_PATH=$(PROJ_PATH)/TestData

#==============================================================================
# Run RC_Simple_Schedule  *
# Run RC_Schedule.c
#==============================================================================

# Ishimori's Port-ordering Resource-constraint Schedule
VPATH = Queue Backend Schedule Port Include Utils Benchmark libhungarian
TestSchedule2 : queue.h Schedule.h PortSchedule.h WBM.h\
  queue.c GenerateDataPathPort.c \
  GenerateTestData.c GenerateDebugData.c GenerateFloatTestBench.c FloatHelper.c \
  GenerateFloatHDL.c  \
  GenerateControlSignals.c GenerateControlPath.c \
  GenerateOutputAssign.c AddMuxRegister.c GetTotalRegNum.c GetTotalMuxInputs.c GetHDLFanout.c \
  MuxAlloc.c LeftEdge.c RegAlloc.c \
  hungarian.c WBM_Helper.c CreateWBMClusters.c  CreateWBMClustersMax.c \
  ScheduleHelper.c PortHelper.c \
  Enumerate.c \
  PortDirectedSchedule.c RC_Simple_Schedule.c all_perm.c  all_perm_rep.c SetPortPriority.c \
  ASAP_rc.c ALAP_rc.c ListHelper.c CheckConflict.c PrintStatistic.c util.c \
  TB.do vlib/*.v Benchmark/*.config Benchmark/*.txt Include/*.config  Include/*.dfg 
ifeq ($(arg2),lea)
	gcc -g -DRC_SCHEDULE -DLEA -D$(arg3) -I$(PROJ_PATH)/Queue -I$(PROJ_PATH)/Include \
  -I$(PROJ_PATH)/Benchmark -I$(PROJ_PATH)/libhungarian $(filter-out %.h %.v %.txt %.config %.dfg %.do, $^)\
  -o Test_Schedule2_$(arg1)_LEA_$(arg3) -lm
endif
ifeq ($(arg2),wbm) 
	gcc -g -DRC_SCHEDULE -DWBM -D$(arg3) -I$(PROJ_PATH)/Queue -I$(PROJ_PATH)/Include \
  -I$(PROJ_PATH)/Benchmark -I$(PROJ_PATH)/libhungarian $(filter-out %.h %.v %.txt %.config %.dfg %.do, $^) \
  -o Test_Schedule2_$(arg1)_WBM_$(arg3) -lm
endif
ifeq ($(arg2),max) 
	gcc -g -DRC_SCHEDULE -D$(arg3) -I$(PROJ_PATH)/Queue -I$(PROJ_PATH)/Include \
  -I$(PROJ_PATH)/Benchmark -I$(PROJ_PATH)/libhungarian $(filter-out %.h %.v %.txt %.config %.dfg %.do, $^) \
  -o Test_Schedule2_$(arg1)_MAX_$(arg3) -lm
endif
ifeq ($(arg2),all) 
	gcc -g -DRC_SCHEDULE -DLEA -D$(arg3) -I$(PROJ_PATH)/Queue -I$(PROJ_PATH)/Include \
  -I$(PROJ_PATH)/Benchmark $(filter-out %.h %.v %.txt %.config %.dfg %.do, $^)\
  -o Test_Schedule2_$(arg1)_LEA_$(arg3) -lm
	gcc -g -DRC_SCHEDULE -DWBM -D$(arg3) -I$(PROJ_PATH)/Queue -I$(PROJ_PATH)/Include \
  -I$(PROJ_PATH)/Benchmark $(filter-out %.h %.v %.txt %.config %.dfg %.do, $^) \
  -o Test_Schedule2_$(arg1)_WBM_$(arg3) -lm
	gcc -g -DRC_SCHEDULE -D$(arg3) -I$(PROJ_PATH)/Queue -I$(PROJ_PATH)/Include -I$(PROJ_PATH)/Benchmark \
  $(filter-out %.h %.v %.txt %.config %.dfg %.do, $^)\
  -o Test_Schedule2_$(arg1)_MAX_$(arg3) -lm
endif

#==============================================================================


#==============================================================================
# Ishimori's Port-ordering Resource-relaxed Schedule
#
# Run MC_Simple_Schedule  *
# Run MC_Schedule.c
#==============================================================================

# Port-ordering schedule with relaxed resource constraint)
VPATH = Queue Backend Schedule Port Include Utils Benchmark
TestSchedule1 : queue.h Schedule.h PortSchedule.h queue.c GenerateDataPathPort.c GenerateTestData.c GenerateDebugData.c GenerateFloatTestBench.c FloatHelper.c GenerateFloatHDL.c GenerateControlSignals.c GenerateControlPath.c MuxAlloc.c LeftEdge.c RegAlloc.c ScheduleHelper.c PortHelper.c QueueDirectedSchedule.c MC_Simple_Schedule.c all_perm.c all_perm_rep.c SetPortPriority.c ASAP_mc.c ALAP_mc.c ListHelper.c hill_climbing.c CheckConflict.c util.c  PrintStatistic.c TB.do vlib/*.v Benchmark/*.config Benchmark/*.txt Include/*.config  Include/*.dfg 
ifeq ($(arg2),lea)
	gcc -g -DMC_SCHEDULE -DLEA -D$(arg3) -I$(PROJ_PATH)/Queue -I$(PROJ_PATH)/Include \
  -I$(PROJ_PATH)/Benchmark $(filter-out %.h %.v %.txt %.config %.dfg %.do, $^)\
  -o Test_Schedule1_$(arg1)_LEA_$(arg3) -lm
endif
ifeq ($(arg2),wbm) 
	gcc -g -DMC_SCHEDULE -DWBM -D$(arg3) -I$(PROJ_PATH)/Queue -I$(PROJ_PATH)/Include \
  -I$(PROJ_PATH)/Benchmark $(filter-out %.h %.v %.txt %.config %.dfg %.do, $^) \
  -o Test_Schedule1_$(arg1)_WBM_$(arg3) -lm
endif
ifeq ($(arg2),max) 
	gcc -g -DMC_SCHEDULE -D$(arg3) -I$(PROJ_PATH)/Queue -I$(PROJ_PATH)/Include \
  -I$(PROJ_PATH)/Benchmark $(filter-out %.h %.v %.txt %.config %.dfg %.do, $^)\
  -o Test_Schedule1_$(arg1)_MAX_$(arg3) -lm
endif
ifeq ($(arg2),all) 
	gcc -g -DMC_SCHEDULE -DLEA -D$(arg3) -I$(PROJ_PATH)/Queue -I$(PROJ_PATH)/Include \
  -I$(PROJ_PATH)/Benchmark $(filter-out %.h %.v %.txt %.config %.dfg %.do, $^)\
  -o Test_Schedule1_$(arg1)_LEA_$(arg3) -lm
	gcc -g -DMC_SCHEDULE -DWBM -D$(arg3) -I$(PROJ_PATH)/Queue -I$(PROJ_PATH)/Include \
  -I$(PROJ_PATH)/Benchmark $(filter-out %.h %.v %.txt %.config %.dfg %.do, $^) \
  -o Test_Schedule1_$(arg1)_WBM_$(arg3) -lm
	gcc -g -DMC_SCHEDULE -D$(arg3) -I$(PROJ_PATH)/Queue -I$(PROJ_PATH)/Include -I$(PROJ_PATH)/Benchmark \
  $(filter-out %.h %.v %.txt %.config %.dfg %.do, $^)\
  -o Test_Schedule1_$(arg1)_MAX_$(arg3) -lm
endif

#==============================================================================


fds: fds.c Schedule.h util.c HAL.txt 
	gcc -g -o fds fds.c util.c

its: Iterative_Scheduling.c ListHelper.c Schedule.h util.c HAL.txt 
	gcc -g -o its Iterative_Scheduling.c ListHelper.c util.c  

its1: Iterative_Scheduling1.c ListHelper.c Schedule.h util.c HAL.txt 
	gcc -g -o its1 Iterative_Scheduling1.c ListHelper.c util.c  


test_all: 
	make test_pos;     \
	make test_sop;     \
	make test_fir16;   \
	make test_ratelaw; \
	make test_ratelaw1;\
	make test_sample1; \
	make test_uur;     \
	make test_uctr;    \
	make test_umar;    \
	make test_uhmr;	   \
	make test_umr;     \
	make test_ucti;    \
	make test_unii;    \
	make test_umai;    \
	make test_uai;     \
	make test_umi;     \
	make test_uuci;    \
	make test_uucr;    \
	make test_uaii;    \
	make test_ucii;    \
	make test_ucir;    \
	make test_uar;     \
	make test_ordbbr;  \
	make test_ordbur;  \
	make test_ordubr;  \
	make test_ppbr;    \
	make test_random1; \
	make test_random2; 

test_sop:
	sed -i "s/benchmark/sop/g" Schedule.c ScheduleHelper.c
	make Test ;\
	./Test > test_sop.log; \
	make ASAP1_sop; make ASAP2_sop; make ASAP3_sop; make ASAP4_sop; \
	make ASAP5_sop; make ASAP6_sop; make ASAP7_sop; make ASAP8_sop; \
	make ALAP1_sop; make ALAP2_sop; make ALAP3_sop; make ALAP4_sop; \
	make ALAP5_sop; make ALAP6_sop; make ALAP7_sop; make ALAP8_sop;
	sed -i "s/sop/benchmark/g" Schedule.c ScheduleHelper.c

test_pos:
	make ASAP1_pos; make ASAP2_pos; make ASAP3_pos; make ASAP4_pos; \
	make ASAP5_pos; make ASAP6_pos; make ASAP7_pos; make ASAP8_pos; \
	make ASAP9_pos; make ASAP10_pos; make ASAP11_pos; make ASAP12_pos; \
	make ASAP13_pos; make ASAP14_pos; make ASAP15_pos; make ASAP16_pos; \
	make ALAP1_pos; make ALAP2_pos; make ALAP3_pos; make ALAP4_pos; \
	make ALAP5_pos; make ALAP6_pos; make ALAP7_pos; make ALAP8_pos; \
	make ALAP9_pos; make ALAP10_pos; make ALAP11_pos; make ALAP12_pos; \
	make ALAP13_pos; make ALAP14_pos; make ALAP15_pos; make ALAP16_pos; 

test_fir16:
	make ASAP1_fir16; make ASAP2_fir16; make ASAP3_fir16; make ASAP4_fir16; \
	make ASAP5_fir16; make ASAP6_fir16; make ASAP7_fir16; make ASAP8_fir16; \
	make ASAP9_fir16; make ASAP10_fir16; make ASAP11_fir16; make ASAP12_fir16; \
	make ASAP13_fir16; make ASAP14_fir16; make ASAP15_fir16; make ASAP16_fir16; \
	make ALAP1_fir16; make ALAP2_fir16; make ALAP3_fir16; make ALAP4_fir16; \
	make ALAP5_fir16; make ALAP6_fir16; make ALAP7_fir16; make ALAP8_fir16; \
	make ALAP9_fir16; make ALAP10_fir16; make ALAP11_fir16; make ALAP12_fir16; \
	make ALAP13_fir16; make ALAP14_fir16; make ALAP15_fir16; make ALAP16_fir16; 

test_ratelaw:
	make ASAP1_ratelaw; make ASAP2_ratelaw; make ASAP3_ratelaw;\
	make ASAP4_ratelaw; make ASAP5_ratelaw; \
	make ALAP1_ratelaw; make ALAP2_ratelaw; make ALAP3_ratelaw;\
	make ALAP4_ratelaw; make ALAP5_ratelaw; \
	make List1_ratelaw; make List2_ratelaw; make List3_ratelaw;\
	make List4_ratelaw; make List5_ratelaw; 


test_ratelaw1:
	make ASAP1_ratelaw1; make ASAP2_ratelaw1; make ASAP3_ratelaw1;
	make ASAP4_ratelaw1; make ASAP5_ratelaw1; 
	make ALAP1_ratelaw1; make ALAP2_ratelaw1; make ALAP3_ratelaw1;
	make ALAP4_ratelaw1; make ALAP5_ratelaw1; 

test_sample:
	make ASAP1_sample; make ASAP2_sample;


test_sample1:
	make ASAP1_sample1; make ASAP2_sample1;\
	make ASAP3_sample1; make ASAP4_sample1;\
	make ALAP1_sample1; make ALAP2_sample1;\
	make ALAP3_sample1; make ALAP4_sample1;

test_uur:
	make ASAP1_uur; make ASAP2_uur; make ASAP3_uur; make ASAP4_uur; \
	make ASAP5_uur; make ASAP6_uur; \
	make ALAP1_uur; make ALAP2_uur; make ALAP3_uur; make ALAP4_uur; \
	make ALAP5_uur; make ALAP6_uur; \
	make List1_uur; make List2_uur; make List3_uur; make List4_uur; \
	make List5_uur; make List6_uur; 

test_ucti:
	make ASAP1_ucti; make ASAP2_ucti; make ASAP3_ucti; make ASAP4_ucti; \
	make ASAP5_ucti; \
	make ALAP1_ucti; make ALAP2_ucti; make ALAP3_ucti; make ALAP4_ucti; \
	make ALAP5_ucti; \
	make List1_ucti; make List2_ucti; make List3_ucti; make List4_ucti; \
	make List5_ucti; \
	make Iter1_ucti; make Iter2_ucti; make Iter3_ucti; make Iter4_ucti; \
	make Iter5_ucti; 

test_unii:
	make ASAP1_unii; make ASAP2_unii; make ASAP3_unii; make ASAP4_unii; \
	make ASAP5_unii; \
	make ALAP1_unii; make ALAP2_unii; make ALAP3_unii; make ALAP4_unii; \
	make ALAP5_unii; \
	make List1_unii; make List2_unii; make List3_unii; make List4_unii; \
	make List5_unii; 

test_uuci:
	make ASAP1_uuci; make ASAP2_uuci; make ASAP3_uuci; make ASAP4_uuci; \
	make ASAP5_uuci; \
	make ALAP1_uuci; make ALAP2_uuci; make ALAP3_uuci; make ALAP4_uuci; \
	make ALAP5_uuci; \
	make List1_uuci; make List2_uuci; make List3_uuci; make List4_uuci; \
	make List5_uuci;

test_uucr:
	make ASAP1_uucr; make ASAP2_uucr; make ASAP3_uucr; make ASAP4_uucr; \
	make ASAP5_uucr; make ASAP6_uucr; make ASAP7_uucr; make ASAP8_uucr; \
	make ALAP1_uucr; make ALAP2_uucr; make ALAP3_uucr; make ALAP4_uucr; \
	make ALAP5_uucr; make ALAP6_uucr; make ALAP7_uucr; make ALAP8_uucr; \
	make List1_uucr; make List2_uucr; make List3_uucr; make List4_uucr; \
	make List5_uucr; make List6_uucr; make List7_uucr; make List8_uucr;

test_uaii:
	make ASAP1_uaii; make ASAP2_uaii; make ASAP3_uaii; make ASAP4_uaii; \
	make ASAP5_uaii; \
	make ALAP1_uaii; make ALAP2_uaii; make ALAP3_uaii; make ALAP4_uaii; \
	make ALAP5_uaii; \
	make List1_uaii; make List2_uaii; make List3_uaii; make List4_uaii; \
	make List5_uaii; \

test_ucii:
	make ASAP1_ucii; make ASAP2_ucii; make ASAP3_ucii; make ASAP4_ucii; \
	make ASAP5_ucii; \
	make ALAP1_ucii; make ALAP2_ucii; make ALAP3_ucii; make ALAP4_ucii; \
	make ALAP5_ucii; \
	make List1_ucii; make List2_ucii; make List3_ucii; make List4_ucii; \
	make List5_ucii; \

test_ucir:
	make ASAP1_ucir; make ASAP2_ucir; make ASAP3_ucir; make ASAP4_ucir; \
	make ASAP5_ucir; make ASAP6_ucir; make ASAP7_ucir; make ASAP8_ucir; \
	make ALAP1_ucir; make ALAP2_ucir; make ALAP3_ucir; make ALAP4_ucir; \
	make ALAP5_ucir; make ALAP6_ucir; make ALAP7_ucir; make ALAP8_ucir; \
	make List1_ucir; make List2_ucir; make List3_ucir; make List4_ucir; \
	make List5_ucir; make List6_ucir; make List7_ucir; make List8_ucir;

test_uai:
	make ASAP1_uai; make ASAP2_uai; make ASAP3_uai; make ASAP4_uai; \
	make ALAP1_uai; make ALAP2_uai; make ALAP3_uai; make ALAP4_uai; \
	make List1_uai; make List2_uai; make List3_uai; make List4_uai;

test_umai:
	make ASAP1_umai; make ASAP2_umai; make ASAP3_umai; make ASAP4_umai; \
	make ASAP5_umai; make ASAP6_umai; \
	make ALAP1_umai; make ALAP2_umai; make ALAP3_umai; make ALAP4_umai; \
	make ALAP5_umai; make ALAP6_umai; \
	make List1_umai; make List2_umai; make List3_umai; make List4_umai; \
	make List5_umai; make List6_umai; 

test_umi:
	make ASAP1_umi; make ASAP2_umi; make ASAP3_umi; make ASAP4_umi; \
	make ASAP5_umi; make ASAP6_umi; \
	make ALAP1_umi; make ALAP2_umi; make ALAP3_umi; make ALAP4_umi; \
	make ALAP5_umi; make ALAP6_umi; \
	make List1_umi; make List2_umi; make List3_umi; make List4_umi; \
	make List5_umi; make List6_umi; 

test_uctr:
	make ASAP1_uctr; make ASAP2_uctr; make ASAP3_uctr; make ASAP4_uctr; \
	make ASAP5_uctr; make ASAP6_uctr; make ASAP7_uctr; make ASAP8_uctr; \
	make ALAP1_uctr; make ALAP2_uctr; make ALAP3_uctr; make ALAP4_uctr; \
	make ALAP5_uctr; make ALAP6_uctr; make ALAP7_uctr; make ALAP8_uctr; 
	make List1_uctr; make List2_uctr; make List3_uctr; make List4_uctr; \
	make List5_uctr; make List6_uctr; make List7_uctr; make List8_uctr; 

test_uar:
	make ASAP1_uar; make ASAP2_uar; make ASAP3_uar; make ASAP4_uar; \
	make ASAP5_uar; make ASAP6_uar; make ASAP7_uar; make ASAP8_uar; \
	make ALAP1_uar; make ALAP2_uar; make ALAP3_uar; make ALAP4_uar; \
	make ALAP5_uar; make ALAP6_uar; make ALAP7_uar; make ALAP8_uar; \
	make List1_uar; make List2_uar; make List3_uar; make List4_uar; \
	make List5_uar; make List6_uar; make List7_uar; make List8_uar;

test_umar:
	make ASAP1_umar; make ASAP2_umar; make ASAP3_umar; make ASAP4_umar; \
	make ASAP5_umar; make ASAP6_umar; make ASAP7_umar; make ASAP8_umar; \
	make ASAP9_umar; \
	make ALAP1_umar; make ALAP2_umar; make ALAP3_umar; make ALAP4_umar; \
	make ALAP5_umar; make ALAP6_umar; make ALAP7_umar; make ALAP8_umar; \
	make ALAP9_umar; \
	make List1_umar; make List2_umar; make List3_umar; make List4_umar; \
	make List5_umar; make List6_umar; make List7_umar; make List8_umar; \
	make List9_umar;

test_uhmr:
	make ASAP1_uhmr; make ASAP2_uhmr; make ASAP3_uhmr; make ASAP4_uhmr; \
	make ASAP5_uhmr; make ASAP6_uhmr; make ASAP7_uhmr; make ASAP8_uhmr; \
	make ASAP9_uhmr; make ASAP10_uhmr; \
	make ALAP1_uhmr; make ALAP2_uhmr; make ALAP3_uhmr; make ALAP4_uhmr; \
	make ALAP5_uhmr; make ALAP6_uhmr; make ALAP7_uhmr; make ALAP8_uhmr; \
	make ALAP9_uhmr; make ALAP10_uhmr; \
	make List1_uhmr; make List2_uhmr; make List3_uhmr; make List4_uhmr; \
	make List5_uhmr; make List6_uhmr; make List7_uhmr; make List8_uhmr; \
	make List9_uhmr; make List10_uhmr;

test_umr:
	make ASAP1_umr; make ASAP2_umr; make ASAP3_umr; make ASAP4_umr; \
	make ASAP5_umr; make ASAP6_umr; make ASAP7_umr; make ASAP8_umr; \
	make ASAP9_umr; \
	make ALAP1_umr; make ALAP2_umr; make ALAP3_umr; make ALAP4_umr; \
	make ALAP5_umr; make ALAP6_umr; make ALAP7_umr; make ALAP8_umr; \
	make ALAP9_umr; \
	make List1_umr; make List2_umr; make List3_umr; make List4_umr; \
	make List5_umr; make List6_umr; make List7_umr; make List8_umr; \
	make List9_umr;

test_ordbbr:
	make ASAP1_ordbbr; make ASAP2_ordbbr; make ASAP3_ordbbr; make ASAP4_ordbbr; \
	make ASAP5_ordbbr; make ASAP6_ordbbr; make ASAP7_ordbbr; make ASAP8_ordbbr; \
	make ASAP9_ordbbr; make ASAP10_ordbbr; make ASAP11_ordbbr; make ASAP12_ordbbr; \
	make ASAP13_ordbbr; make ASAP14_ordbbr; \
	make ALAP1_ordbbr; make ALAP2_ordbbr; make ALAP3_ordbbr; make ALAP4_ordbbr; \
	make ALAP5_ordbbr; make ALAP6_ordbbr; make ALAP7_ordbbr; make ALAP8_ordbbr; \
	make ALAP9_ordbbr; make ALAP10_ordbbr; make ALAP11_ordbbr; make ALAP12_ordbbr; \
	make ALAP13_ordbbr; make ALAP14_ordbbr;\
	make List1_ordbbr; make List2_ordbbr; make List3_ordbbr; make List4_ordbbr; \
	make List5_ordbbr; make List6_ordbbr; make List7_ordbbr; make List8_ordbbr; \
	make List9_ordbbr; make List10_ordbbr; make List11_ordbbr; make List12_ordbbr; \
	make List13_ordbbr; make List14_ordbbr;

test_ordbur:
	make ASAP1_ordbur; make ASAP2_ordbur; make ASAP3_ordbur; make ASAP4_ordbur; \
	make ASAP5_ordbur; make ASAP6_ordbur; make ASAP7_ordbur; make ASAP8_ordbur; \
	make ASAP9_ordbur; make ASAP10_ordbur; \
	make ALAP1_ordbur; make ALAP2_ordbur; make ALAP3_ordbur; make ALAP4_ordbur; \
	make ALAP5_ordbur; make ALAP6_ordbur; make ALAP7_ordbur; make ALAP8_ordbur; \
	make ALAP9_ordbur; make ALAP10_ordbur; \
	make List1_ordbur; make List2_ordbur; make List3_ordbur; make List4_ordbur; \
	make List5_ordbur; make List6_ordbur; make List7_ordbur; make List8_ordbur; \
	make List9_ordbur; make List10_ordbur; 

test_ordubr:
	make ASAP1_ordubr; make ASAP2_ordubr; make ASAP3_ordubr; make ASAP4_ordubr; \
	make ASAP5_ordubr; make ASAP6_ordubr; make ASAP7_ordubr; make ASAP8_ordubr; \
	make ASAP9_ordubr; make ASAP10_ordubr; \
	make ALAP1_ordubr; make ALAP2_ordubr; make ALAP3_ordubr; make ALAP4_ordubr; \
	make ALAP5_ordubr; make ALAP6_ordubr; make ALAP7_ordubr; make ALAP8_ordubr; \
	make ALAP9_ordubr; make ALAP10_ordubr; \
	make List1_ordubr; make List2_ordubr; make List3_ordubr; make List4_ordubr; \
	make List5_ordubr; make List6_ordubr; make List7_ordubr; make List8_ordubr; \
	make List9_ordubr; make List10_ordubr; 

test_ppbr:
	make ASAP1_ppbr; make ASAP2_ppbr; make ASAP3_ppbr; make ASAP4_ppbr; \
	make ASAP5_ppbr; make ASAP6_ppbr; make ASAP7_ppbr; make ASAP8_ppbr; \
	make ASAP9_ppbr; make ASAP10_ppbr; make ASAP11_ppbr; make ASAP12_ppbr; \
	make ASAP13_ppbr; \
	make ALAP1_ppbr; make ALAP2_ppbr; make ALAP3_ppbr; make ALAP4_ppbr; \
	make ALAP5_ppbr; make ALAP6_ppbr; make ALAP7_ppbr; make ALAP8_ppbr; \
	make ALAP9_ppbr; make ALAP10_ppbr; make ALAP11_ppbr; make ALAP12_ppbr; \
	make ALAP13_ppbr; 
	make List1_ppbr; make List2_ppbr; make List3_ppbr; make List4_ppbr; \
	make List5_ppbr; make List6_ppbr; make List7_ppbr; make List8_ppbr; \
	make List9_ppbr; make List10_ppbr; make List11_ppbr; make List12_ppbr; \
	make List13_ppbr; 
	make Iter1_ppbr; make Iter2_ppbr; make Iter3_ppbr; make Iter4_ppbr; \
	make Iter5_ppbr; make Iter6_ppbr; make Iter7_ppbr; make Iter8_ppbr; \
	make Iter9_ppbr; make Iter10_ppbr; make Iter11_ppbr; make Iter12_ppbr; \
	make Iter13_ppbr; 

test_random1:
	make ASAP1_random1; make ASAP2_random1; make ASAP3_random1; make ASAP4_random1; \
	make ALAP1_random1; make ALAP2_random1; make ALAP3_random1; make ALAP4_random1; \
	make List1_random1; make List2_random1; make List3_random1; make List4_random1;

test_random2:
	make ASAP1_random2; make ASAP2_random2; make ASAP3_random2; make ASAP4_random2; \
	make ALAP1_random2; make ALAP2_random2; make ALAP3_random2; make ALAP4_random2; \
	make List1_random2; make List2_random2; make List3_random2; make List4_random2;
#===================================================================
# Directory
#===================================================================
fir16:
	mkdir -p $(SIM_PATH)/fir16/ASAP{1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16}_fir16; \
	mkdir -p $(SIM_PATH)/fir16/ALAP{1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16}_fir16; \
	mkdir -p $(SIM_PATH)/fir16/List{1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16}_fir16; \
	mkdir -p $(SIM_PATH)/fir16/Iter{1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16}_fir16

ordbbr:
	mkdir -p $(SIM_PATH)/ordbbr/ASAP{1,2,3,4,5,6,7,8,9,10,11,12,13,14}_ordbbr; \
	mkdir -p $(SIM_PATH)/ordbbr/ALAP{1,2,3,4,5,6,7,8,9,10,11,12,13,14}_ordbbr; \
	mkdir -p $(SIM_PATH)/ordbbr/List{1,2,3,4,5,6,7,8,9,10,11,12,13,14}_ordbbr; \
	mkdir -p $(SIM_PATH)/ordbbr/Iter{1,2,3,4,5,6,7,8,9,10,11,12,13,14}_ordbbr 

ordbur:
	mkdir -p $(SIM_PATH)/ordbur/ASAP{1,2,3,4,5,6,7,8,9,10}_ordbur; \
	mkdir -p $(SIM_PATH)/ordbur/ALAP{1,2,3,4,5,6,7,8,9,10}_ordbur; \
	mkdir -p $(SIM_PATH)/ordbur/List{1,2,3,4,5,6,7,8,9,10}_ordbur; \
	mkdir -p $(SIM_PATH)/ordbur/Iter{1,2,3,4,5,6,7,8,9,10}_ordbur;

ordubr:
	mkdir -p $(SIM_PATH)/ordubr/ASAP{1,2,3,4,5,6,7,8,9,10}_ordubr; \
	mkdir -p $(SIM_PATH)/ordubr/ALAP{1,2,3,4,5,6,7,8,9,10}_ordubr; \
	mkdir -p $(SIM_PATH)/ordubr/List{1,2,3,4,5,6,7,8,9,10}_ordubr; \
	mkdir -p $(SIM_PATH)/ordubr/Iter{1,2,3,4,5,6,7,8,9,10}_ordubr;

ppbr:
	mkdir -p $(SIM_PATH)/ppbr/ASAP{1,2,3,4,5,6,7,8,9,10,11,12,13}_ppbr; \
	mkdir -p $(SIM_PATH)/ppbr/ALAP{1,2,3,4,5,6,7,8,9,10,11,12,13}_ppbr; \
	mkdir -p $(SIM_PATH)/ppbr/List{1,2,3,4,5,6,7,8,9,10,11,12,13}_ppbr; \
	mkdir -p $(SIM_PATH)/ppbr/Iter{1,2,3,4,5,6,7,8,9,10,11,12,13}_ppbr 

uctr:
	mkdir -p $(SIM_PATH)/uctr/ASAP{1,2,3,4,5,6,7,8}_uctr; \
	mkdir -p $(SIM_PATH)/uctr/ALAP{1,2,3,4,5,6,7,8}_uctr; \
	mkdir -p $(SIM_PATH)/uctr/List{1,2,3,4,5,6,7,8}_uctr; \
	mkdir -p $(SIM_PATH)/uctr/Iter{1,2,3,4,5,6,7,8}_uctr 

uar:
	mkdir -p $(SIM_PATH)/uar/ASAP{1,2,3,4,5,6,7,8}_uar; \
	mkdir -p $(SIM_PATH)/uar/ALAP{1,2,3,4,5,6,7,8}_uar; \
	mkdir -p $(SIM_PATH)/uar/List{1,2,3,4,5,6,7,8}_uar; \
	mkdir -p $(SIM_PATH)/uar/Iter{1,2,3,4,5,6,7,8}_uar

umar:
	mkdir -p $(SIM_PATH)/umar/ASAP{1,2,3,4,5,6,7,8,9}_umar; \
	mkdir -p $(SIM_PATH)/umar/ALAP{1,2,3,4,5,6,7,8,9}_umar; \
	mkdir -p $(SIM_PATH)/umar/List{1,2,3,4,5,6,7,8,9}_umar; \
	mkdir -p $(SIM_PATH)/umar/Iter{1,2,3,4,5,6,7,8,9}_umar 

uhmr:
	mkdir -p $(SIM_PATH)/uhmr/ASAP{1,2,3,4,5,6,7,8,9,10}_uhmr; \
	mkdir -p $(SIM_PATH)/uhmr/ALAP{1,2,3,4,5,6,7,8,9,10}_uhmr; \
	mkdir -p $(SIM_PATH)/uhmr/List{1,2,3,4,5,6,7,8,9,10}_uhmr; \
	mkdir -p $(SIM_PATH)/uhmr/Iter{1,2,3,4,5,6,7,8,9,10}_uhmr;

umr:
	mkdir -p $(SIM_PATH)/umr/ASAP{1,2,3,4,5,6,7,8,9}_umr; \
	mkdir -p $(SIM_PATH)/umr/ALAP{1,2,3,4,5,6,7,8,9}_umr; \
	mkdir -p $(SIM_PATH)/umr/List{1,2,3,4,5,6,7,8,9}_umr; \
	mkdir -p $(SIM_PATH)/umr/Iter{1,2,3,4,5,6,7,8,9}_umr 

uur:
	mkdir -p $(SIM_PATH)/uur/ASAP{1,2,3,4,5,6}_uur; \
	mkdir -p $(SIM_PATH)/uur/ALAP{1,2,3,4,5,6}_uur; \
	mkdir -p $(SIM_PATH)/uur/List{1,2,3,4,5,6}_uur; \
	mkdir -p $(SIM_PATH)/uur/Iter{1,2,3,4,5,6}_uur; 

ucti:
	mkdir -p $(SIM_PATH)/ucti/ASAP{1,2,3,4,5}_ucti; \
	mkdir -p $(SIM_PATH)/ucti/ALAP{1,2,3,4,5}_ucti; \
	mkdir -p $(SIM_PATH)/ucti/List{1,2,3,4,5}_ucti; \
	mkdir -p $(SIM_PATH)/ucti/Iter{1,2,3,4,5}_ucti;

unii:
	mkdir -p $(SIM_PATH)/unii/ASAP{1,2,3,4,5}_unii; \
	mkdir -p $(SIM_PATH)/unii/ALAP{1,2,3,4,5}_unii; \
	mkdir -p $(SIM_PATH)/unii/List{1,2,3,4,5}_unii; \
	mkdir -p $(SIM_PATH)/unii/Iter{1,2,3,4,5}_unii;

uuci:
	mkdir -p $(SIM_PATH)/uuci/ASAP{1,2,3,4,5}_uuci; \
	mkdir -p $(SIM_PATH)/uuci/ALAP{1,2,3,4,5}_uuci; \
	mkdir -p $(SIM_PATH)/uuci/List{1,2,3,4,5}_uuci; \
	mkdir -p $(SIM_PATH)/uuci/Iter{1,2,3,4,5}_uuci;

uucr:
	mkdir -p $(SIM_PATH)/uucr/ASAP{1,2,3,4,5,6,7,8}_uucr; \
	mkdir -p $(SIM_PATH)/uucr/ALAP{1,2,3,4,5,6,7,8}_uucr; \
	mkdir -p $(SIM_PATH)/uucr/List{1,2,3,4,5,6,7,8}_uucr; \
	mkdir -p $(SIM_PATH)/uucr/Iter{1,2,3,4,5,6,7,8}_uucr 

uaii:
	mkdir -p $(SIM_PATH)/uaii/ASAP{1,2,3,4,5}_uaii; \
	mkdir -p $(SIM_PATH)/uaii/ALAP{1,2,3,4,5}_uaii; \
	mkdir -p $(SIM_PATH)/uaii/List{1,2,3,4,5}_uaii; \
	mkdir -p $(SIM_PATH)/uaii/Iter{1,2,3,4,5}_uaii;

ucii:
	mkdir -p $(SIM_PATH)/ucii/ASAP{1,2,3,4,5}_ucii; \
	mkdir -p $(SIM_PATH)/ucii/ALAP{1,2,3,4,5}_ucii; \
	mkdir -p $(SIM_PATH)/ucii/List{1,2,3,4,5}_ucii; \
	mkdir -p $(SIM_PATH)/ucii/Iter{1,2,3,4,5}_ucii;

ucir:
	mkdir -p $(SIM_PATH)/ucir/ASAP{1,2,3,4,5,6,7,8}_ucir; \
	mkdir -p $(SIM_PATH)/ucir/ALAP{1,2,3,4,5,6,7,8}_ucir; \
	mkdir -p $(SIM_PATH)/ucir/List{1,2,3,4,5,6,7,8}_ucir; \
	mkdir -p $(SIM_PATH)/ucir/Iter{1,2,3,4,5,6,7,8}_ucir 

uai:
	mkdir -p $(SIM_PATH)/uai/ASAP{1,2,3,4}_uai; \
	mkdir -p $(SIM_PATH)/uai/ALAP{1,2,3,4}_uai; \
	mkdir -p $(SIM_PATH)/uai/List{1,2,3,4}_uai; \
	mkdir -p $(SIM_PATH)/uai/Iter{1,2,3,4}_uai;

umai:
	mkdir -p $(SIM_PATH)/umai/ASAP{1,2,3,4,5,6}_umai; \
	mkdir -p $(SIM_PATH)/umai/ALAP{1,2,3,4,5,6}_umai; \
	mkdir -p $(SIM_PATH)/umai/List{1,2,3,4,5,6}_umai; \
	mkdir -p $(SIM_PATH)/umai/Iter{1,2,3,4,5,6}_umai 

umi:
	mkdir -p $(SIM_PATH)/umi/ASAP{1,2,3,4,5,6}_umi; \
	mkdir -p $(SIM_PATH)/umi/ALAP{1,2,3,4,5,6}_umi; \
	mkdir -p $(SIM_PATH)/umi/List{1,2,3,4,5,6}_umi; \
	mkdir -p $(SIM_PATH)/umi/Iter{1,2,3,4,5,6}_umi 

pos:
	mkdir -p $(SIM_PATH)/pos/ASAP{1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16}_pos;\
	mkdir -p $(SIM_PATH)/pos/ALAP{1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16}_pos;\
	mkdir -p $(SIM_PATH)/pos/List{1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16}_pos;\
	mkdir -p $(SIM_PATH)/pos/Iter{1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16}_pos;

sop:
	mkdir -p $(SIM_PATH)/sop/SAP{1,2,3,4,5,6,7,8}_sop;\
	mkdir -p $(SIM_PATH)/sop/ALAP{1,2,3,4,5,6,7,8}_sop;\
	mkdir -p $(SIM_PATH)/sop/List{1,2,3,4,5,6,7,8}_sop;\
	mkdir -p $(SIM_PATH)/sop/Iter{1,2,3,4,5,6,7,8}_sop

ratelaw: 
	mkdir -p $(SIM_PATH)/ratelaw/ASAP{1,2,3,4,5}_ratelaw;\
	mkdir -p $(SIM_PATH)/ratelaw/ALAP{1,2,3,4,5}_ratelaw;\
	mkdir -p $(SIM_PATH)/ratelaw/List{1,2,3,4,5}_ratelaw;\
	mkdir -p $(SIM_PATH)/ratelaw/Iter{1,2,3,4,5}_ratelaw;

ratelaw1: 
	mkdir -p $(SIM_PATH)/ratelaw1/ASAP{1,2,3,4,5}_ratelaw1;\
	mkdir -p $(SIM_PATH)/ratelaw1/ALAP{1,2,3,4,5}_ratelaw1;\
	mkdir -p $(SIM_PATH)/ratelaw1/List{1,2,3,4,5}_ratelaw1;\
	mkdir -p $(SIM_PATH)/ratelaw1/Iter{1,2,3,4,5}_ratelaw1;

sample: 
	mkdir -p $(SIM_PATH)/sample/ASAP{1,2}_sample;\
	mkdir -p $(SIM_PATH)/sample/ALAP{1,2}_sample;\
	mkdir -p $(SIM_PATH)/sample/List{1,2}_sample;\
	mkdir -p $(SIM_PATH)/sample/Iter{1,2}_sample;

mux4:
	mkdir -p $(SIM_PATH)/mux4/ASAP1_mux4;

mux2:
	mkdir -p $(SIM_PATH)/mux2/ASAP{1,2,3,4}_mux2;

log_ch:
	mkdir -p $(SIM_PATH)/log/ASAP1_log;

f2d:
	mkdir -p $(SIM_PATH)/f2d/ASAP{1,2,3,4}_f2d;

mrbay:
	mkdir -p $(SIM_PATH)/mrbay/ASAP{1,2,3,4}_mrbay;

sample1: 
	mkdir -p $(SIM_PATH)/sample1/ASAP{1,2,3,4}_sample1;\
	mkdir -p $(SIM_PATH)/sample1/ALAP{1,2,3,4}_sample1;\
	mkdir -p $(SIM_PATH)/sample1/List{1,2,3,4}_sample1;\
	mkdir -p $(SIM_PATH)/sample1/Iter{1,2,3,4}_sample1;

random1:
	mkdir -p $(SIM_PATH)/random1/ASAP{1,2,3,4}_random1; \
	mkdir -p $(SIM_PATH)/random1/ALAP{1,2,3,4}_random1; \
	mkdir -p $(SIM_PATH)/random1/List{1,2,3,4}_random1; \
	mkdir -p $(SIM_PATH)/random1/Iter{1,2,3,4}_random1; 

random2:
	mkdir -p $(SIM_PATH)/random2/ASAP{1,2,3,4}_random2; \
	mkdir -p $(SIM_PATH)/random2/ALAP{1,2,3,4}_random2; \
	mkdir -p $(SIM_PATH)/random2/List{1,2,3,4}_random2; \
	mkdir -p $(SIM_PATH)/random2/Iter{1,2,3,4}_random2 

constant:
	mkdir -p $(SIM_PATH)/constant/ASAP1_constant; \
	mkdir -p $(SIM_PATH)/constant/ALAP1_constant; \

sample8:
	mkdir -p $(SIM_PATH)/sample8/ASAP1_sample8; \
	mkdir -p $(SIM_PATH)/sample8/ALAP1_sample8; \
	mkdir -p $(SIM_PATH)/sample8/ASAP2_sample8; \
	mkdir -p $(SIM_PATH)/sample8/ALAP2_sample8; \
	mkdir -p $(SIM_PATH)/sample8/ASAP3_sample8; \
	mkdir -p $(SIM_PATH)/sample8/ALAP3_sample8;

test_sample8:
	make TestSchedule2 arg1="SAMPLE8"; \
	./TestSchedule2_SAMPLE8_WBM 2 1 &> log_wbm.error; \
  ./TestSchedule2_SAMPLE8_LEA 2 1 &> log_lea.error; \
  ./TestSchedule2_SAMPLE8_ALL 2 1 &> log_all.error

test_sample9:
	make TestSchedule2 arg1="SAMPLE9"; \
	./TestSchedule2_SAMPLE9_WBM 3 3 &> log_wbm.error; \
  ./TestSchedule2_SAMPLE9_LEA 3 3 &> log_lea.error; \
  #./TestSchedule2_SAMPLE9_ALL 3 3 &> log_all.error

sample9:
	mkdir -p $(SIM_PATH)/sample9/ASAP1_sample9; \
	mkdir -p $(SIM_PATH)/sample9/ALAP1_sample9; \
	mkdir -p $(SIM_PATH)/sample9/ASAP2_sample9; \
	mkdir -p $(SIM_PATH)/sample9/ALAP2_sample9; \
	mkdir -p $(SIM_PATH)/sample9/ASAP3_sample9; \
	mkdir -p $(SIM_PATH)/sample9/ALAP3_sample9; \
	mkdir -p $(SIM_PATH)/sample9/ASAP4_sample9; \
	mkdir -p $(SIM_PATH)/sample9/ALAP4_sample9; \
	mkdir -p $(SIM_PATH)/sample9/ASAP5_sample9; \
	mkdir -p $(SIM_PATH)/sample9/ALAP5_sample9; \
	mkdir -p $(SIM_PATH)/sample9/ASAP6_sample9; \
	mkdir -p $(SIM_PATH)/sample9/ALAP6_sample9;

power:
	mkdir -p $(SIM_PATH)/power/ASAP1_power; \
	mkdir -p $(SIM_PATH)/power/ALAP1_power;

tmax:
	mkdir -p $(SIM_PATH)/tmax/ASAP{1,2,3,4,5,6,7,8}_tmax; \
	mkdir -p $(SIM_PATH)/tmax/ALAP{1,2,3,4,5,6,7,8}_tmax; \

simclean :
	for benchmark in $(SIM_PATH)/*; \
	do \
	if [ -d $$benchmark ]; then \
		for dir in $$benchmark/*; \
		do \
			if [ -d $$dir/rtl_work ]; then \
				rm -fr $$dir/rtl_work; \
			fi; \
			if [ -d $$dir/work ]; then \
				rm -fr $$dir/work; \
			fi; \
			rm -f $$dir/*.v; \
			rm -f $$dir/*.do; \
			rm -f $$dir/transcript; \
			rm -f $$dir/*.wlf; \
			rm -f $$dir/*.txt; \
			rm -f $$dir/log; \
			rm -f $$dir/*.log; \
			rm -f $$dir/*.data; \
			rm -f $$dir/*.opt; \
			rm -f $$dir/wlf*; \
		done \
	fi; \
	done



# e.g. Modelsim/fir16/ASAP1_fir16
% :
	@cd $(SIM_PATH)/$(word 2, $(subst _, , $*))/$*; \
	if [ -e $*_TB.do ]; then \
 		cp $(TEST_DATA_PATH)/$(word 2, $(subst _, , $*))_sw_result.txt \
		$(SIM_PATH)/$(word 2, $(subst _, , $*))/$*/sw_result.txt; \
		cp $(TEST_DATA_PATH)/$(word 2, $(subst _, , $*))_test.data \
		$(SIM_PATH)/$(word 2, $(subst _, , $*))/$*/ ; \
		vsim -c -do $*_TB.do ;  \
		diff hw_result.txt sw_result.txt > log; \
		if [ $$? = 0 ]; then \
		cd ../..; \
		else\
       echo "$* results mismatch"; \
       cat $(SIM_PATH)/$(word 2, $(subst _, , $*))/$*/log; \
		fi; \
	else echo Error: HDL files not found in $(SIM_PATH)/$(word 2, $(subst _, , $*))/$*; \
	fi

veryclean :
	rm -r $(SIM_PATH)

clean :
	rm -f *.exe *.stackdump; \
  rm -rf Log/ Stat/ CSV/
