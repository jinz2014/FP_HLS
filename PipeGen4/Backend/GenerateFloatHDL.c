//======================================================================================
// log
//
// FU signals are different for fdiv 12/21/10
//
// Opt2's MUX fanout is not implemented. I still put the description of Opt2 there
// as a comparion with Opt1
//
//======================================================================================

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <string.h>
#include "Schedule.h"
#include "PortSchedule.h"
#include "MUX.h"

// 
void GenerateBackEnd(FILE *fp, char *scheduleName, const char *schedule, 
                     int portNu, int scheduleDelay, ScheduleStats SS[]) {

  int *Map;
  int sharedRegNu;
  int i;
  int muxpCnt, muxCnt; 

  sprintf(scheduleName, schedule);
  sprintf(scheduleName + strlen(schedule), "%d_", portNu);

  // Setup variable-register Map table
  Map = (int *) calloc (NODE_NU, sizeof(int));
  for (i = 0; i < NODE_NU; i++)
    if (DFG[i]->op == nopc) 
      Map[i] = 0 - (int)(DFG[i]->opConstVal);


  // Register allocation
  sharedRegNu = RegAlloc (Map, scheduleDelay);

  #ifdef WBM

  WBM_FAILED = 0;

  if (sharedRegNu != -2) {
    myprintf("WBM cluster divisions worked\n");
  } else { 
    printf("**** WBM failed!! Use LEA and exclude its statistics  ***\n");
    WBM_FAILED = 1;
    sharedRegNu = LeftEdgeRegisterBinding(Map);
  }

  #endif

  //-------------------------------------
  // MUXp allocation
  //-------------------------------------
  muxpCnt = muxCnt = InterconnectionAlloc(Map, sharedRegNu);

  //-------------------------------------
  // 
  //-------------------------------------
  GenerateHDL(fp, Map, &muxCnt, sharedRegNu, scheduleName, scheduleDelay, SS); 

  //-------------------------------------
  // Collect Statistics
  //-------------------------------------
  GetTotalRegNum(scheduleDelay, sharedRegNu);
  GetTotalMuxInputs(muxpCnt, muxCnt, sharedRegNu);
  GetHDLFanout(fp, sharedRegNu);
  CollectStats(SS, scheduleName);

  //-------------------------------------
  // Free
  //-------------------------------------
  // We don't use MuxSelTable anymore
  if (muxCnt != 0) FreeMuxSelTable(muxCnt);

  for (i = 0; i < NODE_NU; i++)
    if (DFG[i]->RegFileTable != NULL) FreeRegFileTable(i);
  FreeTables(sharedRegNu);
  free(Map);
}

int GenerateHDL(FILE *fp, int *Map, int *muxCnt, int SharedRegNu, char *ScheduleName, \
                int delay, ScheduleStats SS[]) {

  int i, j;
  int *MuxCntTable = malloc (sizeof(int) * (SharedRegNu + 1));
  int Phase1MuxInputNu, Phase2MuxInputNu;
  char ModuleName[20];
  char fileName[100];
  char ch;
  int totalRegNu;
  int muxInputNu;

  FILE *vfp;  // Verilog files

#ifdef DATA_PATH_ONLY
  FILE *vpp;
  vfp = fopen(GetFileName(fileName, "_HDL", ".v.tmp", ScheduleName), "w");
  vpp = fopen(GetFileName(fileName, "_HDL", ".v", ScheduleName), "w");
#else
  vfp = fopen(GetFileName(fileName, "_HDL", ".v", ScheduleName), "w");

  if (UseROM(rom) || UseROM(romd)) {
    char memFileName[100];
    FILE *mfp;
    int32_or_float val;
    int64_or_double val_d;

    if (UseROM(rom)) {
      //----------------------------------------------------
      // Single
      //----------------------------------------------------
      val.f = 0.01; // initial memory fill value

      for (j = 0; j < WORDS; j++) {
        sprintf(memFileName, "Modelsim/%s/%s%s/c%d.data", 
            CircuitName, ScheduleName, CircuitName, j);
        mfp = fopen(memFileName, "w");
        for (i = 0; i < DEPTH; i++) {
          fprintf(mfp, "%08x\n", val.i);
          myprintf("c%d.%d %08x(%.11f)\n", j, i, val.i, val.f);
          val.f++;
        }
        fclose(mfp);
      }
    }

    if (UseROM(romd)) {
      //----------------------------------------------------
      // Double
      //----------------------------------------------------
      val_d.f = 0.01; // initial memory fill value
      for (j = 0; j < WORDS; j++) {
        sprintf(memFileName, "Modelsim/%s/%s%s/c%d_d.data", 
            CircuitName, ScheduleName, CircuitName, j);
        mfp = fopen(memFileName, "w");
        for (i = 0; i < DEPTH; i++) {
          fprintf(mfp, "%016llx\n", val_d.i);
          myprintf("c_d%d.%d %016llx(%.11f)\n", j, i, val_d.i, val_d.f);
          val_d.f++;
        }
        fclose(mfp);
      }
    }
  }

#endif

  // Reset CSP fields
  CSP->mux_reg = 0;
  CSP->reg = 0;
  CSP->dly = delay;
  CSP->mux = 0;
  CSP->muxp = 0;
  CSP->muxr = 0;
  CSP->fan = 0;
  CSP->rfo = 0;
  CSP->ffo = 0;
  CSP->pfo = 0;
  CSP->tfo = 0;

  // module name
  strcpy(ModuleName,  CircuitName);

#ifndef DATA_PATH_ONLY


  // module ports
  GenerateModulePort(vfp, ModuleName);

  //-------------------------------------------------------
  // But conventionally parameters are put before the port defs.
  //-------------------------------------------------------
  fprintf(vfp, "parameter Delay = %d;\n\n", delay);
  // Application-specific parameters by software
  fprintf(vfp, "parameter TEST_NU = %d;\n\n", TEST_CNT); // Schedule.h

  fprintf(vfp, "wire [Delay - 1 : 0] state;\n");
  fprintf(vfp, "wire stall;\n");
  //fprintf(vfp, "wire finish;\n");
  //fprintf(vfp, "wire finish_enable;\n");
  fprintf(vfp, "wire go;\n");
  fprintf(vfp, "wire state_wen;\n");
  fprintf(vfp, "wire state_din;\n");
  fprintf(vfp, "wire state_out;\n");
  if (minII > 1) {
    fprintf(vfp, "wire [%d : 0] state_in;\n", minII - 1);
    fprintf(vfp, "wire dio;\n");
  }
  fprintf(vfp, "reg  run;\n");

#endif

  //------------------------------------------------------
  // Registered data inputs
  // Assume 32-bit inputs
  //------------------------------------------------------
  for (i = 1; i <= PORT_NU; i++) {
    fprintf(vfp, "reg [%d : 0] p%d_in_r;\n", 32 - 1, i);
  } 

  fprintf(vfp, "\nalways @ (posedge clk) begin\n");
  fprintf(vfp, "  if (!stall) begin\n");
  for (i = 1; i <= PORT_NU; i++) 
    fprintf(vfp, "   p%d_in_r <= p%d_in;\n", i, i);
  fprintf(vfp, "  end\n");
  fprintf(vfp, "end\n\n");

#ifdef MRBAY
  //------------------------------------------------------
  // Application-specific registered data inputs begins
  // 
  // App: mrbayes
  //
  // Note these input ports don't receive data every minII cycles. 
  // 
  // 1. Four 64-bit prior inputs in mrbay datapath(IIA, IIG, IIC, IIT)
  // 2. 1-bit mux normalization select in mrbay datapath (Norm)
  //------------------------------------------------------
  const char dna [] = {'A', 'C', 'G', 'T'};

  for (i = 1; i <= 4; i++) {
    fprintf(vfp, "reg [%d : 0] II%d_in_r;\n", 32-1, i);
  } 

  for (i = 0; i <= 3; i++)  {
    for (j = 0; j <= 3; j++)  {
      fprintf(vfp, "reg [31 : 0] PL_%c%c_in_r;\n", dna[i], dna[j]);
      fprintf(vfp, "reg [31 : 0] PR_%c%c_in_r;\n", dna[i], dna[j]);
    }
  }

  fprintf(vfp, "reg [%d : 0] Norm_in_r;\n", 0, i);

  fprintf(vfp, "\nalways @ (posedge clk) begin\n");

  for (i = 1; i <= 4; i++) 
    fprintf(vfp, "  II%d_in_r <= II%d_in;\n", i, i);

  fprintf(vfp, "  Norm_in_r <= Norm_in;\n");

  for (i = 0; i <= 3; i++)  {
    for (j = 0; j <= 3; j++)  {
      fprintf(vfp, "  PL_%c%c_in_r <= PL_%c%c_in;\n",
          dna[i], dna[j], dna[i], dna[j]);
      fprintf(vfp, "  PR_%c%c_in_r <= PR_%c%c_in;\n",
          dna[i], dna[j], dna[i], dna[j]);
    }
  }
  fprintf(vfp, "end\n\n");

  //------------------------------------------------------
  // Application-specific registered data inputs ends
  //------------------------------------------------------
#endif


  // Data path Muxes
  GenerateFU(vfp);

  // Data path FU input registers
  GenerateFuncRegister(vfp);

  // Data path 
  totalRegNu = GenerateRegisters(vfp, fp, Map, ModuleName, SharedRegNu);

  // Data path 
  GeneratePhase2Mux(vfp, fp, SharedRegNu, muxCnt, MuxCntTable);

  // Control path
  // Preparation of generating Phase1 MUXes
  GenerateSymbolicFSM(vfp, Map, SharedRegNu, muxCnt, \
                      MuxCntTable, delay, ScheduleName);

  // Data path 
  Phase1MuxInputNu = GeneratePhase1Mux(vfp, fp, Map, SharedRegNu);

  // Data path 
  GeneratePhase1Wire(vfp, fp, Map);
  
  // Data path 
  GeneratePhase2Wire(vfp, SharedRegNu);

  // pipeline completion phase
  GenerateStallControl(vfp, delay);

  GenerateControlWires(vfp, muxCnt, SharedRegNu);

  GenerateControlFSM(vfp);

  // Reduce the number of output registers and ready counters when there are
  // lots of outputs
  GenerateOutputAssign(vfp, ModuleName, "out", "rdy", "");
  //
  // The number of output registers and ready counters is equal
  // to the number of outputs. This increases the FGPA FF usage.
  //
  //GenerateNamedOutputPorts(vfp, ModuleName, "wire", "res", "rdy", ""); 
  //GenerateCounters(vfp, ModuleName, "res", "rdy", ""); 

  fprintf(vfp, "\nendmodule\n\n");

  fclose(vfp);

  #ifdef TB
  GenerateTestBench(ModuleName, ScheduleName, delay);
  GenerateDoFile(ScheduleName);
  #endif

  free(MuxCntTable);

  #ifdef DATA_PATH_ONLY
  FreePortList();
  #endif

  
}

   
void GenerateModulePort(FILE *vfp, char *ModuleName) {

  fprintf(vfp, "module %s (\n", ModuleName);

  GenerateModuleSignals(vfp, ModuleName);

  GenerateInputPorts(vfp, "input", 32);

  // or use output reg signed
  GenerateOutputPorts(vfp, ModuleName, "output signed", \
                           "out", "rdy", ""); 

  fprintf(vfp, "\n\n");

  //-------------------------------------------------------
  // Registered input port fanout
  //-------------------------------------------------------
  PortFanout = (int *) calloc (PORT_NU, sizeof(int));
}


void GenerateModuleSignals(FILE *vfp, char *ModuleName) {

  int i, j;
  char *opSym, comma;
  enum operation lastType, op_type;
  int lastSize, size;
  int ResourceNu;
  portStringPtr p;

  // Generate input signals
  fprintf(vfp, "  clk,\n");
  fprintf(vfp, "  rst,\n");

  for (i = 1; i <= PORT_NU; i++) { 
    fprintf(vfp, "  p%d_in,\n", i);
  } 

#ifdef MRBAY
  //-------------------------------------------------------------------
  // Application-specific 
  // 
  // App: mrbay
  //
  fprintf(vfp, "  II1_in,\n  II2_in,\n  II3_in,\n  II4_in,\n  Norm_in,\n");

  const char dna [] = {'A', 'C', 'G', 'T'};
  for (i = 0; i <= 3; i++)  {
    for (j = 0; j <= 3; j++)  {
      fprintf(vfp, "  PL_%c%c_in,\n", dna[i], dna[j]);
      fprintf(vfp, "  PR_%c%c_in,\n", dna[i], dna[j]);
    }
  }
  // Application-specific 
  //-------------------------------------------------------------------
#endif

  fprintf(vfp, "  stall_in,\n");
  fprintf(vfp, "  go_in,\n");

  // Generate output signals
  lastSize = NODE_NU;
  for (i = 0; i < NODE_NU; i++) {
    if (DFG[i]->op != nop && DFG[i]->opDest[0] < 0) {
      if (i > lastSize) fprintf(vfp, "%c\n", ',');

      opSym = opSymTable(DFG[i]->op);
      ResourceNu = DFG[i]->opResourceNu;

      fprintf(vfp, "  %s_%s_out,\n", ModuleName, DFG[i]->opName);
      fprintf(vfp, "  %s_%s_out_rdy", ModuleName, DFG[i]->opName);

      lastSize = i;
    }
  }

#ifdef DATA_PATH_ONLY
  
  // First add a coma and newline 
  fprintf(vfp, ",\n");

  fprintf(vfp, "  finish,\n");

  // Then add extra control inputs 
  p = PortNameList;
  while ( p != NULL) {
    if (p->next == NULL) 
      // remove a comma appending the last signal
      *(p->s + strlen(p->s) - 1) = '\0'; 
    fprintf(vfp, "  %s\n", p->s);
    p = p->next;
  }

#endif

  fprintf(vfp, ");\n\n");
}

/* Generate input ports based on the PQ */
void GenerateInputPorts(FILE *vfp, const char *ConnectionName, int dataWidth) {
  int i;
  int size;
  char *opSym;
  enum operation op_type;
  portStringPtr p;

  fprintf(vfp, "  %4s clk;\n", ConnectionName);
  fprintf(vfp, "  %4s rst;\n", ConnectionName);

  for (i = 1; i <= PORT_NU; i++) {
    fprintf(vfp, "  %4s [%d : 0] p%d_in;\n", 
            ConnectionName, dataWidth - 1, i);
  }

#ifdef MRBAY
  //--------------------------------------------------------------
  // Application specific ports
  //
  // App: mrbayes
  //--------------------------------------------------------------
  fprintf(vfp, "  %4s [%d : 0] II1_in;\n", ConnectionName, 32 - 1);
  fprintf(vfp, "  %4s [%d : 0] II2_in;\n", ConnectionName, 32 - 1);
  fprintf(vfp, "  %4s [%d : 0] II3_in;\n", ConnectionName, 32 - 1);
  fprintf(vfp, "  %4s [%d : 0] II4_in;\n", ConnectionName, 32 - 1);
  fprintf(vfp, "  %4s [%d : 0] Norm_in;\n", ConnectionName,1 - 1);

  const char dna [] = {'A', 'C', 'G', 'T'};
  int j;
  for (i = 0; i <= 3; i++)  {
    for (j = 0; j <= 3; j++)  {
      fprintf(vfp, "  %4s [31 : 0] PL_%c%c_in;\n", ConnectionName, dna[i], dna[j]);
      fprintf(vfp, "  %4s [31 : 0] PR_%c%c_in;\n", ConnectionName, dna[i], dna[j]);
    }
  }

  //--------------------------------------------------------------
  //
  //--------------------------------------------------------------
#endif


  fprintf(vfp, "  %4s          stall_in;\n", ConnectionName);

  fprintf(vfp, "  %4s          go_in;\n",    ConnectionName);

#ifdef DATA_PATH_ONLY
  
  fprintf(vfp, "  %4s          finish;\n",    ConnectionName);

  p = PortDeclList;
  while ( p != NULL) {
    fprintf(vfp, "  %s\n", p->s);
    p = p->next;
  }

#endif
}

void GenerateCounters(FILE *vfp, char *ModuleName, 
                      const char *SignalName1,
                      const char *SignalName2,
                      const char *SignalName3) 
{

  int i, j, m;
  char *opSym;
  enum operation op_type;
  int resourceNu;
  int dataWidth;

  const int MAX_OUTPUT_PORT_NUM = 1000;

  char sig_name[MAX_OUTPUT_PORT_NUM][100];
  char cnt_name[MAX_OUTPUT_PORT_NUM][100];
  int output_cnt = 0;
  char *find;

  // One output for each shared operation node
  // excl. (port nodes) 
  for (i = 0; i < NODE_NU; i++) {
    //
    // Assumption:
    //
    if (DFG[i]->op != nop && DFG[i]->opDest[0] < 0) {

      opSym = opSymTable(DFG[i]->op);
      resourceNu = DFG[i]->opResourceNu;

      sprintf(sig_name[output_cnt], "%s_%s_%s_%s",  \
         ModuleName, DFG[i]->opName, SignalName1, SignalName2);

      sprintf(cnt_name[output_cnt], "%s_%s_cnt",  ModuleName, DFG[i]->opName);

      output_cnt++;
    }
  }

  // A 32-bit counter 
  for (m = 0; m < output_cnt; m++) {
    fprintf(vfp, "reg [31:0] %s;\n", cnt_name[m]);
  }

  fprintf(vfp, "always @ (posedge clk) begin\n");
  fprintf(vfp, "  if (rst) begin\n");
  for (m = 0; m < output_cnt; m++) {
    fprintf(vfp, "  %s <= 0;\n", cnt_name[m]);
  }
  fprintf(vfp, "  end\n", output_cnt);
  fprintf(vfp, "  else begin\n");

  for (m = 0; m < output_cnt; m++) {
    fprintf(vfp, "  if (!stall && %s) begin\n", sig_name[m]);
    fprintf(vfp, "    %s <= %s + 1'b1;\n", cnt_name[m], cnt_name[m]);
    fprintf(vfp, "  end\n");
  }
  fprintf(vfp, "  end\n");
  fprintf(vfp, "end\n\n");

  fprintf(vfp, "always @ (posedge clk) begin\n");
  // e.g. fprintf(vfp, "  tmax_fadd1_out_rdy <= ~rst & tmax_fadd1_res_rdy;\n");
  for (m = 0; m < output_cnt; m++) {

    find = strstr(sig_name[m], "res");
    assert(find != NULL);
    *find = 'o'; *(find+1) = 'u'; *(find+2) = 't'; 
    fprintf(vfp, "  %s <= ~rst & ", sig_name[m]);

    find = strstr(sig_name[m], "out");
    assert(find != NULL);
    *find = 'r'; *(find+1) = 'e'; *(find+2) = 's'; 
    fprintf(vfp, "%s;\n", sig_name[m]);

    // remove _rdy 
    find = strstr(sig_name[m], "res");
    assert(find != NULL);
    *find = 'o'; *(find+1) = 'u'; *(find+2) = 't'; *(find+3)='\0';
    fprintf(vfp, "  %s <= ", sig_name[m]);

    find = strstr(sig_name[m], "out");
    assert(find != NULL);
    *find = 'r'; *(find+1) = 'e'; *(find+2) = 's'; 
    fprintf(vfp, "%s;\n", sig_name[m]);
  }
  fprintf(vfp, "end\n\n");


  // Restore SINK node's opDest[0]
  for (i = 0; i < NODE_NU; i++) {
    if (DFG[i]->opDest[0] == 0) 
      DFG[i]->opDest[0] = SINK;
  }

  //--------------------------------------------------------------------------
  // data path output assignment 
  // Note the number of outputs doesn't depend on the number of FUs
  //--------------------------------------------------------------------------
  for (i = 0; i < NODE_NU; i++) {
    if (DFG[i]->opDest[0] == SINK && DFG[i]->opName != NULL) {
      resourceNu = DFG[i]->opResourceNu;
      opSym = opSymTable(DFG[i]->op);

      fprintf(vfp, "assign %s_%s_res = ", ModuleName, DFG[i]->opName);
      fprintf(vfp, "%s%d_out;\n", opSym, resourceNu);

      // assign state ouput (unregistered) due to resource sharings
      fprintf(vfp, "assign %s_%s_res_rdy = ", ModuleName, DFG[i]->opName);
      fprintf(vfp, "state[%d] ", DFG[i]->opResultDoneSlot);
      fprintf(vfp, " & (%s_%s_cnt < TEST_NU);\n", ModuleName, DFG[i]->opName);  
    }
  }
}


void GenerateNamedOutputPorts(FILE *vfp, char *ModuleName, 
                         const char *ConnectionName,
                         const char *SignalName1,
                         const char *SignalName2,
                         const char *SignalName3) 
{

  int i, j;
  char *opSym;
  enum operation op_type;
  int ResourceNu;
  int dataWidth;

  // One output for each shared operation node
  // excl. (port nodes) 
  for (i = 0; i < NODE_NU; i++) {
    //
    // Assume the SINK node has name
    //
    if (DFG[i]->op != nop && DFG[i]->opDest[0] < 0 && DFG[i]->opName != NULL) {

      opSym = opSymTable(DFG[i]->op);
      dataWidth = DataPathWidth(DFG[i] -> op, 1);

      /* signed result for simulation */
      fprintf(vfp, "  %4s [%d : 0] %s_%s_%s%s;\n", 
         ConnectionName, dataWidth - 1, 
         ModuleName, DFG[i]->opName, SignalName1, SignalName3);

      // registered ready 
      fprintf(vfp, "  %4s          %s_%s_%s_%s%s;\n\n", // 10/4/11
         ConnectionName, ModuleName, DFG[i]->opName,
         SignalName1, SignalName2, SignalName3); 
    }
  }
}


void GenerateOutputPorts(FILE *vfp, char *ModuleName, 
                         const char *ConnectionName,
                         const char *SignalName1,
                         const char *SignalName2,
                         const char *SignalName3) 
{

  int i, j;
  char *opSym;
  enum operation op_type;
  int ResourceNu;
  int dataWidth;

  // One output for each shared operation node
  // excl. (port nodes) 
  for (i = 0; i < NODE_NU; i++) {
    //
    // Assumption:
    //
    if (DFG[i]->op != nop && DFG[i]->opDest[0] < 0) {
      opSym = opSymTable(DFG[i]->op);
      ResourceNu = DFG[i]->opResourceNu;
      dataWidth = DataPathWidth(DFG[i] -> op, 1);

      /* signed result for simulation */
      fprintf(vfp, "  %4s [%d : 0] %s_%s_%s%s;\n", 
         ConnectionName, dataWidth - 1, ModuleName, DFG[i]->opName,
         SignalName1, SignalName3);

      // registered ready 
      fprintf(vfp, "  %4s          %s_%s_%s_%s%s;\n\n", // 10/4/11
         ConnectionName, ModuleName, DFG[i]->opName, 
         SignalName1, SignalName2, SignalName3); 
    }
  }
}

void GenerateStallControl(FILE *vfp, int delay) {

  int i, j;
  enum operation op_type;
  int flag;
  int size;
  char *opSym;
  int resourceNu;

  //fprintf(vfp, "wire finish_enable;\n");
  //fprintf(vfp, "wire finish;\n");
  //fprintf(vfp, "FinishCounter #(%d, %d)\n", delay - minII + 1, IntLog2a(delay));
  //fprintf(vfp, "cnt_finish(clk, rst, stall, finish_enable, finish);\n\n");
  //fprintf(vfp, "assign finish_enable = run & ~(go_in | stall_in);\n\n");
  //fprintf(vfp, "assign stall = finish | stall_in");
  fprintf(vfp, "assign stall = stall_in");
  fprintf(vfp, ";\n\n");
}

/*
 	Phase 1: Data path Mux from shared registers to FU
  MuxCnt: the number of Muxes needed for this phase
  Return number of phase 1 MUX inputs
 */
int GeneratePhase1Mux(FILE *vfp, FILE *fp, int *Map, int sharedRegNu) {

  enum operation op_type;
  int fu_nu, port_nu;
  MuxPtr Mux, Mux_bk;
  int i, j, k, m;
  int dest;
  int size, offset = 0;
  int ml, p;
  int dest_cnt;
  int *dest_nu;
  int minEndTime;
  int totalMuxInputNu = 0;
  int maxMuxInputNu = 0;
  int tmpMuxInputNu;
  int impMuxInputNu; // implicit Mux input numbers
  int last, RF_PortNu, flag, endTime;
  int tmp;
  int dataWidth;
  int last_mux_grp;


  // check if the extra register has already been counted so that we don't
  // need to count it again.
  reg_st *check_mux_reg = (reg_st*) malloc (sizeof(reg_st) * sharedRegNu);
  for (i = 0; i < sharedRegNu; i++) {
    check_mux_reg[i].use = 0;
    memset(check_mux_reg[i].rfp, 0, 10);
  }

  for (op_type = add; op_type < add + OP_NU; op_type++) {

    dataWidth = DataPathWidth(op_type, 0);

    size = (*RQ[op_type])->size;
    for (fu_nu = 0; fu_nu < size; fu_nu++) {

      // 7/30/11
      int portNu = GetNodeSrcNu(op_type);

      for (port_nu = 0; port_nu < portNu; port_nu++) {

         Mux = Mux_bk = (MuxRAT[op_type][fu_nu]).Fu_portNu[port_nu];

         tmpMuxInputNu = 0;
         impMuxInputNu = 0;

         while (Mux != NULL) {
           totalMuxInputNu += 1;
           tmpMuxInputNu += 1;

           ml = 0;
           //fprintf(vfp, "wire s%d;\n", Mux->m);
           fprintf(vfp, "reg s%d;\n", Mux->m); // 9/4/12

           // wire [31 : 0] mux1_out;
           if (*(Mux->Mo->muxNu) != -1) 
             fprintf(vfp, "wire [%d : 0] mux%d_out;\n", dataWidth-1, *(Mux->Mo->muxNu));

           // wire [31 : 0] mux1_in0;


           for (i = 0; i < 2; i++) 
             // Multi-level Muxes
             if (*(Mux->Mi->muxNu + i) != -1) {
               fprintf(vfp, "wire [%d : 0] mux%d_in%d;\n", dataWidth-1, Mux->m, i);
             }

           fprintf(vfp, "MUX2 #(%0d) mux%d (\n", dataWidth, Mux->m);
           fprintf(vfp, "  .S(s%d),\n", Mux->m);

           for (i = 0; i < 2; i++) {
             #ifdef FLOATING_POINT
             if (dataWidth == 32)
               if (*(Mux->Mi->constNu + i) != 0 && *(Mux->Mi->muxNu + i) == -1)
                 fprintf(vfp, "  .I%d(32'h%08x), // %f\n", 
                           i + 1,  FloatingPointConvert(*(Mux->Mi->constNu + i)), 
                           *(Mux->Mi->constNu + i));

             if (dataWidth == 64)
               if (*(Mux->Mi->constNu_d + i) != 0 && *(Mux->Mi->muxNu + i) == -1)
                 fprintf(vfp, "  .I%d(64'h%016llx), // %lf\n", 
                           i + 1,  DoubleFloatingPointConvert(*(Mux->Mi->constNu_d + i)),
                           *(Mux->Mi->constNu_d + i));
             #else
               fprintf(vfp, "  .I%d(32'h%08x),\n", i + 1,  -1); 
             #endif
           }

           for (i = 0; i < 2; i++) 
             if ((*(Mux->Mi->constNu + i) == 0 && dataWidth == 32 ||
                  *(Mux->Mi->constNu_d + i) == 0 && dataWidth == 64) &&
                  *(Mux->Mi->muxNu + i) == -1) { //if (*(Mux->Mi->regNu + i) != -1)

               k = *(Mux->Mi->regNu + i);
               
               //====================================================================
               // A simple register or a one-port RF(shift register) whose read output
               // (serial shift output) has been assigned to a simple register
               //====================================================================
               if (DFG[k]->RegFileTable == NULL ||
                  (TotalRegFilePortNu(DFG[k]->RegFileTable, DFG[k]->opDestNu) == 1)) {

                 assert (Map[k] != 0);

                 if (Map[k] < 0) // nopc node for which register is not allocated
                  fprintf(vfp, "  .I%d(%s_in_r),\n", i + 1, GetCPortName(-Map[k]));

                 else if (MuxSelTable[Mux->m - 1]->addMuxReg) { // 9/1/2012
                   fprintf(vfp, "  .I%d(r%d_out_r),\n", i + 1, Map[k]);
                   if (check_mux_reg[Map[k]].use == 0) {
                     check_mux_reg[Map[k]].use = 1;
                     myprintf("Adding mux register %d\n", Map[k]);
                     CSP->mux_reg++;
                   }
                 }
                 else
                  fprintf(vfp, "  .I%d(r%d_out),\n", i + 1, Map[k]);

                 //--------------------------------------------------------------
                 // Collect Fanout (Register->MuxP input) of a simple register or the output of
                 // the serial shift output of a shift register.

                 //printf("calling r%d\n", Map[k]);
                 GetRegFanOut(Map, k, 1);
                 //--------------------------------------------------------------
               }
               else {
                 //====================================================================
                 // a multi-port RF
                 // 
                 // Count the number of destination(RF read ports)
                 // that share a MUX2 input. This is different from the number of RF
                 // read ports! Otherwise, dest_cnt is always > 1.
                 //====================================================================

                 dest_cnt = 0;
                 dest_nu  = NULL;

                 for (j = 0; j < DFG[k]->opDestNu; j++) {
                   dest = DFG[k]->opDest[j];
                   if (dest > 0) {
                     if (DFG[dest]->opSrc[port_nu] == k && 
                         DFG[dest]->opResourceNu == (fu_nu + 1) && 
                         DFG[dest]->op == op_type) {
                       dest_cnt++;
                       dest_nu = (int *) realloc(dest_nu, sizeof(int) * dest_cnt);
                       dest_nu[dest_cnt-1] = dest;
                     }
                   }
                 } // for 

                 if (dest_cnt > 1) {

                   flag = 0;
                   tmp = dest_cnt;

                   //-----------------------------------------------------------------------------
                   // Let's check if all the destinations of a node mapped to 
                   // the same RF port. If they are not mapped to the same port,
                   // we need additional Muxs (e.g. s1 ? d1 : s2 ? d2 : d3;) 
                   //
                   // NOTE
                   //
                   // There are two options for grouping the multiple 
                   // destinations of a node
                   //
                   // Opt1: They can be shared by the same port only when
                   // their death time are the same
                   //
                   // Opt2: When there is no conflict,  they can be shared
                   // by the same port 
                   // 
                   // In the end, Opt2 has fewer RF ports than Opt1 does.
                   // But Opt2 requires MUXs for the destinations to
                   // share the same port.
                   // Opt1 eliminates the MUXs as required for Opt2
                   //
                   //
                   // But it is possible that elimination of MUXs in Opt1
                   // will require more MUXs as more of them may be mapped to the
                   // same port.
                   //-----------------------------------------------------------------------------

                   last = RF_PortNu = GetRegFilePortNu(k, dest_nu[--dest_cnt]);
                   myprintf("parent=%d child = %d RF_port = %d\n", k, dest_nu[dest_cnt], RF_PortNu);
                   while(dest_cnt-- > 0) {
                     RF_PortNu = GetRegFilePortNu(k, dest_nu[dest_cnt]);
                     myprintf("parent=%d child = %d RF_port = %d\n", k, dest_nu[dest_cnt], RF_PortNu);
                     if (last != RF_PortNu) {
                       flag = 1;
                       myprintf("******* Multiple RF read data ports share a mux\n"); 
                     }
                     last = RF_PortNu;
                   }

                   if (!flag) {
                     // This implies that all the RF_PortNu is the same
                     assert (Map[k] != 0);
                      // 9/1/2012
                      if (MuxSelTable[Mux->m - 1]->addMuxReg) {
                        fprintf(vfp, "  .I%d(RF%d_rdata%d_r),\n", i + 1,  Map[k], RF_PortNu);
                        if (check_mux_reg[Map[k]].use == 0) {
                          check_mux_reg[Map[k]].use = 1;
                          check_mux_reg[Map[k]].rfp[RF_PortNu] = 1;
                          CSP->mux_reg++;
                          myprintf("Adding mux RF register %d\n", Map[k]);
                        } else if (check_mux_reg[Map[k]].rfp[RF_PortNu] == 0) {
                          check_mux_reg[Map[k]].rfp[RF_PortNu] = 1;
                          CSP->mux_reg++;
                          myprintf("Adding mux RF register %d\n", Map[k]);
                        }
                      }
                      else
                        fprintf(vfp, "  .I%d(RF%d_rdata%d),\n", i + 1,  Map[k], RF_PortNu);

                     //------------------------------------------------------
                     // Reg -> MuxP Fanout
                     GetRegFanOut(Map, k, RF_PortNu);
                     //------------------------------------------------------
                   }
                   else {
                     // There are no fanout from this implict mux chain as the chain is
                     // generated every time we have sharing of RF data ports
                     dest_cnt = tmp;
                     fprintf(vfp, "  .I%d(", i + 1);
                     last = RF_PortNu = GetRegFilePortNu(k, dest_nu[--dest_cnt]);
                     //endTime = DFG[dest_nu[dest_cnt]]->opScheduledSlot;
                     endTime = GetEndTime(dest_nu[dest_cnt]);

                     assert (Map[k] != 0);

                     fprintf(vfp, "state[%d] ? RF%d_rdata%d : ", endTime, Map[k], RF_PortNu); 

                     //------------------------------------------------------
                     // Reg -> MuxPQ Fanout

                     GetRegFanOut(Map, k, RF_PortNu);
                     //------------------------------------------------------

                     totalMuxInputNu += 1;
                     impMuxInputNu++;

                     while(dest_cnt-- > 0) {

                       RF_PortNu = GetRegFilePortNu(k, dest_nu[dest_cnt]);
                       //endTime = DFG[dest_nu[dest_cnt]]->opScheduledSlot;
                       endTime = GetEndTime(dest_nu[dest_cnt]);

                       if (last != RF_PortNu) {

                         assert (Map[k] != 0);

                         if (dest_cnt == 0)
                           fprintf(vfp, "RF%d_rdata%d),\n", Map[k], RF_PortNu); 
                         else
                           fprintf(vfp, "state[%d] ? RF%d_rdata%d : ", endTime, Map[k], RF_PortNu); 

                         //------------------------------------------------------
                         // Reg -> MuxPQ Fanout
                         GetRegFanOut(Map, k, RF_PortNu);
                         //------------------------------------------------------

                         totalMuxInputNu += 1;
                         impMuxInputNu++;

                         //---------------------------------------------------------------------
                         // Consider RF{x}_rdata{} may be the output of MUXs if Opt2 is adopted.
                         // If that is the case, then the implicit mux fan-in should include
                         // those MUXs as well. 
                         //
                         // In Opt1 all RF{x}_rdata{} are connected with
                         // registers.
                         //---------------------------------------------------------------------
                         RF_portPtr RegFileTable = DFG[k]->RegFileTable;
                         if (RegFileTable != NULL && RegFileTable[RF_PortNu-1].sharedCnt > 1)
                           impMuxInputNu += RegFileTable[RF_PortNu-1].sharedCnt;
                       }
                       last = RF_PortNu;
                     }
                     if (impMuxInputNu > maxMuxInputNu) maxMuxInputNu = impMuxInputNu;
                   } // else
                 } // if (dest_cnt > 1)
                 else {
                   assert (Map[k] != 0);

                   assert (dest_cnt = 1); 

                   // 9/1/2012
                   if (MuxSelTable[Mux->m - 1]->addMuxReg) {

                     RF_PortNu = GetRegFilePortNu(k, dest_nu[0]);
                     fprintf(vfp, "  .I%d(RF%d_rdata%d_r),\n", i + 1,  Map[k], RF_PortNu);
                     if (check_mux_reg[Map[k]].use == 0) {
                       check_mux_reg[Map[k]].use = 1;
                       check_mux_reg[Map[k]].rfp[RF_PortNu] = 1;
                       CSP->mux_reg++;
                       myprintf("Adding mux RF register %d\n", Map[k]);
                     } else if (check_mux_reg[Map[k]].rfp[RF_PortNu] == 0) {
                       check_mux_reg[Map[k]].rfp[RF_PortNu] = 1;
                       CSP->mux_reg++;
                       myprintf("Adding mux RF register %d\n", Map[k]);
                     }
                   }
                   else
                     fprintf(vfp, "  .I%d(RF%d_rdata%d),\n", i + 1, 
                              Map[k], GetRegFilePortNu(k, dest_nu[0]));

                   //------------------------------------------------------
                   // Reg -> MuxP Fanout
                   //------------------------------------------------------
                   GetRegFanOut(Map, k, GetRegFilePortNu(k, dest_nu[0]));
                 }

                 free(dest_nu);
               }
             }

           for (i = 0; i < 2; i++) 
             // Multi-level Muxes
             if (*(Mux->Mi->muxNu + i) != -1) {
               fprintf(vfp, "  .I%d(mux%d_in%d),\n", 
                       i + 1, Mux->m, i);
               ml = 1;
               p = i;
             }

           // Mux whose input is connected to FU will be generated in Phase 2

           if (*(Mux->Mo->muxNu) != -1) 
             fprintf(vfp, "  .Z(mux%d_out)\n);\n\n", *(Mux->Mo->muxNu));

           if (*(Mux->Mo->fuNu) != -1) {
             offset = 0; 
             //---------------------------------------------------------
             // Register insertion 7/9/11
             //---------------------------------------------------------
             //fprintf(vfp, "  .Z(%s%d_in%d)\n);\n\n", \
                     opSymTable(op_type), fu_nu + 1 + offset, port_nu);
             fprintf(vfp, "  .Z(%s%d_in%d_rin)\n);\n\n", 
                     opSymTable(op_type), fu_nu + 1 + offset, port_nu);
           }

           if (ml) { // connect mux output the next level mux input

             // mux chain register insertion 8/25/2012 
             // We know las_mux_grp is not unitialized after producing the root mux
             if (Mux->grp != last_mux_grp) {

               CSP->mux_reg++;

               fprintf(vfp, "reg [%d : 0] %s%d_in%d_rout_g%d;\n", dataWidth-1,
                     opSymTable(op_type), fu_nu + 1 + offset, port_nu, Mux->grp);

               fprintf(vfp, "always @ (posedge clk) begin\n");
               fprintf(vfp, "  if (!stall) %s%d_in%d_rout_g%d <= mux%d_out;\n",
                     opSymTable(op_type), fu_nu + 1 + offset, port_nu, Mux->grp, Mux->m);
               fprintf(vfp, "end\n\n");

               fprintf(vfp, "assign mux%d_in%d = %s%d_in%d_rout_g%d;\n\n", Mux->m, p, 
                     opSymTable(op_type), fu_nu + 1 + offset, port_nu, Mux->grp);
             }
             else  
               fprintf(vfp, "assign mux%d_in%d = mux%d_out;\n\n", Mux->m, p, Mux->m);
           }

           last_mux_grp = Mux->grp;
           Mux = Mux->next;
         }


         // For cascaded muxes, the last one has two input ports.
         if (tmpMuxInputNu != 0) {
           totalMuxInputNu += 1; 
           tmpMuxInputNu += 1;
           myprintf("MUXP fu_op=%c fu_num =%d fu_port=%d : %d-1 Mux\n", 
               GetOpSym(op_type), fu_nu, port_nu, tmpMuxInputNu);
           if (tmpMuxInputNu > maxMuxInputNu) maxMuxInputNu = tmpMuxInputNu;
         }
      }
    }
  }

  free(check_mux_reg);

  //------------------------------------------------------------
  // Stats
  //------------------------------------------------------------

  myfprintf(fp, "Total MUXP inputs = %d (max %d)\n", 
      totalMuxInputNu, maxMuxInputNu);

  //CSP->muxp = totalMuxInputNu;

  //CSP->mux += totalMuxInputNu;
  //if (maxMuxInputNu > CSP->fan) CSP->fan = maxMuxInputNu; 

  //return totalMuxInputNu;
  return maxMuxInputNu;
}

/* 
  Phase 2: Data path Mux from FU to shared registers
  Return number of phase 2 MUX inputs

  wire s($muxNuCnt)[$(muxSelWidth - 1) : 0];
	MUX($inputNu) mux($muxNuCnt)(
	 .S(s($muxNuCnt)),
	 .I($inputNu)(($opSym)($nu)_out),
	 .Z(r($regNu)_in));
*/
int GeneratePhase2Mux(FILE *vfp, FILE *fp, int SharedRegNu, int *MuxCnt, int *MuxCntTable) {
  int muxInputNu;
  int regNu;
  double fval;
  int ival;
  int muxSelWidth;
  ResourceList fu, head;
  int totalMuxInputNu = 0;
  int maxMuxInputNu = 0;
  int dataWidth = 32;
  int lastDataWidth;

  for (regNu = 1; regNu <= SharedRegNu; regNu++) {
    muxInputNu = 0;
    fu = head = FuRAT[regNu];

    //----------------------------------------------------
    // Assume data width of each FU's output is the same,
    // which allows same-width register sharing in register
    // allocation method.
    //----------------------------------------------------
    while (fu != NULL) {
      lastDataWidth = dataWidth;
      dataWidth = DataPathWidth(fu->op, 1);
      if (muxInputNu > 1) 
        assert(dataWidth == lastDataWidth);
      fu = fu->next;
      muxInputNu++;
    }

    if (muxInputNu >= 2) {

      // Get max number of inputs of a MUX
      if (muxInputNu > maxMuxInputNu) maxMuxInputNu = muxInputNu;

      (*MuxCnt)++;

      myprintf("MUXR mux%d reg_num=%d : %d-1 Mux\n", *MuxCnt, regNu, muxInputNu);

      totalMuxInputNu += muxInputNu;

      muxSelWidth = IntLog2(muxInputNu);
      MuxCntTable[regNu] = *MuxCnt;
      
      //fprintf(vfp, "wire [%d : 0] s%d;\n",  muxSelWidth - 1, *MuxCnt);
      fprintf(vfp, "reg [%d : 0] s%d;\n",  muxSelWidth - 1, *MuxCnt); // 9/4/12

      fprintf(vfp, "MUX%d #(%0d) mux%d (\n", muxInputNu, dataWidth, *MuxCnt);
      fprintf(vfp, "  .S(s%d),\n", *MuxCnt);

      fu = head;
      muxInputNu = 0;
      while (fu != NULL) {
        muxInputNu++;

        if (fu->op == nop) {
          fprintf(vfp, "  .I%d(p%d_in_r),\n",  // registered input
                  muxInputNu, GetInputPortNu(fu->nu));

          //---------------------------------------------------------
          // Registered port fanout (port -> MuxP)
          //---------------------------------------------------------
          PortFanout[GetInputPortNu(fu->nu) - 1]++;
        }

        else if (fu->op == nopc) {
          fprintf(vfp, "  .I%d(%s_in_r),\n",  // registered input
                  muxInputNu, GetCPortName(head->nu));

          //---------------------------------------------------------
          // Registered port fanout (port -> MuxP)
          // Not yet implemented
          //---------------------------------------------------------
        }

        else {
          fprintf(vfp, "  .I%d(%s%d_out),\n", 
                  muxInputNu, opSymTable(fu->op), fu->nu);
          //---------------------------------------------------------
          // FU output -> MuxR input Fanout
          //---------------------------------------------------------
          FuFanout[fu->op][fu->nu - 1]++;
        }

        fu = fu->next;
      } // while
      
      fprintf(vfp, "  .Z(r%d_in)\n);\n\n", regNu);

    } // muxInputNu >= 2
  }

  //------------------------------------------------
  // Stats
  //------------------------------------------------
  //myfprintf(fp, "Total MUXR inputs = %d (max %d)\n", \
      totalMuxInputNu, maxMuxInputNu);

  //fprintf(stderr, "Total MUXR inputs = %d (max %d)\n", 
      //totalMuxInputNu, maxMuxInputNu);
  //CSP->muxr = totalMuxInputNu;

  //CSP->mux += totalMuxInputNu;
  //if (maxMuxInputNu > CSP->fan) CSP->fan = maxMuxInputNu; 

  return maxMuxInputNu;
}


//===================================================================================
/* 
 * Descriptions of FU instantiation *
 *   fadd fadd1 (
 *     .clk(clk), 
 *     .a(), 
 *     .b(), 
 *     .go(), 
 *     .pipeEn(),
 *     .result(), 
 *     .rdy());

 *   fdiv fdiv1 (
 *     .clk(clk), 
 *     .a(), 
 *     .b(), 
 *     .go(), 
 *     .result(), 
 *     .done());
 */

// 6/22/2011
//
// During FU generation, we also setup a fanout table for each FU
// 
// size: The size of the table (i.e. number of operations);
// depth:The depth of each operation (i.e. number of functional unit of that type)
// 
// *Assumption*
// 2-1 MUX only, 4-input ROM only ===================================================================================
void GenerateFU(FILE *vfp) {
  
  char *opSym;
  enum operation op_type;
  int size;
  int i;
  int tmpFuNuCnt = 0;;
  static int FuNuCnt;
  int dataWidth;

  //------------------------------------------------------------------------
  // Fanout for each type in the DFG
  //------------------------------------------------------------------------
  FuFanout = (int **) malloc (sizeof(int *) * OP_NU);

  for (op_type = add; op_type < add + OP_NU; op_type++) {

    // FU input data width
    dataWidth = DataPathWidth(op_type, 0);

    FuNuCnt = 0;

    size = (*RQ[op_type])->size;

    //------------------------------------------------------------------------
    // Fanout for each FUs of the same type
    //------------------------------------------------------------------------
    FuFanout[op_type] = (int *) calloc (size, sizeof(int));

    //------------------------------------------------------------------------
    // Functional Units (No FU selection)
    //------------------------------------------------------------------------
    for (i = 0; i < size; i++) {
      //----------------------
      // begin of instantiation
      //----------------------
      FuNuCnt++;
      opSym = opSymTable(op_type);

      if (op_type == tod || op_type == tof) { // single to double conversion i/o width specified
        fprintf(vfp, "wire [%d : 0] %s%d_in0;\n", dataWidth-1, opSym, FuNuCnt);
        fprintf(vfp, "wire [%d : 0] %s%d_out;\n", DataPathWidth(op_type, 1)-1, opSym, FuNuCnt);
        fprintf(vfp, "%s %s%d (\n", opSym, opSym, FuNuCnt);  // latency 
        fprintf(vfp, "   .a(%s%d_in0),\n", opSym, FuNuCnt);
        fprintf(vfp, "   .result(%s%d_out)\n", opSym, FuNuCnt);
      }

      else if (op_type == mx || op_type == mxd) { // This is a 2-1 MUX as a FU
      //else if (!strcmp(opTypeTable(op_type), "select")) { 

        fprintf(vfp, "wire [%d : 0] %s%d_in0;\n", dataWidth-1, opSym, FuNuCnt);
        fprintf(vfp, "wire [%d : 0] %s%d_in1;\n", dataWidth-1, opSym, FuNuCnt);
        fprintf(vfp, "wire [%d : 0] %s%d_sel;\n", 0, opSym, FuNuCnt); // special input
        fprintf(vfp, "wire [%d : 0] %s%d_out;\n", dataWidth-1, opSym, FuNuCnt);
        fprintf(vfp, "MUX2 #(%0d) %s%d (\n", dataWidth, opSym, FuNuCnt);
        fprintf(vfp, "   .I1(%s%d_in0),\n", opSym, FuNuCnt);
        fprintf(vfp, "   .I2(%s%d_in1),\n", opSym, FuNuCnt);
        fprintf(vfp, "   .S(%s%d_sel),\n", opSym, FuNuCnt);
        fprintf(vfp, "   .Z(%s%d_out)\n", opSym, FuNuCnt);

      } else if (op_type == rom || op_type == romd) { 

        // A 4-bit address ROM with addr[3:0] = {a3, a2, a1, a0}

        fprintf(vfp, "wire [%d : 0] %s%d_in0;\n", dataWidth-1, opSym, FuNuCnt);
        fprintf(vfp, "wire [%d : 0] %s%d_in1;\n", dataWidth-1, opSym, FuNuCnt);
        fprintf(vfp, "wire [%d : 0] %s%d_in2;\n", dataWidth-1, opSym, FuNuCnt);
        fprintf(vfp, "wire [%d : 0] %s%d_in3;\n", dataWidth-1, opSym, FuNuCnt);
        fprintf(vfp, "wire [%d : 0] %s%d_out;\n", DataPathWidth(op_type, 1)-1, opSym, FuNuCnt);
        fprintf(vfp, "ROM #(%d, %d) %s%d (\n", DataPathWidth(op_type, 1), DEPTH, opSym, FuNuCnt);
        fprintf(vfp, "   .clk(clk),\n");
        fprintf(vfp, "   .a0(%s%d_in0),\n", opSym, FuNuCnt);
        fprintf(vfp, "   .a1(%s%d_in1),\n", opSym, FuNuCnt);
        fprintf(vfp, "   .a2(%s%d_in2),\n", opSym, FuNuCnt);
        fprintf(vfp, "   .a3(%s%d_in3),\n", opSym, FuNuCnt);
        fprintf(vfp, "   .data(%s%d_out)\n", opSym, FuNuCnt);
      }
      else 
      {

        fprintf(vfp, "wire [%d : 0] %s%d_in0;\n", dataWidth-1, opSym, FuNuCnt);
        fprintf(vfp, "wire [%d : 0] %s%d_in1;\n", dataWidth-1, opSym, FuNuCnt);
        fprintf(vfp, "wire [%d : 0] %s%d_out;\n", DataPathWidth(op_type, 1)-1, opSym, FuNuCnt);
        fprintf(vfp, "wire          %s%d_go;\n",  opSym, FuNuCnt);
        fprintf(vfp, "wire          %s%d_rdy;\n", opSym, FuNuCnt);

        fprintf(vfp, "%s %s%d (\n", opSym, opSym, FuNuCnt);
        fprintf(vfp, "   .clk(clk),\n");
        fprintf(vfp, "   .a(%s%d_in0),\n", opSym, FuNuCnt);
        fprintf(vfp, "   .b(%s%d_in1),\n", opSym, FuNuCnt);

      //fprintf(vfp, "   .go(%s%d_go),\n", opSym, FuNuCnt);
        fprintf(vfp, "   .go(1'b1),\n", opSym, FuNuCnt);

        fprintf(vfp, "   .result(%s%d_out),\n", opSym, FuNuCnt);
        fprintf(vfp, "   .pipeEn(~stall),\n", opSym, FuNuCnt);

        if (strcmp(opSym, "fdiv") && strcmp(opSym, "fdivd")) {
          fprintf(vfp, "   .rdy(%s%d_rdy)\n", opSym, FuNuCnt);
        }
        else {
          // div output "done" signal is not implemented in float.vhd
          fprintf(vfp, "   .done(%s%d_rdy)\n", opSym, FuNuCnt); 
        }

      }
      fprintf(vfp, ");\n\n"); 
    }
    myprintf("Generate %d non-port FUs of type %d\n", FuNuCnt - tmpFuNuCnt, op_type);
  }
}

//-----------------------------------------------------------------------------
// Generate a register.
//
// Table look-up may be used to access the signal char names
//
// RegisterFile numbers are based on the shared number 
// (i.e. not necessarily starting from 1)
//
// ----------------------------------------------------------------------
// 6/21/2011
//
// During register generation, we also setup a fanout table for each shared register.
// 
// size: The size of the table (i.e. number of shared registers, SharedRegNu);
// depth:The depth of each shared regsister. 
// 
// *Assumption*
//
// FU input register always has fanout equal 1. So it is not considered.
//
// The register entry (in the range [0, depth)) of a shift register at Fanout 
// table entry X (in the range [0, size) is equivalent to the significance 
// of the registers in the shift register 
//
// e.g. suppose a N-depth shift register
//
// Fanout[X].entry[0] corresponds to the fanout of the least significant register,
// which is the input of the shift register X.
//
// Fanout[X].entry[N] corresponds to the fanout of most significant register, 
// which is the output of the shift register X. 
//
// ----------------------------------------------------------------------
// setup a Mux fanout table
//
// size: SharedRegNu
// Depth: RF_portNu
//
// Observation

// 1. Both MuxR and MuxP have fanout 1, so they are not considered here.
// 2. Since RMux is an implicit register mux chain, the name of the output
// signal, r{x}_out or RF{x}_out{x}, are connected to the inputs of 
// MuxP(s) and FU(s).
// In this case, they are actually the fanout of the implicit Muxs.
//
// Assumption
// 
// Here register number and port number are used to represent the 
// Mux fannout, which is a little confusing.
// 
// FMUX as a FU is not considered here
// -----------------------------------------------------------------------------
int GenerateRegisters(FILE *vfp, FILE *fp, int *Map, char *ModuleName, int SharedRegNu) {

  int regNu;
  int dataWidth;
  int i, j;
  int resourceNu;
  int map, last_map;
  char *opSym;
  int lifeTime, addrWidth, dataDepth, cntVal;
  varList p;
  int destNu, endTime_i, endTime_j, RF_PortNu;
  int parent_ID, child_ID;
  RF_portNuPtr RegFileList;
  RF_portPtr RegFileTable;

  int totalRegNu = 0;
  int totalRegMuxInput = 0;
  int maxRegMuxInput = 0;
  
  // Set up Register Fanout table
  RegFanout = (FanoutR_ptr) malloc (sizeof(FanoutR) * SharedRegNu);

  // Set up Mux Fanout table
  //MuxFanout = (FanoutR_ptr) malloc (sizeof(FanoutR) * SharedRegNu);


  for (regNu = 0; regNu < SharedRegNu; regNu++) {

    
    /* Observation: 
     *
     * 1. For a node with multiple destinations, only the path with
     * maximum endtime is inserted into the VarList. This means
     * all the destination paths (branches) from the multi-destination 
     * node are merged into a single path.
     * 
     * 2. Regardless of the number of destinations of a node, 
     * if a variable's lifeTime is greater than minII(i.e. always overlapping), 
     * it becomes a single shared register without sharing
     * with any other variables. So there is only one variable in the 
     * corresponding varTable's entry. 
     *
     * 3. (1, 2) means it is not possible to share any branch(path) of a 
     * multi-destination node with other variables.
     * 
     */

    p = VarTable[regNu];

    dataWidth = DataPathWidth(DFG[p->varID]->op, 1);
          
    while (p != NULL) {

      // add 12/22
      lifeTime = p->endTime - p->startTime;

      // 10000 is set in LeftEdge.c
      if (lifeTime > minII && p->endTime != 10000) {

        //---------------------------------------------------------------------------
        // Prepare data for generating multiple-port Register File
        // 
        // Some questions to think about:
        //
        // Is start time the same for a node with multiple destinations regardless
        // of the scheduling method?  Yes.
        //
        //---------------------------------------------------------------------------

        //cntVal = (lifeTime / minII);
        cntVal = (lifeTime / minII) - ((lifeTime % minII) ? 0 : 1);

        //dataDepth = 1 << addrWidth;
        dataDepth = (lifeTime / minII) + ((lifeTime % minII) ? 1 : 0);

        // Set up Fanout table entry
        (RegFanout[regNu]).entry = (int *) calloc (dataDepth, sizeof(int));
        (RegFanout[regNu]).depth = dataDepth;

        //---------------------------------------------------------------------------
        // i.e. number of registers in a delay line
        //---------------------------------------------------------------------------
        totalRegNu += dataDepth;

        addrWidth = IntLog2(dataDepth);

        //---------------------------------------------------------------------------
        // Get node ID 
        //---------------------------------------------------------------------------
        parent_ID = p->varID;

        //---------------------------------------------------------------------------
        // Node's number of destinations
        //---------------------------------------------------------------------------
        destNu = DFG[parent_ID]->opDestNu;

        //---------------------------------------------------------------------------
        // Create RegFileTable for any nodes with lifetime > minII
        //---------------------------------------------------------------------------
        RegFileTable = DFG[parent_ID]->RegFileTable = 
          (RF_portPtr) malloc (sizeof(RF_NuCnt) * destNu);

        //---------------------------------------------------------------------------
        // Initialize RegFileTable for the node
        //---------------------------------------------------------------------------
        for (i = 0; i < destNu; i++) {
          RegFileTable[i].RegFileList = NULL;
          RegFileTable[i].sharedCnt = 0;
        }

        //---------------------------------------------------------------------------
        // Insert child_ID nodes (i.e. destinations) of a parent node to RegFileTable 
        //---------------------------------------------------------------------------
        for (i = 0; i < destNu; i++) {
          child_ID = *(DFG[parent_ID]->opDest + i);
          if (child_ID == SINK) continue;  // 10/9/11
          RF_PortNu = AddRegFilePortNu(RegFileTable, child_ID, i);
          RegFileTableCheckInsert(RegFileTable, RF_PortNu, child_ID);
        }

        // Debug RegFileList
        myprintf("node %d:\n", parent_ID);
        PrintRegFileTable(RegFileTable, destNu);

        //---------------------------------------------------------------------------
        // Collect Mux inputs 
        //---------------------------------------------------------------------------
        if (RegFileTable != NULL) {
          for (i = 0; i < destNu; i++) {
            if (RegFileTable[i].RegFileList != NULL) {
              if (RegFileTable[i].sharedCnt > 1) {
                totalRegMuxInput += RegFileTable[i].sharedCnt;
                if (RegFileTable[i].sharedCnt > maxRegMuxInput)
                  maxRegMuxInput = RegFileTable[i].sharedCnt;
              }
            }
          }
        }

        //------------------------------------------------------------
        // Based on observation 2, no duplicate of a register file
        // will be generated

        // Obsolete function
        // GenerateRegisterFile(vfp, addrWidth, dataWidth, dataDepth, 
        //                     cntVal, regNu + 1, RegFileTable, destNu);
        //------------------------------------------------------------
        GenerateShiftRegister (vfp, fp, addrWidth, dataWidth, dataDepth, 
                             cntVal, regNu + 1, RegFileTable, destNu);

#ifdef DATA_PATH_ONLY
        GenerateRegFileEnablePort(regNu + 1, RegFileTable, destNu);
#endif

      } 
      else { 

        //-------------------------------------------------------------
        // lifetime <= minII
        // A simple register whose depth is 1
        //-------------------------------------------------------------
        totalRegNu += 1;

        dataDepth = 1;

        // Set up Fanout table entry
        (RegFanout[regNu]).entry = (int *) calloc (dataDepth, sizeof(int));
        (RegFanout[regNu]).depth = dataDepth;

        GenerateRegister(vfp, dataWidth, regNu + 1);

#ifdef DATA_PATH_ONLY
        GenerateRegEnablePort(regNu + 1);
#endif

        break;  // no duplicate of a simple register  
      }
      p = p->next;
    }
  }


  fprintf(vfp, "\n");

  //------------------------------------------------
  // Stats
  //------------------------------------------------
  myfprintf(fp, "Number of register units : %d\n", SharedRegNu);
  myfprintf(fp, "Number of all registers  : %d\n", totalRegNu);
  myfprintf(fp, "Total Reg Mux inputs %d (max %d)\n", totalRegMuxInput, maxRegMuxInput);

  //------------------------------------------------------------
  // Stats
  //------------------------------------------------------------
  //CSP->reg = totalRegNu;
  //if (maxRegMuxInput > CSP->fan) CSP->fan = maxRegMuxInput; 
  //CSP->mux += totalRegMuxInput;

  return totalRegNu;
}

//-----------------------------------------------------------------------------
// Wire each shared register's output which are directly connected to the FU 
//-----------------------------------------------------------------------------
void GeneratePhase1Wire(FILE *vfp, FILE *fp, int *Map) {

  enum operation op_type;
  int fu_nu, port_nu;
  int i, j;
  int size, offset;
  int dest;
  MuxPtr Mux;

  int totalMuxInputNu = 0;
  int maxMuxInputNu = 0;
  int impMuxInputNu;
  int portNu;

  int dest_cnt;
  int *dest_nu  = NULL;
  int m;
  int last, RF_PortNu, flag, endTime;
  int tmp;
  int lastConstVal;
  long lastConstVal_d;

  for (op_type = add; op_type < add + OP_NU; op_type++) {
    size = (*RQ[op_type])->size; // number of FUs for each operation type 

    for (fu_nu = 0; fu_nu < size; fu_nu++) {

      // 7/30/11
      portNu = GetNodeSrcNu(op_type);

      for (port_nu = 0; port_nu < portNu; port_nu++) { // two ports for each FU

         Mux = (MuxRAT[op_type][fu_nu]).Fu_portNu[port_nu];

         if (Mux == NULL) {  // Indicate FU port is not yet connected

           // FU number offset (obsolete)
           offset = 0; 

           //------------------------------------------------------------------------
           // Debug 
           //------------------------------------------------------------------------
           for (i = 0; i < NODE_NU; i++) {
             if (DFG[i]->opSrc[0] < 0)
               myprintf("i: %d type: %c resource Nu: %d src0: %d src1: null dest: %d\n", 
                     i, GetOpSym(DFG[i]->op), DFG[i]->opResourceNu, 
                     DFG[i]->opSrc[0], DFG[i]->opDest[0]);
             else {
               if (DFG[i]->op == mx || DFG[i]->op == mxd)
                 myprintf("i: %d type: %c resource Nu: %d src0: %d src1: %d  src2: %d dest: %d\n", \
                       i, GetOpSym(DFG[i]->op), DFG[i]->opResourceNu, \
                       DFG[i]->opSrc[0], DFG[i]->opSrc[1], DFG[i]->opSrc[2], DFG[i]->opDest[0]);
               else
                 myprintf("i: %d type: %c resource Nu: %d src0: %d src1: %d dest: %d\n", \
                       i, GetOpSym(DFG[i]->op), DFG[i]->opResourceNu, \
                       DFG[i]->opSrc[0], DFG[i]->opSrc[1], DFG[i]->opDest[0]);
             }
           }

           //------------------------------------------------------------------------
           // Find all the shared registers which are connected to each FU's input port 
           //
           // We have two cases to consider here:
           // 1. A node with just one destination 
           // 2. A node with multiple destinations
           //------------------------------------------------------------------------
           lastConstVal = 0;
           lastConstVal_d = 0;
           for (i = 0; i < NODE_NU; i++) {
             dest_cnt = 0;
             dest_nu  = NULL;

             for (j = 0; j < DFG[i]->opDestNu; j++) {

               dest = DFG[i]->opDest[j];

               if (dest > 0) {

                 if (port_nu < DFG[dest]->opSrcNu && DFG[dest]->opSrc[port_nu] == i &&  // 4/12/12
                     DFG[dest]->opResourceNu == (fu_nu + 1 + offset) && 
                     DFG[dest]->op == op_type) {

                   //-----------------------------------------------------------------
                   // I. assign constant number to a FU's port input (excl. fmux)
                   //-----------------------------------------------------------------
                   if ((DFG[dest]->opConst && port_nu == 1)) {
                     
                     if (DFG[dest]->opPrecision == sfp) {
                       if (lastConstVal != FloatingPointConvert(DFG[dest]->opConstVal)) {
                         //============================================================
                         // Register insertion in front of any FUs
                         //============================================================
                          fprintf(vfp, "assign %s%d_in%d_rin = 32'h%08x;\n",   \
                                  opSymTable(op_type), fu_nu + 1 + offset, port_nu, \
                                  FloatingPointConvert(DFG[dest]->opConstVal));

                         if (DFG[dest]->opPrecision == sfp) 
                          lastConstVal = FloatingPointConvert(DFG[dest]->opConstVal);
                       }
                     }

                     if (DFG[dest]->opPrecision == dfp) {
                       if (lastConstVal_d != DoubleFloatingPointConvert(DFG[dest]->opConstVal_d)) {
                         //============================================================
                         // Register insertion in front of any FUs
                         //============================================================
                          fprintf(vfp, "assign %s%d_in%d_rin = 64'h%016llx;\n",   \
                                  opSymTable(op_type), fu_nu + 1 + offset, port_nu, \
                                  DoubleFloatingPointConvert(DFG[dest]->opConstVal_d));

                         if (DFG[dest]->opPrecision == dfp) 
                           lastConstVal_d = DoubleFloatingPointConvert(DFG[dest]->opConstVal_d);
                       }
                     }

                     //fprintf(vfp, "assign %s%d_in%d = 32'h%08x;\n", \
                               opSymTable(op_type), fu_nu + 1 + offset, port_nu, -1);

                   } else {
                   //-----------------------------------------------------------------
                   // register or mux output as FU input
                   //-----------------------------------------------------------------

                     if (DFG[i]->RegFileTable == NULL ||
                        (TotalRegFilePortNu(DFG[i]->RegFileTable, DFG[i]->opDestNu) == 1)) {
                       //------------------------------------------------------------------------
                       // case 1: a simple register, not a regfile.
                       // case 2: an one-port regfile. Here the node may have one destination or 
                       // multiple destinations. The read time (end time) of each destination is 
                       // not necessarily the same for the multiple case.
                       // 
                       // II. For both cases, assign r{x}_out to a FU's port input
                       //------------------------------------------------------------------------

                       if (DFG[i]->op != nopc) assert (Map[i] != 0);

                       myprintf("Phase1Wire reg: i = %d Map[i] = %d dest = %d \n",i, Map[i], dest);

                      //============================================================
                      // Register insertion in front of any FUs
                      //============================================================

                       // fprintf(vfp, "assign %s%d_in%d = r%d_out;\n", \
                                 opSymTable(op_type), fu_nu + 1 + offset, port_nu, Map[i]);
                          
                      //============================================================
                      // 7/27/11 add mux condition
                      // 8/4/11  add nopc condition
                      //============================================================
                         if (op_type == mx || op_type == mxd) {
                           if (port_nu == 0)  {// actually mux select
                             if (DFG[i]->op == nopc) {
                             fprintf(vfp, "assign %s%d_sel_rin = %s_in_r;\n",   \
                                     opSymTable(op_type), fu_nu + 1 + offset, \
                                     GetCPortName((int)(DFG[i]->opConstVal)));
                             }
                             else
                             fprintf(vfp, "assign %s%d_sel_rin = r%d_out;\n",   \
                                     opSymTable(op_type), fu_nu + 1 + offset, Map[i]);
                           }
                           else {
                             if (DFG[i]->op == nopc) // a regisered port 
                             fprintf(vfp, "assign %s%d_in%d_rin = %s_in_r;\n",   \
                                     opSymTable(op_type), fu_nu + 1 + offset, port_nu - 1, \
                                     GetCPortName((int)(DFG[i]->opConstVal)));
                             else
                             fprintf(vfp, "assign %s%d_in%d_rin = r%d_out;\n",   \
                                     opSymTable(op_type), fu_nu + 1 + offset, port_nu - 1, Map[i]);
                           }
                         }
                         else {
                           if (DFG[i]->op == nopc)
                           fprintf(vfp, "assign %s%d_in%d_rin = %s_in_r;\n",   \
                                   opSymTable(op_type), fu_nu + 1 + offset, port_nu, \
                                   GetCPortName((int)(DFG[i]->opConstVal)));
                           else
                           fprintf(vfp, "assign %s%d_in%d_rin = r%d_out;\n",   \
                                   opSymTable(op_type), fu_nu + 1 + offset, port_nu, Map[i]);
                         }

                       //fprintf(vfp, "assign %s%d_in%d = %s%d_in%d_rout;\n",   \
                               opSymTable(op_type), fu_nu + 1 + offset, port_nu, \
                               opSymTable(op_type), fu_nu + 1 + offset, port_nu);

                       //-------------------------------------------------------------------------
                       // Collect Fanout (Register->FU input) of a simple register or the output of
                       // the serial shift output.
                       //printf("calling r%d\n", Map[i]);
                       if (DFG[i]->op != nopc) GetRegFanOut(Map, i, 1);
                       //-------------------------------------------------------------------------

                       break; // break loop for (j = 0; j < DFG[i]->opDestNu; j++)
                     }

                     else {
                       //------------------------------------------------------------------------
                       // Collect the destination ids of node i for a multi-port regfile
                       //------------------------------------------------------------------------
                       myprintf("Phase1Wire RF: i = %d dest = %d \n",i, dest);
                       dest_cnt++;
                       dest_nu = (int *) realloc(dest_nu, sizeof(int) * dest_cnt);
                       dest_nu[dest_cnt-1] = dest;
                     } // multiple regfile ports
                   } // not a constant Mux input

                 } // shared registers connected to a FU
               } // if (dest > 0)
             } // for (j = 0; j < DFG[i]->opDestNu; j++)

             if (dest_cnt > 0) {

               if (dest_cnt > 1) {
                   //---------------------------------------------------------------------------- 
                   // Generate implicit multiplexer (i.e. State[x] ? a1 : a2;
                   //---------------------------------------------------------------------------- 

                 flag = 0;
                 /* old implmentation
                 // generate implicit multiplexer (i.e. e1 ? a1 : e2 ? a2 : 'bx;) 
                 fprintf(vfp, "assign %s%d_in%d = ",
                               opSymTable(op_type), fu_nu + 1 + offset, port_nu);
                 for (m = 0; m < dest_cnt; m++) {
                   GenerateRegFilePortMux(vfp, Map, i, dest_nu[m]);
                 }
                 fprintf(vfp, "32'bx;\n");
                 */

                 //----------------------------------------------------------------------
                 // Check if multiple RF read ports share a FU input thru MUXs
                 // Note that there are multiple destinations connected to the FU's port input
                 //----------------------------------------------------------------------
                 tmp = dest_cnt;

                 // for one destination of node i
                 last = RF_PortNu = GetRegFilePortNu(i, dest_nu[--dest_cnt]);

                 // for other destination(s) of node i
                 while(dest_cnt-- > 0) {
                   RF_PortNu = GetRegFilePortNu(i, dest_nu[dest_cnt]);
                   if (last != RF_PortNu) {
                     myprintf("******* Multiple RF read data ports actually share a FU input port\n"); 
                     flag = 1;
                   }
                   last = RF_PortNu;
                 }

                 //----------------------------------------------------------------------
                 // III No. Simply assign the output to the FU's port input
                 //----------------------------------------------------------------------
                 if (!flag) {

                   //------------------------------------------------------
                   // Register insertion 
                   //------------------------------------------------------
                   assert (Map[i] != 0);

                     //fprintf(vfp, "assign %s%d_in%d = RF%d_rdata%d;\n",\
                                   opSymTable(op_type), fu_nu + 1 + offset, port_nu,\
                                   Map[i], RF_PortNu);

                   if (op_type == mx || op_type == mxd) {
                     if (port_nu == 0)  // actually mux select
                       fprintf(vfp, "assign %s%d_sel_rin = RF%d_rdata%d;\n",   \
                               opSymTable(op_type), fu_nu + 1 + offset, \
                               Map[i], RF_PortNu);
                     else
                       fprintf(vfp, "assign %s%d_in%d_rin = RF%d_rdata%d;\n",   \
                               opSymTable(op_type), fu_nu + 1 + offset, port_nu - 1, \
                               Map[i], RF_PortNu);
                   }
                   else
                     fprintf(vfp, "assign %s%d_in%d_rin = RF%d_rdata%d;\n",\
                                   opSymTable(op_type), fu_nu + 1 + offset, port_nu,\
                                   Map[i], RF_PortNu);

                   //fprintf(vfp, "assign %s%d_in%d = %s%d_in%d_rout;\n",   \
                           opSymTable(op_type), fu_nu + 1 + offset, port_nu, \
                           opSymTable(op_type), fu_nu + 1 + offset, port_nu);

                   //------------------------------------------------------
                   // Reg -> MuxP Fanout
                   GetRegFanOut(Map, i, RF_PortNu);
                   //------------------------------------------------------
                 } else {

                   //----------------------------------------------------------------------
                   // III Yes. Multiple RF read data ports share a FU input thru MUXs.
                   // For simplicity, allocate implict MUXs using ? operator
                   //
                   // So there are no fanout from this implict mux chain as the chain is
                   // generated every time we have sharing of RF data ports
                   //----------------------------------------------------------------------
                   impMuxInputNu = 0; // max fan-in
                   dest_cnt = tmp;

                   //------------------------------------------------------
                   // Register insertion 
                   //------------------------------------------------------
                     //fprintf(vfp, "assign %s%d_in%d = ",\
                                   opSymTable(op_type), fu_nu + 1 + offset, port_nu);

                   if (op_type == mx || op_type == mxd) {
                     if (port_nu == 0)  // actually mux select
                       fprintf(vfp, "assign %s%d_sel_rin = ", \
                               opSymTable(op_type), fu_nu + 1 + offset);
                     else
                       fprintf(vfp, "assign %s%d_in%d_rin = ",\
                               opSymTable(op_type), fu_nu + 1 + offset, port_nu - 1);
                   }
                   else
                     fprintf(vfp, "assign %s%d_in%d_rin = ",\
                                   opSymTable(op_type), fu_nu + 1 + offset, port_nu);

                   last = RF_PortNu = GetRegFilePortNu(i, dest_nu[--dest_cnt]);
                   //endTime = DFG[dest_nu[dest_cnt]]->opScheduledSlot;
                   endTime = GetEndTime(dest_nu[dest_cnt]);

                   assert (Map[i] != 0);

                   fprintf(vfp, "state[%d] ? RF%d_rdata%d : ", endTime, Map[i], RF_PortNu); 

                   //------------------------------------------------------
                   // Reg -> MuxPQ Fanout

                   GetRegFanOut(Map, i, RF_PortNu);
                   //------------------------------------------------------

                   totalMuxInputNu++;
                   impMuxInputNu++;

                   while(dest_cnt-- > 0) {
                     RF_PortNu = GetRegFilePortNu(i, dest_nu[dest_cnt]);
                     //endTime = DFG[dest_nu[dest_cnt]]->opScheduledSlot;
                     endTime = GetEndTime(dest_nu[dest_cnt]);

                     assert (Map[i] != 0);

                     if (last != RF_PortNu) {
                       if (dest_cnt == 0)
                         fprintf(vfp, "RF%d_rdata%d;\n", Map[i], RF_PortNu); 
                       else
                         fprintf(vfp, "state[%d] ? RF%d_rdata%d : ", endTime, Map[i], RF_PortNu); 

                       //------------------------------------------------------
                       // Reg -> MuxPQ Fanout

                       GetRegFanOut(Map, i, RF_PortNu);
                       //------------------------------------------------------

                       totalMuxInputNu++;
                       impMuxInputNu++;

                       //---------------------------------------------------------------------
                       // Consider RF{x}_rdata{} may be the output of MUXs. If that is the case,
                       // then the implicit MUX's fan-in should include them as well 
                       //---------------------------------------------------------------------
                       RF_portPtr RegFileTable = DFG[i]->RegFileTable;
                       if (RegFileTable != NULL && RegFileTable[RF_PortNu-1].sharedCnt > 1)
                         impMuxInputNu = RegFileTable[RF_PortNu-1].sharedCnt;
                     }
                     last = RF_PortNu;
                   }

                   //------------------------------------------------------
                   // Register insertion 
                   //------------------------------------------------------
                   //fprintf(vfp, "assign %s%d_in%d = %s%d_in%d_rout;\n",   \
                           opSymTable(op_type), fu_nu + 1 + offset, port_nu, \
                           opSymTable(op_type), fu_nu + 1 + offset, port_nu);

                   if (impMuxInputNu > maxMuxInputNu) maxMuxInputNu = impMuxInputNu;

                 } // else (flag = 1)

               } else {

                 assert (Map[i] != 0);

                 //---------------------------------------------------------------------
                 // IV. dest_nu = 1 
                 // Note that there is only one destination connected to the FU's port input
                 //---------------------------------------------------------------------
                 assert (dest_cnt = 1);

                 //------------------------------------------------------
                 // Register insertion 
                 //------------------------------------------------------
                   //fprintf(vfp, "assign %s%d_in%d = RF%d_rdata%d;\n", \
                                 opSymTable(op_type), fu_nu + 1 + offset, port_nu, \
                                 Map[i], GetRegFilePortNu(i, dest_nu[0]));
                   if (op_type == mx || op_type == mxd) {
                     if (port_nu == 0)  // actually mux select
                       fprintf(vfp, "assign %s%d_sel_rin = RF%d_rdata%d;\n",   \
                               opSymTable(op_type), fu_nu + 1 + offset, \
                               Map[i], GetRegFilePortNu(i, dest_nu[0]));
                     else
                       fprintf(vfp, "assign %s%d_in%d_rin = RF%d_rdata%d;\n",   \
                               opSymTable(op_type), fu_nu + 1 + offset, port_nu - 1, \
                               Map[i], GetRegFilePortNu(i, dest_nu[0]));
                   }
                   else
                     fprintf(vfp, "assign %s%d_in%d_rin = RF%d_rdata%d;\n", \
                             opSymTable(op_type), fu_nu + 1 + offset, port_nu, \
                             Map[i], GetRegFilePortNu(i, dest_nu[0]));

                 //fprintf(vfp, "assign %s%d_in%d = %s%d_in%d_rout;\n",   \
                         opSymTable(op_type), fu_nu + 1 + offset, port_nu, \
                         opSymTable(op_type), fu_nu + 1 + offset, port_nu);

                 // Fanout
                 GetRegFanOut(Map, i, GetRegFilePortNu(i, dest_nu[0]));
               }
               free(dest_nu);
             }
           } // for 
         }
       }
     }
   }

  //------------------------------------------------------------
  // Stats
  //------------------------------------------------------------

  myfprintf(fp, "Total MUXPQ inputs = %d (max %d)\n\n", 
      totalMuxInputNu, maxMuxInputNu);

  //CSP->mux += totalMuxInputNu;
  //if (maxMuxInputNu > CSP->fan) CSP->fan = maxMuxInputNu; 
}


// Use destination node ID to get RF read port from RegFile Table
int GetRegFilePortNu(int nodeID, int childID) {

  int RF_portNu = 0;
  int j, k;
  RF_portNuPtr head;
  RF_portPtr RegFileTable;

  RegFileTable = DFG[nodeID]->RegFileTable;

  assert(RegFileTable != NULL);

  for (j = 0; j < DFG[nodeID]->opDestNu; j++) {
    head = RegFileTable[j].RegFileList;
    if (head != NULL) {
      RF_portNu++;
      while (head != NULL) {
        for (k = 0; k < head->sharedDestNu; k++) { 
          if (childID == *(head->sharedDestNuPtr + k)) {
            return RF_portNu; 
          }
        }
        //else
        head = head->next;
      } // while
    } // if
  }

  // We should not come here
  myprintf("error: nodeID = %d childID = %d j = %d RF_portNu = %d\n", 
         nodeID, childID, j, RF_portNu);
  exit(-1);
}

/* Use child ID to get RF read port address enable
int GenerateRegFilePortMux(FILE *vfp, int *Map, int nodeID, int childID) {

  int RF_portNu = 0;
  int RF_enNu;
  int j, k;
  char idx;
  RF_NuCnt head;
  RF_portNuPtr temp;
  RF_portPtr RegFileTable;

  RegFileTable = DFG[nodeID]->RegFileTable;

  assert(RegFileTable != NULL);

  for (j = 0; j < DFG[nodeID]->opDestNu; j++) {
    head = RegFileTable[j];
    temp = head.RegFileList;
    if (temp != NULL) {
      idx = 'a';
      RF_portNu++;
      RF_enNu = 0;
      while (temp != NULL) {
        RF_enNu++;
        for (k = 0; k < temp->sharedDestNu; k++) { 
          if (childID == *(temp->sharedDestNuPtr + k)) {
            if (head.sharedCnt > 1)
              fprintf(vfp, "r%d_raddr_%d%c_en ? RF%d_rdata%d : ",\
               Map[nodeID], RF_portNu, idx + RF_enNu - 1,\
               Map[nodeID], RF_portNu); 
            else 
              fprintf(vfp, "r%d_raddr_%d_en ? RF%d_rdata%d : ",\
               Map[nodeID], RF_portNu, \
               Map[nodeID], RF_portNu); 
          }
        }
        //else
        temp = temp->next;
      } // while
    } // if
  }
}
*/


//-----------------------------------------------------------------------------
// Wire FU output to each shared register's input 
// if the number of its sources is 1
//-----------------------------------------------------------------------------
void GeneratePhase2Wire(FILE *vfp, int SharedRegNu) {

  int muxInputNu;
  int regNu;
  ResourceList fu, head;

  for (regNu = 1; regNu <= SharedRegNu; regNu++) {
    muxInputNu = 0;

    fu = head = FuRAT[regNu];

    while (fu != NULL) {
      fu = fu->next;
      muxInputNu++;
    }

    // Each shared register has at least one source
    if (muxInputNu < 2) {
      if (head->op == nop) {
        fprintf(vfp, "assign r%d_in = p%d_in_r;\n",\
              regNu, GetInputPortNu(head->nu)); // registered input 

        //---------------------------------------------------------
        // Registered port fanout (port -> FU input)
        //---------------------------------------------------------
        PortFanout[GetInputPortNu(head->nu) - 1]++;
      }
      else if (head->op == nopc) {
        fprintf(vfp, "assign r%d_in = %s_in_r;\n", \
            GetCPortName(head->nu));

        //---------------------------------------------------------
        // Registered port fanout (port -> FU input)
        //
        // Not yet implemented
        //---------------------------------------------------------
      }
      else {
        fprintf(vfp, "assign r%d_in = %s%d_out;\n", 
              regNu, opSymTable(head->op), head->nu);
        //---------------------------------------------------------
        // FU output -> Register input Fanout
        //---------------------------------------------------------
        FuFanout[head->op][head->nu - 1]++;
      }
    }
  }

  fprintf(vfp, "\n");
}

//-------------------------------------------------------
// Logical portNu starts from 0 to MAX_PORT_NU - 1
// search SortedPortPriority for index of node port number 
//-------------------------------------------------------
int GetInputPortNu(int portNu) {

  int i;

  // 5/31/11 PortPriority is NULL for fully parallel case 
  if (SortedPortPriority != NULL) {
    for (i = 0; i < MAX_PORT_NU; i++) {
      if (portNu == SortedPortPriority[i]) 
        break;
    }
  } else {
    i = portNu;
  }

  assert(i < MAX_PORT_NU);
  assert(PortAssignment[i] >= 1);
  assert(PortAssignment[i] <= PORT_NU);

  return PortAssignment[i];
}

void GenerateRegister(FILE *vfp, int dataWidth, int regNu) {

  fprintf(vfp, "wire [%d : 0] r%d_in;\n", dataWidth - 1, regNu);
  fprintf(vfp, "wire [%d : 0] r%d_out;\n", dataWidth - 1, regNu);
  fprintf(vfp, "wire          r%d_wen;\n", regNu);
  fprintf(vfp, "Register #(%d) r%d(\n", dataWidth, regNu);
  fprintf(vfp, "  .clk   (clk     ),\n");
  fprintf(vfp, "  .rst   (rst     ),\n");
  //fprintf(vfp, "  .stall (stall   ),\n");
  fprintf(vfp, "  .wen   (r%d_wen ),\n", regNu);
  fprintf(vfp, "  .wdata (r%d_in  ),\n",regNu);
  fprintf(vfp, "  .rdata (r%d_out )\n);\n\n", regNu);

  // 8/29/12
  fprintf(vfp, "reg [%d : 0] r%d_out_r;\n", dataWidth - 1, regNu);
  //fprintf(vfp, "wire [%d : 0] r%d_out_tmp;\n", dataWidth - 1, regNu);
  fprintf(vfp, "always @ (posedge clk)\n");
  fprintf(vfp, "  if (!stall) r%d_out_r <= r%d_out;\n\n", regNu, regNu);

}

void GenerateFuncRegister(FILE *vfp) {
  
  char *opSym;
  enum operation op_type;
  int p;
  int size;
  int i;
  int n;
  int tmpFuNuCnt = 0;
  int dataWidth; // by default
  int portNu;

  static int FuNuCnt;

  for (op_type = add; op_type < add + OP_NU; op_type++) {

    FuNuCnt = 0;

    opSym = opSymTable(op_type);
    size = (*RQ[op_type])->size;

    for (i = 0; i < size; i++) {

      FuNuCnt++;

      //---------------------------------------------------------------------
      // e.g. fmul1_in0_rin, fmul1_in0_rout, fmul1_in0_wen
      //---------------------------------------------------------------------
      portNu = GetNodeSrcNu(op_type);

      for (p = 0; p < portNu; p++) {

        if (GetOpSym(op_type) == 'm' && p == 2) {

          dataWidth =1; // mux sel

          fprintf(vfp, "wire [%d : 0] %s%d_sel_rin;\n", dataWidth - 1, opSym, FuNuCnt);
          fprintf(vfp, "wire [%d : 0] %s%d_sel_rout;\n", dataWidth - 1, opSym, FuNuCnt);
          fprintf(vfp, "wire          %s%d_sel_wen = 1'b1;\n", opSym, FuNuCnt);
          fprintf(vfp, "Register #(%d) %s%d_sel_r(\n", 1, opSym, FuNuCnt);
          fprintf(vfp, "  .clk   (clk     ),\n");
          fprintf(vfp, "  .rst   (rst     ),\n");
          //fprintf(vfp, "  .stall (stall   ),\n");
          fprintf(vfp, "  .wen   (%s%d_sel_wen & ~stall),\n", opSym, FuNuCnt);
          fprintf(vfp, "  .wdata (%s%d_sel_rin ),\n", opSym, FuNuCnt);
          fprintf(vfp, "  .rdata (%s%d_sel_rout)\n);\n\n", opSym, FuNuCnt);

          //---------------------------------------------------------------------
          // assign register insertion output to FU input
          //---------------------------------------------------------------------
          fprintf(vfp, "assign %s%d_sel = %s%d_sel_rout;\n", 
              opSym, FuNuCnt, opSym, FuNuCnt);
        }

        else {

          dataWidth = DataPathWidth(op_type, 0);

          fprintf(vfp, "wire [%d : 0] %s%d_in%d_rin;\n", dataWidth - 1, opSym, FuNuCnt, p);
          fprintf(vfp, "wire [%d : 0] %s%d_in%d_rout;\n", dataWidth - 1, opSym, FuNuCnt, p);

          //---------------------------------------------------------------------
          // always enabled
          //---------------------------------------------------------------------
          fprintf(vfp, "wire          %s%d_in%d_wen = 1'b1;\n", opSym, FuNuCnt, p);
          fprintf(vfp, "Register #(%d) %s%d_in%d_r(\n", dataWidth, opSym, FuNuCnt, p);
          fprintf(vfp, "  .clk   (clk     ),\n");
          fprintf(vfp, "  .rst   (rst     ),\n");
          //fprintf(vfp, "  .stall (stall   ),\n");
          fprintf(vfp, "  .wen   (%s%d_in%d_wen & ~stall),\n", opSym, FuNuCnt, p);
          fprintf(vfp, "  .wdata (%s%d_in%d_rin ),\n", opSym, FuNuCnt, p);
          fprintf(vfp, "  .rdata (%s%d_in%d_rout)\n);\n\n", opSym, FuNuCnt, p);

          //---------------------------------------------------------------------
          // assign register insertion output to FU input
          //---------------------------------------------------------------------
          fprintf(vfp, "assign %s%d_in%d = %s%d_in%d_rout;\n", 
              opSym, FuNuCnt, p, opSym, FuNuCnt, p);


          //*******************************************************************//
          // Assume FU mux, tod are unsharable.
          //
          //*******************************************************************//

          for (n = MAX_PORT_NU; n < NODE_NU; n++) {
          //---------------------------------------------------------------------
          // Assume ROM has no input connected to a constant
          //---------------------------------------------------------------------

          //---------------------------------------------------------------------
          // Assign Single2Double converstion constant input
          // Assume only one MUX with constant input can be found in the for loop
          //---------------------------------------------------------------------
            if (DFG[n]->op == tod && DFG[n]->opConst) {
              fprintf(vfp, "assign %s%d_in%d_rin = 32'h%08x; // %f\n",   \
                   opSym, FuNuCnt, 0, FloatingPointConvert(DFG[n]->opConstVal), DFG[n]->opConstVal);
            }

            if (DFG[n]->op == tof && DFG[n]->opConst) {
              fprintf(vfp, "assign %s%d_in%d_rin = 64'h%016x; // %lf\n",   \
                   opSym, FuNuCnt, 0, DoubleFloatingPointConvert(DFG[n]->opConstVal_d), DFG[n]->opConstVal_d);
            }

          //---------------------------------------------------------------------
          // Assign MUX constant inputs
          // Assume only one MUX with constant input can be found in the for loop
          //---------------------------------------------------------------------

            if ((DFG[n]->op == mx && op_type == mx || DFG[n]->op == mxd && op_type == mxd) && 
                DFG[n]->opResourceNu == FuNuCnt && DFG[n]->opSrc[p+1] == -1) {

              if (DFG[n]->opPrecision == sfp)
                 fprintf(vfp, "assign %s%d_in%d_rin = 32'h%08x; // %f\n",   \
                       opSym, FuNuCnt, p, FloatingPointConvert(DFG[n]->opMuxVal[p]), DFG[n]->opMuxVal[p]);

              else if (DFG[n]->opPrecision == dfp) 
                 fprintf(vfp, "assign %s%d_in%d_rin = 64'h%016llx; // %lf\n",   \
                       opSym, FuNuCnt, p, DoubleFloatingPointConvert(DFG[n]->opMuxVal_d[p]), DFG[n]->opMuxVal_d[p]);
            }
          }
        }
        fprintf(vfp, "\n");
      }
    }
  }
}

void GenerateRegisterC(FILE *vfp, int dataWidth, int regNu) {

  //fprintf(vfp, "wire [%d : 0] r%d_in;\n", dataWidth - 1, regNu);
  //fprintf(vfp, "wire          r%d_wen;\n", regNu);
  fprintf(vfp, "wire [%d : 0] r%d_rdy;\n", dataWidth - 1, regNu);
  fprintf(vfp, "RegisterC #(%d) r%dc(\n", dataWidth, regNu);
  fprintf(vfp, "  .clk   (clk     ),\n");
  fprintf(vfp, "  .rst   (rst     ),\n");
  fprintf(vfp, "  .stall (stall   ),\n");
  fprintf(vfp, "  .wdata (r%d_wen ),\n",regNu);
  fprintf(vfp, "  .rdata (r%d_rdy )\n);\n\n", regNu);
}

void GenerateRegisterV(FILE *vfp, int dataWidth, int regNu) {

  fprintf(vfp, "wire [%d : 0] r%d_in;\n", dataWidth - 1, regNu);
  fprintf(vfp, "wire [%d : 0] r%d_out;\n", dataWidth - 1, regNu);
  fprintf(vfp, "wire          r%d_wen;\n", regNu);
  fprintf(vfp, "wire          r%d_rdy;\n", regNu);
  fprintf(vfp, "RegisterV #(%d) r%d(\n", dataWidth, regNu);
  fprintf(vfp, "  .clk   (clk     ),\n");
  fprintf(vfp, "  .rst   (rst     ),\n");
  //fprintf(vfp, "  .stall (stall   ),\n");
  fprintf(vfp, "  .clr   (r%d_clr ),\n", regNu);
  fprintf(vfp, "  .wen   (r%d_wen ),\n", regNu);
  fprintf(vfp, "  .wdata (r%d_in  ),\n",regNu);
  fprintf(vfp, "  .rdata (r%d_out ),\n", regNu);
  fprintf(vfp, "  .rdy   (r%d_rdy )\n);\n\n", regNu);
}

void GenerateShiftRegister(FILE *vfp, FILE *fp, int addrWidth, int dataWidth, 
                          int dataDepth, int cntVal, int regNu, 
                          RF_portPtr RegFileTable, int DestNu) {
  char idx;
  int i, j;
  int RF_PortNu;
  RF_NuCnt head;

  //---------------------------------------------------------------------------
  // part 1: instantiate Register File
  //---------------------------------------------------------------------------
  RF_PortNu = TotalRegFilePortNu(RegFileTable, DestNu);

  if (RF_PortNu > 1)
    for (i = 1; i <= RF_PortNu; i++)
      fprintf(vfp, "wire [%d : 0] RF%d_rdata%d;\n", dataWidth - 1, regNu, i);
  else
    fprintf(vfp, "wire [%d : 0] r%d_out;\n", dataWidth - 1, regNu);

#if MUX_OP != 0
  // 8/27/12
  // add a register on the path without mux chain register.
  // If this register is not used, then the synthesis tool will trim it
  
  // net declarations
  if (RF_PortNu > 1) {
    for (i = 1; i <= RF_PortNu; i++) {
      fprintf(vfp, "reg  [%d : 0] RF%d_rdata%d_r;\n", dataWidth - 1, regNu, i);
      fprintf(vfp, "always @ (posedge clk)\n");
      fprintf(vfp, "  if (!stall) RF%d_rdata%d_r <= RF%d_rdata%d;\n\n", regNu, i, regNu, i);
    }
  } else {
   fprintf(vfp, "reg  [%d : 0] r%d_out_r;\n", dataWidth - 1, regNu);
   fprintf(vfp, "always @ (posedge clk)\n");
   fprintf(vfp, "  if (!stall) r%d_out_r <= r%d_out;\n\n", regNu, regNu);
  }
#endif

  // 9/6/12 additional register write enable after all input data are read
  fprintf(vfp, "wire          r%d_fen;\n",  regNu);

  fprintf(vfp, "wire          r%d_wen;\n",  regNu);
  fprintf(vfp, "wire [%d : 0] r%d_in;\n",   dataWidth - 1, regNu);
  fprintf(vfp, "wire [%d : 0] sr%d_out;\n", dataWidth - 1, regNu);
  fprintf(vfp, "wire [%d : 0] r%d_wout;\n", dataWidth * dataDepth - 1, regNu);
  fprintf(vfp, "\n");

  fprintf(vfp, "ShiftRegister #(%d, %d) r%d (\n", dataWidth, dataDepth, regNu);
  fprintf(vfp, "  .clk  (clk       ),\n");
  fprintf(vfp, "  .en   (r%d_wen   ),\n", regNu);
  fprintf(vfp, "  .din  (r%d_in    ),\n", regNu);
  fprintf(vfp, "  .dout (sr%d_out  ),\n", regNu);
  fprintf(vfp, "  .wdout(r%d_wout  ));\n",regNu);

  //---------------------------------------------------------------------------
  // part 2: instantiate Register File read data select signals
  //---------------------------------------------------------------------------
  for (i = 1; i <= RF_PortNu; i++) {
    head = RegFileTable[i - 1];
    assert (head.RegFileList != NULL);

    for (j = 0; j < head.sharedCnt - 1; j++) {
      idx = 'a' + j;
      fprintf(vfp, "wire r%d_%d%c_sel;\n",  regNu, i, idx);
    }
  } // for 

  //---------------------------------------------------------------------------
  // part 3: wire Register File with counters
  //---------------------------------------------------------------------------

  // If shift register has only one data port, then its name is in the form
  // of "r{x}_out".
  if (RF_PortNu == 1) {

    // Simply assign the shift register output to r{x}_out
    if (RegFileTable[0].sharedCnt == 1)
      fprintf(vfp, "assign r%d_out = sr%d_out;\n\n", regNu, regNu);

    else { // (RegFileTable[0].sharedCnt > 1)
      // Assign the Mux output to r{x}_out; actually not a register output
      fprintf(vfp, "assign r%d_out = ", regNu);
      GenerateRegMux(vfp, regNu, RF_PortNu, RegFileTable, dataWidth);
      GenerateRegMuxSel(vfp, regNu, RF_PortNu, RegFileTable);
    }
  } else {

    // When there are more than one RF ports,
    // r{x}_out is not declared. 
    // Use  RF{x}_rdata{x} at the moment 
    for (i = 1; i <= RF_PortNu; i++) {
      fprintf(vfp, "assign RF%d_rdata%d = ", regNu, i);
      GenerateRegMux(vfp, regNu, i, RegFileTable, dataWidth);
      GenerateRegMuxSel(vfp, regNu, i, RegFileTable);
    } // for
  }
  fprintf(vfp, "\n");
}

void GenerateRegMuxSel(FILE *vfp, int regNu, int RF_PortNu, 
                       RF_portPtr RegFileTable) {
  int startTime, endTime, lifeTime;
  int child_ID;
  int dsel;
  int j;
  char idx;
  RF_NuCnt head;
  RF_portNuPtr temp;
  //startTime = VarTable[regNu-1]->startTime;
  head = RegFileTable[RF_PortNu-1];
  temp = head.RegFileList; 

  for (j = 0; j < head.sharedCnt; j++) {
    assert(temp != NULL);
    idx = 'a' + j;
    child_ID = *(temp->sharedDestNuPtr);
    //endTime = DFG[child_ID]->opScheduledSlot;
    endTime = GetEndTime(child_ID);

    // no select when head.sharedCnt = 1
    if (j != head.sharedCnt - 1) 
      fprintf(vfp, "assign r%d_%d%c_sel = state[%2d];\n",\
                  regNu, RF_PortNu, idx, endTime);
    temp = temp->next;
  }
}

void GenerateRegMux(FILE *vfp, int regNu, int RF_PortNu, 
                    RF_portPtr RegFileTable, int dataWidth) {
  int startTime, endTime, lifeTime;
  int child_ID;
  int dsel;
  char idx;
  int j;
  RF_NuCnt head;
  RF_portNuPtr temp;

  // remember its lifetime is > minII and there is only one variable.
  startTime = VarTable[regNu-1]->startTime;
  head = RegFileTable[RF_PortNu-1];
  temp = head.RegFileList; 

  for (j = 0; j < head.sharedCnt; j++) {
    assert(temp != NULL);
    idx = 'a' + j;
    child_ID = *(temp->sharedDestNuPtr);
    //endTime = DFG[child_ID]->opScheduledSlot;
    endTime = GetEndTime(child_ID);
    
    myprintf("GenerateRegMux regNu=%d childID %d endTime %d startTime %d\n",
        regNu, child_ID, endTime, startTime);

    lifeTime = endTime - startTime;

    dsel = lifeTime / minII + ((lifeTime % minII) ? 1 : 0); // myceil(lifeTime, minII);
    if (j == head.sharedCnt - 1) {
      //e.g. fprintf(vfp, "r%d_wout[%d*32-1 : %d*32];\n", regNu, dsel, dsel-1);
      fprintf(vfp, "r%d_wout[%d*%d-1 : %d*%d];\n", regNu, \
          dsel, dataWidth, dsel-1, dataWidth);
    }
    else {
      //e.g. fprintf(vfp, "r%d_%d%c_sel ? r%d_wout[%d*%d-1 : %d*%d] : ",\
                  regNu, RF_PortNu, idx, regNu, dsel, dsel - 1);
      fprintf(vfp, "r%d_%d%c_sel ? r%d_wout[%d*%d-1 : %d*%d] : ",\
                  regNu, RF_PortNu, idx, regNu, \
                  dsel, dataWidth, dsel - 1, dataWidth);
    }

    //-------------------------------------------------------
    // Fanout register->RMux input
    if (head.sharedCnt > 1) {
      int *entry = (RegFanout[regNu - 1]).entry;
      entry[dsel-1]++;
    }
    //-------------------------------------------------------

    temp = temp->next;
  }
}

// Return the total number of register file ports 
int TotalRegFilePortNu(RF_portPtr RegFileTable, int DestNu) {
  RF_NuCnt head;
  int i;
  int RF_PortNu = 0;

  for (i = 0; i < DestNu; i++) {
    head = RegFileTable[i];
    if (head.RegFileList != NULL) {
      assert(head.sharedCnt != 0);
      RF_PortNu++;
    }
  }
  assert(RF_PortNu != 0);
  /*
  if (RF_PortNu == 0) {
    myprintf("Error: RF_PortNu = 0\n"); 
    PrintRegFileTable(RegFileTable, DestNu);
  } */
  return RF_PortNu;
}

void GenerateRegisterFile(FILE *vfp, int addrWidth, int dataWidth,\
                          int dataDepth, int cntVal, int regNu,\
                          RF_portPtr RegFileTable, int DestNu) {
  char idx;
  int i, j;
  int RF_PortNu;
  RF_NuCnt head;

  fprintf(vfp, "wire [%d : 0] RF%d_waddr;\n", addrWidth - 1, regNu);
  fprintf(vfp, "wire [%d : 0] RF%d_wdata;\n", dataWidth - 1, regNu);

  RF_PortNu = TotalRegFilePortNu(RegFileTable, DestNu);

  //---------------------------------------------------------------------------
  // part 1: instantiate Register File
  //---------------------------------------------------------------------------
  for (i = 1; i <= RF_PortNu; i++) {
    fprintf(vfp, "wire [%d : 0] RF%d_raddr%d;\n", addrWidth - 1, regNu, i);
    fprintf(vfp, "wire [%d : 0] RF%d_rdata%d;\n", dataWidth - 1, regNu, i);
  }

  fprintf(vfp, "wire          RF%d_wen;\n", regNu);
  fprintf(vfp, "wire          RF%d_ren;\n", regNu);
  fprintf(vfp, "\n");

  fprintf(vfp, "RegisterFile_%dP #(%d, %d, %d) RF%d (\n", 
                RF_PortNu, dataWidth, addrWidth, dataDepth, regNu);
  fprintf(vfp, "  .clk  (clk        ),\n");
  fprintf(vfp, "  .rst  (rst        ),\n");
  fprintf(vfp, "  .stall(stall      ),\n");
  fprintf(vfp, "  .wen  (RF%d_wen   ),\n", regNu);
  fprintf(vfp, "  .waddr(RF%d_waddr ),\n", regNu);
  fprintf(vfp, "  .wdata(RF%d_wdata ), \n",regNu);

  for (i = 1; i <= RF_PortNu; i++) {
    fprintf(vfp, "  .raddr%d(RF%d_raddr%d),\n", i, regNu, i);
    fprintf(vfp, "  .rdata%d(RF%d_rdata%d),\n", i, regNu, i);
  }

  //fprintf(vfp, "  .ren(RF%d_ren   ),\n", regNu);
  fprintf(vfp, "  .ren  (1'b0       ));\n");


  //---------------------------------------------------------------------------
  // part 2: instantiate Register File read address counters
  //---------------------------------------------------------------------------

  // net declarations
  fprintf(vfp, "wire [%d : 0] r%d_waddr;\n", addrWidth - 1, regNu);
  fprintf(vfp, "wire          r%d_wen;\n"   , regNu);

  // Instantiate address counter to write RF 
  fprintf(vfp, "counterA #(%d, %d, %d, %d) cnt_r%d_waddr\n", 
                addrWidth, cntVal, IntLog2a(minII), minII, regNu, regNu);    
  fprintf(vfp, "(clk, rst, stall, r%d_wen, r%d_waddr);\n\n", regNu, regNu); 

  for (i = 1; i <= RF_PortNu; i++) {
    head = RegFileTable[i - 1];
    assert (head.RegFileList != NULL);

    if (head.sharedCnt == 1) {

      // Instantiate just one address counter to read RF 
      
      // net declarations
#ifdef DATA_PATH_ONLY
      fprintf(vfp, " wire          R%d_enable_%d;\n", regNu, i);
#else
      fprintf(vfp, " reg          R%d_enable_%d;\n", regNu, i);
#endif
      fprintf(vfp, "wire [%d : 0] r%d_raddr%d;\n", addrWidth - 1, regNu, i);

      // + 1/19/11
      fprintf(vfp, "wire          r%d_raddr_%d_en;\n", regNu, i);

      // - 1/19/11
      //fprintf(vfp, "counterA #(%d, %d, %d, %d) cnt_r%d_raddr%d\n",
      //               addrWidth, cntVal, IntLog2a(minII), minII , regNu, i);    
      //fprintf(vfp, "(clk, rst, stall, R%d_enable_%d, r%d_raddr%d);\n\n", regNu, i, regNu, i); 

      // + 1/19/11
      fprintf(vfp, "counterAC #(%d, %d, %d, %d) cnt_r%d_raddr%d\n",
                     addrWidth, cntVal, IntLog2a(minII), minII , regNu, i);    
      fprintf(vfp, "(clk, rst, stall, R%d_enable_%d, r%d_raddr%d, r%d_raddr_%d_en);\n\n",
                     regNu, i, regNu, i, regNu, i); 

    } else {

      assert (head.sharedCnt > 1);

      // Instantiate multiple address counters to read RF 
      for (j = 0; j < head.sharedCnt; j++) {
        idx = 'a' + j;

        // net declarations
#ifdef DATA_PATH_ONLY
        fprintf(vfp, " wire         R%d_enable_%d%c;\n", regNu, i, idx);
#else
        fprintf(vfp, " reg          R%d_enable_%d%c;\n", regNu, i, idx);
#endif

        fprintf(vfp, "wire [%d : 0] r%d_raddr_%d%c;\n",  addrWidth - 1, regNu, i, idx);
        fprintf(vfp, "wire          r%d_raddr_%d%c_en;\n", regNu, i, idx);

        fprintf(vfp, "counterAC #(%d, %d, %d, %d) cnt_r%d_raddr%d%c\n",
                             addrWidth, cntVal, IntLog2a(minII), minII , regNu, i, idx);    

        fprintf(vfp, "(clk, rst, stall, R%d_enable_%d%c, r%d_raddr_%d%c, r%d_raddr_%d%c_en);\n\n", 
                                                     regNu, i, idx, 
                                                     regNu, i, idx,
                                                     regNu, i, idx); 
      } //for
    } // else
  } // for 

  //---------------------------------------------------------------------------
  // part 3: wire Register File with counters
  //---------------------------------------------------------------------------
  fprintf(vfp, "wire [%d : 0] r%d_in;\n", dataWidth - 1, regNu);
  if (RF_PortNu == 1) fprintf(vfp, "wire [%d : 0] r%d_out;\n", dataWidth - 1, regNu);
  fprintf(vfp, "assign RF%d_wen   = r%d_wen;\n", regNu, regNu);
  fprintf(vfp, "assign RF%d_waddr = r%d_waddr;\n", regNu, regNu);
  fprintf(vfp, "assign RF%d_wdata = r%d_in;\n", regNu, regNu);

  // 12/26  (Just one regfile for regNu)
  if (RF_PortNu == 1) // && RegFileTable[0].sharedCnt == 1)
    fprintf(vfp, "assign r%d_out = RF%d_rdata1;\n\n", regNu, regNu);

  for (i = 1; i <= RF_PortNu; i++) {

    head = RegFileTable[i - 1];

    assert (head.RegFileList != NULL);

    fprintf(vfp, "assign RF%d_raddr%d = ",  regNu, i);

    if (head.sharedCnt == 1) {
      fprintf(vfp, "r%d_raddr%d;\n", regNu, i);

    } else {

      for (j = 0; j < head.sharedCnt; j++) {
        idx = 'a' + j;
        // A read address MUX chain
        fprintf(vfp, "r%d_raddr_%d%c_en ? r%d_raddr_%d%c : ", 
            regNu, i, idx, regNu, i, idx);
      }
      fprintf(vfp, "%d\'bx;\n", addrWidth);
    } // if
  } // for

  fprintf(vfp, "\n");
}

void GenerateRomInstance(FILE *vfp , int muxNu, char instanceNuIdx, int inputNu, int flag) {

  // e.g. 
  // wire [ ] m1[a]_sel; 
  // rom_m1[a] rom_m1[a]_i (m1[a]_count, m1[a]_sel);
  
  int i;
  const int SignalNu = 3;

  // Wire declaration
  fprintf(vfp, "wire [%d : 0] ", IntLog2(inputNu) - 1);
  fprintf(vfp, "m%d", muxNu);
  if (flag) fprintf(vfp, "%c", instanceNuIdx);
  fprintf(vfp, "_sel;\n");

  // Module name and instance name
  for (i = 0; i < 2; i++) {
    fprintf(vfp, "rom_m%d", muxNu);
    if (flag) fprintf(vfp, "%c", instanceNuIdx);

    if (i) fprintf(vfp, "_i");

    fprintf(vfp, " ");
  }

  // Instance signals
  fprintf(vfp, "(");

  for (i = 0; i < SignalNu; i++) {
    fprintf(vfp, "m%d", muxNu);
    if (flag) fprintf(vfp, "%c", instanceNuIdx);
    fprintf(vfp, "_");
    if (i == SignalNu - 1)
      fprintf(vfp, "sel);\n\n");
    else {
      fprintf(vfp, "%s", ROMSignalSymTable(i));
      fprintf(vfp, ", ");
    }
  }
}
  
void GenerateRom
(FILE *vfp, char *instanceName, int muxNu, char instanceNuIdx, 
 int inputNu, int flag, MuxSelList head, MuxSelList end) {

  MuxSelList p;

  // Module name
  fprintf(vfp, "module rom_");
  fprintf(vfp, "%c%d", instanceName[0], muxNu);
  if (flag) fprintf(vfp, "%c", instanceNuIdx);
  // Signals
  fprintf(vfp, " (addr, enable, out);\n");
  // Paramters
  fprintf(vfp, "   parameter CntWidth = %d;\n", IntLog2(minII));
  fprintf(vfp, "   parameter MemWidth = %d;\n", IntLog2(inputNu));
  // Signals
  fprintf(vfp, "   input  [CntWidth-1 : 0] addr;\n");
  fprintf(vfp, "   input                   enable;\n");
  fprintf(vfp, "   output [MemWidth-1 : 0] out;\n");
  fprintf(vfp, "      reg [MemWidth-1 : 0] out_int;\n");
  fprintf(vfp, "   always @ (addr) begin\n");
  fprintf(vfp, "     case (addr)\n");

  // Control pattern in ROM
  p = head;
  while (p != end) {
    fprintf(vfp, "         %d: out_int = %d;\n", p->startTime % minII, p->selVal);
    p = p->next;
  }
  
  fprintf(vfp, "       default: out_int = 0;\n");
  fprintf(vfp, "     endcase\n");
  fprintf(vfp, "   end\n");
  fprintf(vfp, "   assign out = enable ? out_int : {MemWidth{1'b0}};\n");
  fprintf(vfp, "endmodule\n");
  fprintf(vfp, "\n");
}

void GenerateCounter
(FILE *vfp, int sum, char *instanceName, int regNu, char instanceNuIdx, 
 int flag) {

  int i;
  int SignalNu = 5;

  if (!sum) SignalNu++; // count_enable

  // Wire declarations excluding clk and rst
  for (i = 2; i < SignalNu; i++) {
    if (i == 3) // reg because connected directly to FSM outputs
      fprintf(vfp, "reg ");
    else if (i == SignalNu - 1 && !sum)
      fprintf(vfp, "wire [%d : 0] ", IntLog2(minII) - 1);
    else 
      fprintf(vfp, "wire ");

    fprintf(vfp, "%c%d", instanceName[0], regNu);
    if (flag) fprintf(vfp, "%c", instanceNuIdx);
    fprintf(vfp, "_");
    fprintf(vfp, "%s;\n", CounterSignalSymTable(instanceName, i));
  }

  // counter must not be generated when minII is 1
  if (minII == 1) {

   // reg enable_r
    fprintf(vfp, "reg ");
    fprintf(vfp, "%c%d", instanceName[0], regNu);
    if (flag) fprintf(vfp, "%c", instanceNuIdx);
    fprintf(vfp, "_");
    fprintf(vfp, "%s", CounterSignalSymTable(instanceName, 3));
    fprintf(vfp, "_r;\n");

    fprintf(vfp, "assign ");
    i = SignalNu - 1;
    fprintf(vfp, "%c%d", instanceName[0], regNu);
    if (flag) fprintf(vfp, "%c", instanceNuIdx);
    fprintf(vfp, "_");
    fprintf(vfp, "%s", CounterSignalSymTable(instanceName, i));
    fprintf(vfp, " = ");

    // ~_stall & (enable | enable_r)
    fprintf(vfp, "~%c%d", instanceName[0], regNu);
    if (flag) fprintf(vfp, "%c", instanceNuIdx);
    fprintf(vfp, "_");
    fprintf(vfp, "%s", CounterSignalSymTable(instanceName, i - 2));
    fprintf(vfp, " & ("); 

    // _enable
    fprintf(vfp, "%c%d", instanceName[0], regNu);
    if (flag) fprintf(vfp, "%c", instanceNuIdx);
    fprintf(vfp, "_");
    fprintf(vfp, "%s", CounterSignalSymTable(instanceName, i - 1));

    fprintf(vfp, " | ");

    fprintf(vfp, "%c%d", instanceName[0], regNu);
    if (flag) fprintf(vfp, "%c", instanceNuIdx);
    fprintf(vfp, "_");
    fprintf(vfp, "%s_r", CounterSignalSymTable(instanceName, i - 1));

    fprintf(vfp, ");\n\n");

    fprintf(vfp, "always @ (posedge clk)\n");
    fprintf(vfp, "  if (rst)\n");
    fprintf(vfp, "    %c%d", instanceName[0], regNu);
    if (flag) fprintf(vfp, "%c", instanceNuIdx);
    fprintf(vfp, "_");
    fprintf(vfp, "%s_r <= 1'b0;\n", CounterSignalSymTable(instanceName, i - 1));

    fprintf(vfp, "  else if ("); 
    fprintf(vfp, "%c%d", instanceName[0], regNu);
    if (flag) fprintf(vfp, "%c", instanceNuIdx);
    fprintf(vfp, "_");
    fprintf(vfp, "%s", CounterSignalSymTable(instanceName, i - 1));
    fprintf(vfp, ")\n");
    fprintf(vfp, "    %c%d", instanceName[0], regNu);
    if (flag) fprintf(vfp, "%c", instanceNuIdx);
    fprintf(vfp, "_");
    fprintf(vfp, "%s_r <= 1'b1;", CounterSignalSymTable(instanceName, i - 1));
    fprintf(vfp, "\n\n");

    return;
  }

  // Module name and parameters
  if (sum)
    fprintf(vfp, "counter #(%2d, %2d, %5d)\n", minII, IntLog2(minII), sum);
  else {
    fprintf(vfp, "counterM #(%2d, %2d)\n", minII, IntLog2(minII));
  }

  // Instance name
  fprintf(vfp, "cnt_");
  fprintf(vfp, "%c%d", instanceName[0], regNu);
  if (flag) fprintf(vfp, "%c", instanceNuIdx);
  fprintf(vfp, " (");

  // Signal names
  for (i = 0; i < SignalNu; i++) {
    if (i >= 2) {
      // e.g. r1_ or r1a_, r1b_ ; m1_ or m1a_, m1b_
      fprintf(vfp, "%c%d", instanceName[0], regNu);
      if (flag) fprintf(vfp, "%c", instanceNuIdx);
      fprintf(vfp, "_");
    }

    fprintf(vfp, "%s", CounterSignalSymTable(instanceName, i));

    if (i == SignalNu - 1)
      fprintf(vfp, ");\n\n");
    else
      fprintf(vfp, ", ");
  }
}

/*
void GenerateCounterStall(FILE *vfp, int compNu, FSM_outputs_ptr *CtlTable, 
                          char *instanceName) {
  int i, j;
  int cntNu;
  char idx;
  for (i = 0; i < compNu; i++) {
    cntNu = CtlTable[i]->cntNu;
    idx = 'a';
    if ( cntNu > 1) {
      for (j = 0; j < cntNu; j++)
        fprintf(vfp, "assign %c%d%c_stall = stall;\n", // | finish;\n", 
            instanceName[0], i + 1, idx++);
    } else {
        fprintf(vfp, "assign %c%d_stall  = stall;\n", // | finish;\n",
            instanceName[0], i + 1);
    }
  }
  fprintf(vfp, "\n");
}
*/

char *PortSymTable(int op) {
  char *opSym;
  switch (op) {
    case 0: opSym = "stall"; break;
    case 1: opSym = "go"; break;
    case 2: opSym = "in0"; break;
    case 3: opSym = "in1"; break;
    default: { 
      myprintf("Unknown port number %d\n in SymTable", op); 
      opSym = "unknown";
    }
  }
  return opSym;
}


char *CounterSignalSymTable(char *instanceName, int signalNu) {
  char *name;
  switch (signalNu) {
    case 0: name = "clk"; break;
    case 1: name = "rst"; break;
    case 2: name = "stall"; break;
    case 3: name = "enable"; break;
    case 4: {
              if (!strcmp(instanceName, "reg"))
                name = "wen_tmp";
              else if (!strcmp(instanceName, "mux"))
                name = "sel";
              else if (!strcmp(instanceName, "muxM"))
                //name = "count";
                name = "count_enable";
              else
                name = "unknown";
              break;
            }
    case 5: name = "count"; break;

    default: { 
      myprintf("Unknown number %d\n in SignalTable", signalNu); 
      name = "unknown";
    }
  }
  return name;
}

char *ROMSignalSymTable(int signalNu) {
  char *name;
  switch (signalNu) {
    case 0: name = "count"; break;
    case 1: name = "count_enable"; break;
    case 2: name = "out"; break;
    default: { 
      myprintf("Unknown number %d\n in ROMTable", signalNu); 
      name = "unknown";
    }
  }
  return name;
}


/* Update Do file and put it in the local directory */
void GenerateDoFile(char *ScheduleName) {

  FILE *vdo;
  char *fileNamePtr;
  char fileName[100];
  char ch;
  int i, j;
  FILE *src;

  vdo = fopen(GetFileName(fileName, "_TB", ".do", ScheduleName), "w");

  // copy a template TB.do file
  src = fopen("TB.do", "r"); 
  while((ch = getc(src)) != EOF) putc(ch, vdo);
  fclose(src);

  //fprintf(vdo, "vcom -work rtl_work -explicit $LIB_PATH/float.vhd\n");

  // Compile generated HDL file 
  fileNamePtr = GetFileName(fileName, "_HDL", ".v", ScheduleName);
  src = fopen(fileNamePtr, "r");
  if (src != NULL) {
    fprintf(vdo, "vlog -work rtl_work $PROJ_PATH/%s\n", fileNamePtr);
    fclose(src);
  }

  // $bitstoshortreal
  fileNamePtr = GetFileName(fileName, "_TB", ".v", ScheduleName);
  src = fopen(fileNamePtr, "r");
  if (src != NULL) {
    fprintf(vdo, "vlog -sv -work rtl_work $PROJ_PATH/%s\n", fileNamePtr);
    fclose(src);
  }

  fileNamePtr = GetFileName(fileName, "_sim", ".log", ScheduleName);

  //fprintf(vdo, "vsim -t ns -voptargs=+acc work.%s_TB\n", CircuitName);
  fprintf(vdo, "eval vsim $SIM_LIB $SIM_OPTIONS -l $PROJ_PATH/%s work.%s_TB\n",
                fileNamePtr, CircuitName);

  fprintf(vdo, "if {[file exists wave.do]} {\n");
  fprintf(vdo, "  do wave.do\n}\n");

  fprintf(vdo, "run -all\n");
  fprintf(vdo, "quit -f\n");

  fclose(vdo);
}

//=============================================================================
//
// Decscription of RegFileTable structure 
//
// For lifetime of a variable greater than minII,
// 
// e.g.
//
// Suppose minII = 3 and the node has 4 destinations whose
// startTime and endTime are as follows:
// DFG[node]->opDest[0](26/30)
// DFG[node]->opDest[1](26/28)
// DFG[node]->opDest[2](26/27)
// DFG[node]->opDest[3](26/27)
//
// Then the constructed RegFileTable is:
//
// 1. The sharedCnt means the number of list nodes for 
//    a specific Register file port.
//
// 2. The sharedDestNu means the number of destinations
//    of a node whose lifetime are the same
//
// DFG[node]->RegFileTable[0].RegFileList->|opDest[0]        | -> |opDest[1]        | -> NULL  
//                                         |sharedDestNu = 1 |    |sharedDestNu = 1 |
//            RegFileTable[0].sharedCnt = 2 
//            ----------------------------------------------------------------
//            RegFileTable[1].RegFileList->|opDest[2]        | -> NULL
//                                         |opDest[3]        |
//                                       ->|sharedDestNu = 2 |
//            RegFileTable[1].sharedCnt = 1
//            ----------------------------------------------------------------
//            RegFileTable[2].RegFileList->NULL
//            RegFileTable[2].sharedCnt = 0
//            ----------------------------------------------------------------
//            RegFileTable[3].RegFileList->NULL
//            RegFileTable[3].sharedCnt = 0
//            ----------------------------------------------------------------
// 
// For the above table list, the RF has two read ports with one port shared
// by two read addresses (R1a, R1b). Two ports, because at time 30, opDest[2/3]
// read a sencond data (29/30) while opDest[0] read the first data(26/30). 
// So there is a read conflict with only one read output. 
//=============================================================================
void RegFileTableCheckInsert(RF_portPtr RegFileTable,
                            int RF_PortNu, 
                            int curr_child_ID) {
  RF_portNuPtr head;
  int i;
  int child_ID;

  assert(RegFileTable != NULL);
  head = RegFileTable[RF_PortNu-1].RegFileList;

  if (head == NULL)
    //--------------------------------------------------------------------
    // 
    //--------------------------------------------------------------------
    RegFileTableInsert(RegFileTable, RF_PortNu, curr_child_ID);
  else {
    while (head != NULL) {
      //--------------------------------------------------------------------
      // For a multi-destination node, find if the child's end time is equal 
      // to the end time of any other destinations. If equal, update sharedDestNu, 
      // but perform no RegFileListInsert
      //--------------------------------------------------------------------
      child_ID = *(head->sharedDestNuPtr);
      if (DFG[child_ID]->opScheduledSlot == 
          DFG[curr_child_ID]->opScheduledSlot) {
        head->sharedDestNu++;
        head->sharedDestNuPtr = (int *) realloc (head->sharedDestNuPtr, 
                                sizeof(int) * head->sharedDestNu);
        *(head->sharedDestNuPtr + head->sharedDestNu - 1) = curr_child_ID;
        return;
      }
      head = head->next;
    }
    // if equal end time is not found, insert the curr_child_ID node to the Table 
    RegFileTableInsert(RegFileTable, RF_PortNu, curr_child_ID);
  }
}

//==================================================================================
// Return the register file port number(i + 1) or RegFileTable entry
// 1. if curr_child_ID's end time has no conflict
//    with any nodes in the RegFileList at entry RegFileTable[i].
// 2. if curr_child_ID's end time has conflicts with all the nodes in all the entries 
// 
// So it always return i + 1 though the meaning is not the same. 
// One is an allocated or empty RegFileTable entry. Another is a new one.
//==================================================================================
int AddRegFilePortNu (RF_portPtr RegFileTable, int curr_child_ID, int DestNu) {

  RF_portNuPtr head;
  int i, tmp;
  int cnt = 0;
  int endTime, currEndTime;
  int child_ID;

  //currEndTime = DFG[curr_child_ID]->opScheduledSlot;
  currEndTime = GetEndTime(curr_child_ID);

  // Iterate all the entries in RegFileTable
  for (i = 0; i < DestNu; i++) {

    head = RegFileTable[i].RegFileList;
    tmp = cnt;
    while (head != NULL) {
      assert(head->sharedDestNuPtr != NULL);
      child_ID = *(head->sharedDestNuPtr);
      //endTime = DFG[child_ID]->opScheduledSlot;
      endTime = GetEndTime(child_ID);
      
      // Opt1: They can be shared by the same port only when currEndTime == endTime
      if (currEndTime != endTime) {

      // Opt2: When there is no conflict,  they can be shared by the same port 
      //if (currEndTime < endTime && ((endTime - currEndTime) % minII == 0) || \
         (currEndTime > endTime && ((currEndTime - endTime) % minII == 0))) {
        cnt++; 
        break;
      }
      head = head->next;
    }

    //-----------------------------------------------------------------
    // Clearly if break command is executed, then tmp != cnt
    //
    // On the other hand, if the end time of the ith rows has 
    // no conflict with current child ID's end time, 
    // or the ith row is empty,
    // then return the current port number i + 1 since port number
    // starts from 1.
    //-----------------------------------------------------------------
    if (tmp == cnt) return (i+1);
  } // for

  // If the end time of any rows conflicts with current child ID's end time,
  // then we need a new port for this child.
  if (cnt == i) return (i + 1);
}

//==================================================================================
// Add a new current_child_ID to the RegFileList at RegFileTable[RF_PortNu-1]
//==================================================================================
void RegFileTableInsert(RF_portPtr RegFileTable, int RF_PortNu, int curr_child_ID) {
  RF_portNuPtr head, p0, p1, dest;

  p0 = head = RegFileTable[RF_PortNu-1].RegFileList;
  RegFileTable[RF_PortNu-1].sharedCnt++;

  dest = (RF_portNuPtr) malloc (sizeof(RF_DestNu));
  dest->sharedDestNu = 1;
  dest->sharedDestNuPtr = (int *) malloc (sizeof(int));
  *(dest->sharedDestNuPtr) = curr_child_ID;

  if (head == NULL) {
    head = dest;
    dest->next = NULL;
  } else {
    while (p0 != NULL) {
      p1 = p0;
      p0 = p0->next;
    }
    p1->next = dest;
    dest->next = NULL;
  }
  RegFileTable[RF_PortNu-1].RegFileList = head;
}

void FreeRegFileTable (int nodeID) {
  RF_portNuPtr head, p, p1, p2;
  int DestNu = DFG[nodeID]->opDestNu;
  int i;

  RF_portPtr RegFileTable = DFG[nodeID]->RegFileTable;

  for (i = 0; i < DestNu; i++) {
    p = head = RegFileTable[i].RegFileList;
    // For each loop, go to the last node and delete it
    while (p != NULL) {
      p2 = p;
      p1 = p->next;
      if (p1 != NULL) {
        while (p1->next != NULL) {
          p2 = p1;
          p1 = p1->next;
        }
        p2->next = p1->next;
        free(p1->sharedDestNuPtr);
        free(p1);
        //p = head;
      } else {
        free(p->sharedDestNuPtr);
        free(p);
        p = NULL;
        //break; 
      }
    }
  }
  if (RegFileTable != NULL) {
    free(RegFileTable);
    DFG[nodeID]->RegFileTable = NULL;
  }
}

void PrintRegFileTable(RF_portPtr RegFileTable, int DestNu) {
  int i, j;
  RF_portNuPtr head;

  if (RegFileTable != NULL) {
    for (i = 0; i < DestNu; i++) {
      myprintf("RegFileTable[%d]'s size = %d  %p: ", i, RegFileTable[i].sharedCnt, RegFileTable[i]);
      head = RegFileTable[i].RegFileList;
      while (head != NULL) {
        myprintf("(");
        for (j = 0; j < head->sharedDestNu; j++)
          myprintf(" %d", *(head->sharedDestNuPtr + j));
        myprintf(" )");
        head = head->next;
      }
      myprintf("\n");
    }
  }
}

//---------------------------------------------------------------------
// Return simulation file path 
// e.g Modelsim/pos/ASAP1_pos/ASAP1_pos_HDL.v
//---------------------------------------------------------------------
char *GetFileName(char *fileName, char *fileType, char *fileExt, char *ScheduleName) {

  *fileName = '\0';
  // Modelsim dir
  strcpy(fileName, "Modelsim/"); 
  // subdirs
  strcat(fileName, CircuitName); 
  strcat(fileName, "/"); 
  strcat(fileName, ScheduleName); 
  strcat(fileName, CircuitName); 
  strcat(fileName, "/"); 
  // fileName
  strcat(fileName, ScheduleName); 
  strcat(fileName, CircuitName); 
  strcat(fileName, fileType);
  strcat(fileName, fileExt); 
  return fileName;
}

//------------------------------------------------------------------------
// Collect Fanout (Register->MUXp input or Register->FU input) 
// of a simple register or the output of a serial shift register.
//------------------------------------------------------------------------
void GetRegFanOut(int *Map, int k, int RF_PortNu) {

  int depth;
  int *entry;
  int startTime, endTime, lifeTime;
  int child_ID;
  int j;
  RF_portPtr RegFileTable;
  RF_NuCnt head;
  RF_portNuPtr temp;

  if (Map[k] < 0) return;

  entry = RegFanout[Map[k] - 1].entry;
 
  RegFileTable = DFG[k]->RegFileTable; 

  // A simple register
  if (RegFileTable == NULL)  {

    entry[0]++;

  } else {
    //----------------------------------------------------------------
    // A shift register whose sharedCnt at a output port is 1
    // We want to find a map from a RF_port and general register number 
    // to a register entry
    //----------------------------------------------------------------

    startTime = (VarTable[Map[k] -1])->startTime;
    head = RegFileTable[RF_PortNu-1];
    temp = head.RegFileList; 

    if (head.sharedCnt == 1) {
      assert(temp != NULL);
      child_ID = *(temp->sharedDestNuPtr);
      //endTime = DFG[child_ID]->opScheduledSlot;
      endTime = GetEndTime(child_ID);
      lifeTime = endTime - startTime;
      depth = myceil(lifeTime, minII) - 1;
      entry[depth]++;
      temp = temp->next;
    }
  }
}

//=================================================================
// Decrement the scheduled time for an operation node. This is the
// time when a shared register will be written as in the case when
// FU's two input are not registered.
//
// Also consider mux chain register
//=================================================================
int GetEndTime(int nodeID) {
  //if (DFG[nodeID] -> opSrc[0] < 0) {
  enum operation op_type = DFG[nodeID]->op;

  //---------------------------------------------------------------
  // 7/9/11
  // Consider register insertion in front of any FUs.
  // Recall that opScheduledTime is incremented by 1
  // at scheduling phase
  //---------------------------------------------------------------
  
  // delay=1 wo mux chain register insertion 
  // delay=N - muxPtr->grp w/ mux chain register insertion 
#if MUX_OP == 1
  if (op_type == div_) 
#elif MUX_OP == 2
  if (op_type == add) 
#elif MUX_OP == 3
 if (op_type == div_ || op_type == add) 
#elif MUX_OP == 7
 if (op_type == div_ || op_type == add  || op_type == mul) 
#elif MUX_OP == 14
 if (op_type == add  || op_type == mul  || op_type == sub) 
#elif MUX_OP == 15
 if (op_type == div_ || op_type == add  || 
     op_type == mul  || op_type == sub) 
#else
 if (0) 
#endif
    return DFG[nodeID] -> opScheduledSlot - 2;
  else
    return DFG[nodeID] -> opScheduledSlot - 1;
  //} else {
  //  return DFG[nodeID] -> opScheduledSlot;
  //}
}
  
