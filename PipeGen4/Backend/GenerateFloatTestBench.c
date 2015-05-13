#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "Schedule.h"
#include "PortSchedule.h"

// II inputs are 32-bit wide. 4/12/12

void GenerateTestBench(char *ModuleName, char *ScheduleName, int pipeline_depth) {

  char fileName[100];
  FILE *vfp = NULL;
  FILE *cfp = NULL;

  vfp = fopen(GetFileName(fileName, "_TB", ".v", ScheduleName), "w");
  cfp = fopen(GetFileName(fileName, "_debug", ".txt", ScheduleName), "w");

  fprintf(vfp, "`timescale 1ns/1ps\n");
  fprintf(vfp, "module %s_TB();\n", ModuleName);

  GenerateInputPorts(vfp, "reg", 32);

  GenerateOutputPorts(vfp, ModuleName, "wire", "out", "rdy", "");

  GenerateInstance(vfp, ModuleName);

  GenerateSetUp(vfp, cfp, ModuleName, ScheduleName, pipeline_depth);

  fclose(vfp);
}

// Sequences directed by function units' go time 
void GenerateDebugData(FILE *vfp, FILE *cfp, char *ModuleName, int testCount) {
  int i, tmp, t, k;
  int *FUGoTimeIdx = (int *) malloc(NODE_NU * sizeof(int));

  for (i = 0; i < NODE_NU; i++)
    FUGoTimeIdx[i] = i;

  // Indirect selection sort FU go time (not recommended)
  for (t = NODE_NU-1; t >= 1; t--) {
    i = 0;
    for (k = 1; k <= t; k++) {
      if (DFG[FUGoTimeIdx[i]]->opScheduledSlot < 
          DFG[FUGoTimeIdx[k]]->opScheduledSlot)
        i = k;
    }
    tmp = FUGoTimeIdx[t];
    FUGoTimeIdx[t] = FUGoTimeIdx[i];
    FUGoTimeIdx[i] = tmp;
  }

  //------------------------------------------
  // SW gold results
  //------------------------------------------
  GenerateSWDebugData(cfp, ModuleName, testCount, FUGoTimeIdx);

  //------------------------------------------
  // HW test struct
  //------------------------------------------
  GenerateHWDebugData(vfp, ModuleName, testCount, FUGoTimeIdx);

  free(FUGoTimeIdx);
}

void GenerateInstance(FILE *vfp, char *ModuleName) {
  fprintf(vfp, "%s %s_i (\n", ModuleName,  ModuleName);
  GenerateModuleSignals(vfp, ModuleName);
}

// Specify the total number of tests (e.g. int testCount = 8;)
void GenerateSetUp(FILE *vfp, FILE *cfp, char *ModuleName, 
    char *ScheduleName, int pipeline_depth ) {
  FILE *src;
  char ch;
  char *opSym;
  int i;
  enum operation op_type;
  int size;
  int timeSlotNu;
  int testCount = TEST_CNT;  // Schedule.h
  char *proj_path;  // current project path
  char path[100];
  const int clock_cycle = 20;

  //-----------------------------------------------------
  // Copy from vlib/TestTask.v
  // The file contains the tasks called in testbench 
  //-----------------------------------------------------
  src = fopen("vlib/TestTask.v", "r"); 
  while((ch = getc(src)) != EOF) putc(ch, vfp);
  fclose(src);

  // e.g. proj_path/Modelsim/mrbay/ASAP4_mrbay
  proj_path = gnu_getcwd();
  sprintf(path, "%s/Modelsim/%s/%s%s", 
            proj_path, CircuitName, ScheduleName, CircuitName);

  //-----------------------------------------------------
  // File descriptor and arithmetic exp ouput declarations
  //-----------------------------------------------------
  fprintf(vfp, "reg signed [31 : 0] res;\n");
  // Declare file descriptors 
  fprintf(vfp, "integer FD_hw;\n");
  fprintf(vfp, "integer FD_sw;\n");
  fprintf(vfp, "integer FD_dbg[%d : 0];\n", testCount - 1);
  fprintf(vfp, "integer i;\n");

  fprintf(vfp, "reg [50*8-1: 0] filename;\n\n");
  fprintf(vfp, "reg [31:0] val1;\n");
  fprintf(vfp, "reg [31:0] val2;\n\n");

  //-----------------------------------------------------
  // Test set-up 
  //-----------------------------------------------------

  // clock
  fprintf(vfp, "always #%d clk = ~clk;\n\n", clock_cycle / 2);

  // Read coefficient memory 
  // Note c(i) doesn't necessarily correspond to rom(i+1).mem due to resource scheduling
  fprintf(vfp, "initial begin\n");
  for (i = 0; i < NODE_NU; i++) {

    if (DFG[i]->op == rom) 
      fprintf(vfp, "%2s$readmemh(\"%s/c%0d.data\", %s_i.rom%0d.mem);\n", " ", 
          path, (int)DFG[i]->opConstVal-1, ModuleName, DFG[i]->opResourceNu);

    if (DFG[i]->op == romd) 
      fprintf(vfp, "%2s$readmemh(\"%s/c%0d_d.data\", %s_i.romd%0d.mem);\n", " ", 
          path, (int)DFG[i]->opConstVal_d-1, ModuleName, DFG[i]->opResourceNu);
  }
  fprintf(vfp, "end\n");

  // hardware debug files
  fprintf(vfp, "initial begin\n");
  fprintf(vfp, "%2sfor (i = 0; i < %d; i = i + 1) begin\n", " ", testCount);
  fprintf(vfp, "%4s$sformat(filename, \"%s_debug%%0d.txt\", i);\n", " ",CircuitName);
  fprintf(vfp, "%4sFD_dbg[i] = $fopen(filename);\n", " "); // w
  fprintf(vfp, "%2send\n", " ");
  fprintf(vfp, "end\n");

  // hardware result file
  fprintf(vfp, "initial begin\n");
  fprintf(vfp, "  FD_hw = $fopen(\"hw_result.txt\", \"w+\");\n"); // w
  fprintf(vfp, "  FD_sw = $fopen(\"%s/TestData/%s_test.data\", \"r+\");\n", proj_path, CircuitName); // r

  free(proj_path);

  ResetStimulus(vfp);

  fprintf(vfp, "  repeat (2) @(negedge clk) rst = 1'b1;\n");
  fprintf(vfp, "  repeat (2) @(negedge clk) rst = 1'b0;\n");
  fprintf(vfp, "\n");

  //-----------------------------------------------------
  // Generate Stimulus several times
  //-----------------------------------------------------
  GenerateStimulus(vfp, testCount);

  //-----------------------------------------------------
  // Clear go at the rising clock edge
  //-----------------------------------------------------
  fprintf(vfp, "%2s@(posedge clk) #1\n", "");
  fprintf(vfp, "%2sgo_in = 1'b0;\n\n", "");

  //-----------------------------------------------------
  // Stop and close files
  //-----------------------------------------------------
  fprintf(vfp, "%2s#%d\n", "", MAX_PORT_NU +  (clock_cycle * pipeline_depth)); 
  fprintf(vfp, "%2s$fclose(FD_sw);\n", "");
  fprintf(vfp, "%2s$fclose(FD_hw);\n", "");  
  fprintf(vfp, "%2sfor (i = 0; i < %d; i = i + 1) ", " ", testCount);
  fprintf(vfp, "$fclose(FD_dbg[i]);\n", "");  
  fprintf(vfp, "%2s$finish;\n", "");
  fprintf(vfp, "end\n");
  fprintf(vfp, "\n");

  DumpOutputValues(vfp, ModuleName);

  // add 12/24
  GenerateDebugData(vfp, cfp, ModuleName, testCount);

  fprintf(vfp, "endmodule\n"); 
}

void DumpOutputValues(FILE *vfp, char *ModuleName) {

  int i, j;
  char *opSym;
  char *OutputName;
  enum operation op_type;
  int size;
  int outputNu;
  int resourceNu;

  //------------------------------------------------------------
  // Assumption
  //
  // Dump only one node whose id = NODE_NU-1
  //
  //------------------------------------------------------------
  // Generate output signals
  //------------------------------------------------------------

  fprintf(vfp, "reg stall_in_r;\n\n");
  fprintf(vfp, "reg [15:0] result_cnt = 0;\n\n");
  fprintf(vfp, "always @(posedge clk) begin\n");
  fprintf(vfp, "  stall_in_r <= stall_in;\n"); // use stall_r for valid output dump

  for (i = 0; i < NODE_NU; i++) {

    if (DFG[i]->opDest[0] == SINK) {

      opSym = opSymTable(DFG[i]->op);
      resourceNu = DFG[i]->opResourceNu;
      OutputName = DFG[i]->opName;


      // rdy -> out_rdy 
      fprintf(vfp, "  if (%s_%s_out_rdy && !stall_in_r) begin\n",  // if
          ModuleName, OutputName); 

      if (i == NODE_NU - 1) fprintf(vfp, "    result_cnt <= result_cnt + 1;\n"); 

      if (!strcmp(opTypeTable(DFG[i]->op), "compare")) {
        fprintf(vfp, "    $fdisplay(FD_hw, \"%s = %%d\", %s_%s_out);\n",
          OutputName, ModuleName, OutputName);
      }
      else {
        if (DFG[i]->opPrecision == sfp)
          #ifdef HEX_DUMP
          fprintf(vfp, "    $fdisplay(FD_hw, \"%s = %%h(%%f)\", %s_%s_out, $bitstoshortreal(%s_%s_out));\n", 
            OutputName, ModuleName, OutputName, ModuleName, OutputName);
          #else
          fprintf(vfp, "    $fdisplay(FD_hw, \"%s = %%f\", $bitstoshortreal(%s_%s_out));\n", 
            OutputName, ModuleName, OutputName);
          #endif
        
        if (DFG[i]->opPrecision == dfp)
          #ifdef HEX_DUMP
          fprintf(vfp, "    $fdisplay(FD_hw, \"%s = %%h(%%f)\", %s_%s_out, $bitstoreal(%s_%s_out));\n", 
            OutputName, ModuleName, OutputName, ModuleName, OutputName);
          #else
          fprintf(vfp, "    $fdisplay(FD_hw, \"%s = %%f\", $bitstoreal(%s_%s_out));\n", 
            OutputName, ModuleName, OutputName);
          #endif
      }

      fprintf(vfp, "  end\n"); // if_end
    }
  }
  fprintf(vfp, "end\n\n"); // always_end
}

/* Case1:
 * input ports are fed with data in the toplogical order of the input nodes
 * with the corresponding functional units numbered according to the scheduling.
 * 
 * Case2:
 * input ports are fed with data NOT in the toplogical order of the input nodes
 * 1) The data in the test data file are generated in the toplogical order 
 * of the input nodes.
 *
 * solution1: change how data are generated in the GenerateTestData function
 * solution2: change how data are fed into the input ports based on the scheduling
 *
 * Generate TestCount number of stimulus 
 */
int GenerateStimulus(FILE *vfp, int TestCount) {
  int i, j, cnt;
  int maxPortNu;
  int node;
  int stall;
  int lastSlot;
  int *TestValues;
  int stall_cycles;

  const int MAX = 8;

  // Generate test data and result for hw simulation
  GenerateTestData(TestCount);
  
  maxPortNu = MAX_PORT_NU;

  TestValues = (int *) malloc (sizeof(int) * maxPortNu);

  srand(11);

  for (cnt = 0; cnt < TestCount; cnt++) {

    lastSlot = 0;

    // start at pos clock edge and after a 1 unit delay
    fprintf(vfp, "  @(posedge clk) #1\n");
    for (i = 0; i < maxPortNu; i++) {

      // use node just for reuse of variable
      node = (SortedPortPriority != NULL) ? SortedPortPriority[i] : i;

      if (DFG[node]->opScheduledSlot > lastSlot) { 

        #ifdef STALL
        //-------------------------------------------------------------------
        // insert stall cycles randomly before maxPortNu number of data 
        // in a set have been consumed
        //-------------------------------------------------------------------
        if (rand() % 2) {
          stall_cycles = rand() % 10 + 1;
          fprintf(vfp, "  repeat(%d) begin\n", stall_cycles);
          fprintf(vfp, "    @(posedge clk) #1\n");
          fprintf(vfp, "    test(1, 1, 32'bx, stall_in, go_in, p1_in);\n");
          fprintf(vfp, "  end\n");
        }
        #endif

        fprintf(vfp, "  @(posedge clk) #1\n");
      }

      j = PortAssignment[i];
       
      fprintf(vfp, "  $fseek(FD_sw, (%d * %d + %d) * 9, 0);\n", cnt, maxPortNu, node);
      fprintf(vfp, "  $fscanf(FD_sw, \"%%h\", val1);\n");
      fprintf(vfp, "  test(1, 0, val1, stall_in, go_in, p%d_in);\n", j); 

      lastSlot = DFG[node]->opScheduledSlot;
    }

    #ifdef STALL
    //-------------------------------------------------------------------
    // insert stall cycles randomly after maxPortNu number of data 
    // in a set have been consumed
    //
    // Note 
    // If the circuit has more than one input port and any input data 
    // is not ready, then stall is asserted.
    //-------------------------------------------------------------------
    if (rand() % 2) {
      stall_cycles = rand() % 50 + 1;
      fprintf(vfp, "  repeat(%d) begin\n", stall_cycles);
      fprintf(vfp, "    @(posedge clk) #1\n");
      fprintf(vfp, "    test(1, 1, 32'bx, stall_in, go_in, p1_in);\n");
      fprintf(vfp, "  end\n");
    }
    #endif
  }

  //-----------------------------------------------------
  // Clear both stall and go, leave port inputs unchanged
  //-----------------------------------------------------
  fprintf(vfp, "%2s@(posedge clk) #1\n", "");
  fprintf(vfp, "%2sstall_in = 1'b0;\n\n", "");
  fprintf(vfp, "%2sgo_in = 1'b0;\n\n", "");

  free(TestValues);
  return TestCount;
}

void ResetStimulus(FILE *vfp) {

  int i, j;
  int size;
  enum operation op_type;
  const int PortSignalNu = 4;

  fprintf(vfp, "  rst = 0;\n");
  fprintf(vfp, "  clk = 0;\n");

#ifdef MRBAY
  //------------------------------------------------------------------
  // Application-specific ports
  //
  // App: mrbay
  // PL/PR values are generated in Beagle MJC69
  //------------------------------------------------------------------
  fprintf(vfp, "  II1_in = 32'h%x;\n", FloatingPointConvert(0.25));
  fprintf(vfp, "  II2_in = 32'h%x;\n", FloatingPointConvert(0.25));
  fprintf(vfp, "  II3_in = 32'h%x;\n", FloatingPointConvert(0.25));
  fprintf(vfp, "  II4_in = 32'h%x;\n", FloatingPointConvert(0.25));

  fprintf(vfp, "  PL_AA_in = 32'h%x;\n", FloatingPointConvert(0.90638));
  fprintf(vfp, "  PL_AC_in = 32'h%x;\n", FloatingPointConvert(0.0312067));
  fprintf(vfp, "  PL_AG_in = 32'h%x;\n", FloatingPointConvert(0.0312067));
  fprintf(vfp, "  PL_AT_in = 32'h%x;\n", FloatingPointConvert(0.0312067));

  fprintf(vfp, "  PL_CA_in = 32'h%x;\n", FloatingPointConvert(0.0312067));
  fprintf(vfp, "  PL_CC_in = 32'h%x;\n", FloatingPointConvert(0.90638));
  fprintf(vfp, "  PL_CG_in = 32'h%x;\n", FloatingPointConvert(0.0312067));
  fprintf(vfp, "  PL_CT_in = 32'h%x;\n", FloatingPointConvert(0.0312067));

  fprintf(vfp, "  PL_GA_in = 32'h%x;\n", FloatingPointConvert(0.0312067));
  fprintf(vfp, "  PL_GC_in = 32'h%x;\n", FloatingPointConvert(0.0312067));
  fprintf(vfp, "  PL_GG_in = 32'h%x;\n", FloatingPointConvert(0.90638));
  fprintf(vfp, "  PL_GT_in = 32'h%x;\n", FloatingPointConvert(0.0312067));

  fprintf(vfp, "  PL_TA_in = 32'h%x;\n", FloatingPointConvert(0.0312067));
  fprintf(vfp, "  PL_TC_in = 32'h%x;\n", FloatingPointConvert(0.0312067));
  fprintf(vfp, "  PL_TG_in = 32'h%x;\n", FloatingPointConvert(0.0312067));
  fprintf(vfp, "  PL_TT_in = 32'h%x;\n", FloatingPointConvert(0.90638));

  fprintf(vfp, "  PR_AA_in = 32'h%x;\n", FloatingPointConvert(0.90638));
  fprintf(vfp, "  PR_AC_in = 32'h%x;\n", FloatingPointConvert(0.0312067));
  fprintf(vfp, "  PR_AG_in = 32'h%x;\n", FloatingPointConvert(0.0312067));
  fprintf(vfp, "  PR_AT_in = 32'h%x;\n", FloatingPointConvert(0.0312067));

  fprintf(vfp, "  PR_CA_in = 32'h%x;\n", FloatingPointConvert(0.0312067));
  fprintf(vfp, "  PR_CC_in = 32'h%x;\n", FloatingPointConvert(0.90638));
  fprintf(vfp, "  PR_CG_in = 32'h%x;\n", FloatingPointConvert(0.0312067));
  fprintf(vfp, "  PR_CT_in = 32'h%x;\n", FloatingPointConvert(0.0312067));

  fprintf(vfp, "  PR_GA_in = 32'h%x;\n", FloatingPointConvert(0.0312067));
  fprintf(vfp, "  PR_GC_in = 32'h%x;\n", FloatingPointConvert(0.0312067));
  fprintf(vfp, "  PR_GG_in = 32'h%x;\n", FloatingPointConvert(0.90638));
  fprintf(vfp, "  PR_GT_in = 32'h%x;\n", FloatingPointConvert(0.0312067));

  fprintf(vfp, "  PR_TA_in = 32'h%x;\n", FloatingPointConvert(0.0312067));
  fprintf(vfp, "  PR_TC_in = 32'h%x;\n", FloatingPointConvert(0.0312067));
  fprintf(vfp, "  PR_TG_in = 32'h%x;\n", FloatingPointConvert(0.0312067));
  fprintf(vfp, "  PR_TT_in = 32'h%x;\n", FloatingPointConvert(0.90638));

  fprintf(vfp, "  Norm_in = 1'b1;\n"); // always scaling
#endif

}


