#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "Schedule.h"

void GenerateHWDebugData(FILE *vfp, char *ModuleName, int testCount, int *FUGoTimeIdx) {
  int goTime; 
  int resourceNu;
  int srcNu;
  enum operation op;
  int i, t, n, cnt = 0;
  int last_goTime = -1;

  fprintf(vfp, "// Debug FUs' input data\n");
  for (i = 0; i < NODE_NU; i++) {
    t = FUGoTimeIdx[i];
    op = DFG[t]->op;
    if (op == nop || op == nopc) continue;
    goTime = DFG[t]->opScheduledSlot;
    resourceNu = DFG[t]->opResourceNu;

    //----------------------------------------------------------------------------------------------
    // run test for testCount
    //
    // sample verilog output
    // initial begin: slot_1
    //   integer f0 = 0; // local variable
    ////   wait (fir16_i.TimeState == 'd1);
    //   wait (fir16_i.state[1]);
    //   repeat (8) begin
    //     #3.000
    //     $fdisplay(FD_dbg[f0], "%t fir16_i.fadd1_in0 = %h(%f)  fir16_i.fadd1_in1 = %h(%f)", 
    //        $time, fir16_i.fadd1_in0, $bitstoshortreal(fir16_i.fadd1_in0), fir16_i.fadd1_in1,
    //        $bitstoshortreal(fir16_i.fadd1_in1));
    //        f0 = f0 + 1;
    //     repeat (6) @(posedge clk);
    //   end
    // end
    //----------------------------------------------------------------------------------------------
    // if more than one FU have the same goTime, they are put
    // into the same verilog initial block to ensure proper output
    //  dump sequence 
    //----------------------------------------------------------------------------------------------

    if (last_goTime != goTime && last_goTime != -1) {
      fprintf(vfp, "%4sf%d = f%d + 1;\n", " ", cnt, cnt);
      fprintf(vfp, "%4srepeat (%d) @(posedge clk);\n", " ", minII);
      fprintf(vfp, "%2send\n", " ");
      fprintf(vfp, "end\n\n");
      cnt++;
    }

    if (last_goTime != goTime) {
      fprintf(vfp, "initial begin: b%d\n", cnt);
      fprintf(vfp, "%2sinteger f%d = 0;\n", " ", cnt);
      fprintf(vfp, "%2swait (%s_i.state['d%d])\n", " ", ModuleName, goTime);
      fprintf(vfp, "%2srepeat (%d) begin\n", " ", testCount);
      fprintf(vfp, "%4s#3\n", " ");
      //fprintf(vfp, "%4s$time,", " "); // if we need time info
    }


    fprintf(vfp, "%4s$fdisplay(FD_dbg[f%d], \"%d:", " ", cnt, t);
    srcNu = GetNodeSrcNu(op);

    for (n = 0; n < srcNu; n++) {
      if (GetOpSym(op) == 'm' && n == 2)
          fprintf(vfp, " %s_i.%s%d_sel = %%0d", ModuleName, opSymTable(op), resourceNu);
      else
        fprintf(vfp, " %s_i.%s%d_in%0d = %%h(%%f)", ModuleName, opSymTable(op), resourceNu, n);
    }

    fprintf(vfp, "\"");


    if (DFG[t]->opPrecision == sfp) {
      for (n = 0; n < srcNu; n++) {
        fprintf(vfp, ",\n");
        if (GetOpSym(op) == 'm' && n == 2)
          fprintf(vfp, "%4s%s_i.%s%d_sel", \
              " ", ModuleName, opSymTable(op), resourceNu);
        else
          fprintf(vfp, "%4s%s_i.%s%d_in%0d, $bitstoshortreal(%s_i.%s%d_in%0d)", \
              " ", ModuleName, opSymTable(op), resourceNu, n, \
                   ModuleName, opSymTable(op), resourceNu, n);
      }
    }

    if (DFG[t]->opPrecision == dfp) {
      for (n = 0; n < srcNu; n++) {
        fprintf(vfp, ",\n");

        if (op == tod) // special case
          fprintf(vfp, "%4s%s_i.%s%d_in%0d, $bitstoshortreal(%s_i.%s%d_in%0d)", \
              " ", ModuleName, opSymTable(op), resourceNu, n, \
                   ModuleName, opSymTable(op), resourceNu, n);

        else if (op == tof) // special case
          fprintf(vfp, "%4s%s_i.%s%d_in%0d, $bitstoreal(%s_i.%s%d_in%0d)", \
              " ", ModuleName, opSymTable(op), resourceNu, n, \
                   ModuleName, opSymTable(op), resourceNu, n);

        else if (GetOpSym(op) == 'm' && n == 2) // special case
            fprintf(vfp, "%4s%s_i.%s%d_sel", \
                " ", ModuleName, opSymTable(op), resourceNu);
        else
          fprintf(vfp, "%4s%s_i.%s%d_in%0d, $bitstoreal(%s_i.%s%d_in%0d)", \
              " ", ModuleName, opSymTable(op), resourceNu, n, \
                   ModuleName, opSymTable(op), resourceNu, n);
      }
    }

    fprintf(vfp, ");\n");

    last_goTime = goTime;
  }
  // last slot
  fprintf(vfp, "%4sf%d = f%d + 1;\n", " ", cnt, cnt);
  fprintf(vfp, "%4srepeat (%d) @(posedge clk);\n", " ", minII);
  fprintf(vfp, "%2send\n", " ");
  fprintf(vfp, "end\n\n");
} 

void GenerateSWDebugData(FILE *cfp, char *ModuleName, int testCount, int *FUGoTimeIdx) {

  // Open CircuitName_test.data and CircuitName_debug.txt
  char srcFileName[50];

#ifdef FLOATING_POINT
  sprintf(srcFileName, "TestData/%s_test.data", CircuitName);
  //const float MINUS1 = -1.0; 
#else
  sprintf(srcFileName, "TestData/%s_test.data.bak", CircuitName);
  //const int MINUS1 = -1; 
#endif

  FILE *fs = fopen(srcFileName, "r");

  int32_or_float opA, opB, opS, res;
  int32_or_float opC, opD;

  int64_or_double opA_d, opB_d, opS_d, res_d;
  int64_or_double opC_d, opD_d;

  int m, n;
  int i, j, k;
  int t;
  int destNu;
  int op, resourceNu;
  int size;
  int K;

  //-------------------------------------------------------------
  // Set up debug structures
  //-------------------------------------------------------------
  for (i = 0; i < NODE_NU; i++) {

    if (DFG[i]->opSrc[0] < 0)
      continue;

    else {
      //--------------------------------------------------------
      // s2d-2:    0 - s,   1 - out
      // d2s-2:    0 - d,   1 - out
      // mx-4:     0 - sel, 1 - in0, 2 - in1, 3 - out
      // rom-5:    0 - in0, 1 - in1, 2 - in2, 3 - in3, 4 - out
      // others-3: 0 - in0, 1 - in1, 2 - out
      //--------------------------------------------------------

      size = GetNodeSrcNu(DFG[i]->op) + 1;

      DFG[i]->opVal = NULL;
      DFG[i]->opVal_d = NULL;

      if (DFG[i]->op == tod || DFG[i]->op == tof) { // special cases
        DFG[i]->opVal   = (float *) malloc (sizeof(float) * size);
        DFG[i]->opVal_d = (double *) malloc (sizeof(double) * size);
      }

      else if (DFG[i]->opPrecision == sfp) 
        DFG[i]->opVal = (float *) malloc (sizeof(float) * size);

      else if (DFG[i]->opPrecision == dfp) 
        DFG[i]->opVal_d = (double *) malloc (sizeof(double) * size);
    }
  }

  //-------------------------------------------------------------
  // Set up a rom structures if it exists
  //-------------------------------------------------------------
  int32_or_float **mem = NULL;
  int64_or_double **mem_d = NULL;

  if (UseROM(rom) || UseROM(romd)) {
    float val = 0.01; // initial memory fill value
    double val_d = 0.01; // initial memory fill value

    if (UseROM(rom)) {
      mem = (int32_or_float **) malloc (sizeof(int32_or_float *) * WORDS);
      for (j = 0; j < WORDS; j++) {
        mem[j] = (int32_or_float *) malloc (sizeof(int32_or_float) * DEPTH);
        for (i = 0; i < DEPTH; i++) {
          mem[j][i].f = val++;  
        }
      }
    }

    if (UseROM(romd)) {
      mem_d = (int64_or_double **) malloc (sizeof(int64_or_double *) * WORDS);
      for (j = 0; j < WORDS; j++) {
        mem_d[j] = (int64_or_double *) malloc (sizeof(int64_or_double) * DEPTH);
        for (i = 0; i < DEPTH; i++) {
          mem_d[j][i].f = val_d++;
        }
      }
    }
  }

  //-------------------------------------------------------------
  // load test data
  //-------------------------------------------------------------
  int32_or_float *ports = 
  (int32_or_float *) malloc (sizeof(int32_or_float) * MAX_PORT_NU);

#ifdef MRBAY

  // a hardcoded part to load PL and PR data into the inputs of multipliers
  float tr_mat[] = {
      0.90638, 0.0312067, 0.0312067, 0.0312067,
    0.0312067,   0.90638, 0.0312067, 0.0312067,
    0.0312067, 0.0312067,   0.90638, 0.0312067,
    0.0312067, 0.0312067, 0.0312067,   0.90638 };

  int base, ofst;
  for (i = 0; i < NODE_NU; i++) {
    // see mrbay.txt
    // PLAA(0), PLAC(1), PLAG(2), PLAT(3), PRAA(0), PRAC(1), PRAG(2), PRAT(3), 
    // ....
    // PLGA, PLGC, PLGG, PLGT, PRGA, PRGC, PRGG, PRGT
    if ((int)DFG[i]->opConstVal >= 6 && (int)DFG[i]->opConstVal <= 37 && 
        DFG[i]->op == nopc) {
      base = ((int)(DFG[i]->opConstVal) - 6) / 8;
      base *= 4;
      ofst = ((int)(DFG[i]->opConstVal) - 6) % 4;
      DFG[DFG[i]->opDest[0]]->opVal[0] = tr_mat[base+ofst];  
      myprintf("DFG[%d]->opVal = %f\n", DFG[i]->opDest[0], DFG[DFG[i]->opDest[0]]->opVal[0]);
    }
  }


#endif

  for (m = 0; m < testCount; m++) {

    for (n = 0; n < MAX_PORT_NU; n++)
      fscanf(fs, "%x", &(ports[n].i));

    for (i = 0; i < NODE_NU; i++) {

      //===========================================
      // proprogate 32-bit input port values
      // TODO mixed input port width
      //
      // Application-specific ports are also considered
      // as constants (app: mrbay)
      //===========================================
      if (DFG[i]->opSrc[0] < 0) {
        for (j = 0; j < DFG[i]->opDestNu; j++) {
          destNu = DFG[i]->opDest[j];
          if (destNu < 0) continue;  // a dummy input node ?

          K = GetNodeSrcNu(DFG[destNu]->op);
          for (k = 0; k < K; k++) {
            if (i == DFG[destNu]->opSrc[k])
              #ifdef FLOATING_POINT

              // TODO mixed port width
              if (DFG[destNu]->op == tod && DFG[i]->opPrecision == sfp) {
                DFG[destNu]->opVal[0] = ports[i].f;
              }

              else if (GetOpSym(DFG[destNu]->op) == 'm' && k == 0)  {
                //======================================================
                // suppose circuit input port (width > 1) 
                // connected to mux sel (width = 1)
                // hardcoded for Norm since it is an input to mux(m)
                //======================================================
                if (DFG[i]->op == nopc)
                  DFG[destNu]->opVal[k] = 1;      // mrbay app: Norm = 1 (always scaling)
                else
                  DFG[destNu]->opVal[k] = ports[i].i & 1;
              }

              else {
              // TODO mixed port width
              // We need to differentiate different nopc inputs: 
              // 1 tipL and tipR
              // 2 IIA, IIC, IIG, IIT
                  if (DFG[i]->op == nopc) {
                    if (DFG[i]->opPrecision == dfp) {
                      DFG[destNu]->opVal_d[k] = 0.25; // original mrbay app: IIA, IIC, IIG, IIT (1/4)
                    }
                    if (DFG[i]->opPrecision == sfp & i <= 14 ) { // 14: see mrbay.txt
                      DFG[destNu]->opVal[k] = 0.25; // mrbay app: IIA, IIC, IIG, IIT (1/4)
                    }
                  }
                  else
                    DFG[destNu]->opVal[k] = ports[i].f; 
              }

              #else
              DFG[destNu]->opVal[k] = ports[i].i;
              #endif
          }
        }
      } else {

        // get and save operation result
        if (DFG[i]->op == tod)
          opA.f = DFG[i]->opVal[0];

        if (DFG[i]->op == tof)
          opA_d.f = DFG[i]->opVal_d[0];

        else if (DFG[i]->op == mx) {
          opS.f = DFG[i]->opVal[0];

          opA.f = DFG[i]->opSrc[1] == -1 ? 
            DFG[i]->opMuxVal[0] : DFG[i]->opVal[1];

          opB.f = DFG[i]->opSrc[2] == -1 ?
            DFG[i]->opMuxVal[1] : DFG[i]->opVal[2];
        }

        else if (DFG[i]->op == mxd) {
          opS_d.f = DFG[i]->opVal_d[0];

          opA_d.f = DFG[i]->opSrc[1] == -1 ? 
            DFG[i]->opMuxVal_d[0] : DFG[i]->opVal_d[1];

          opB_d.f = DFG[i]->opSrc[2] == -1 ?
            DFG[i]->opMuxVal_d[1] : DFG[i]->opVal_d[2];
        }

        else if (DFG[i]->op == rom) {
          opA.i = (int)DFG[i]->opVal[0]; // A0
          opB.i = (int)DFG[i]->opVal[1]; // A1
          opC.i = (int)DFG[i]->opVal[2]; // A2
          opD.i = (int)DFG[i]->opVal[3]; // A3

          assert(opA.i == 0 || opA.i == 1);
          assert(opB.i == 0 || opB.i == 1);
          assert(opC.i == 0 || opC.i == 1);
          assert(opD.i == 0 || opD.i == 1);
        }

        else if (DFG[i]->op == romd) {
          opA_d.i = (int)DFG[i]->opVal_d[0]; // A0
          opB_d.i = (int)DFG[i]->opVal_d[1]; // A1
          opC_d.i = (int)DFG[i]->opVal_d[2]; // A2
          opD_d.i = (int)DFG[i]->opVal_d[3]; // A3

          assert(opA_d.i == 0 || opA_d.i == 1);
          assert(opB_d.i == 0 || opB_d.i == 1);
          assert(opC_d.i == 0 || opC_d.i == 1);
          assert(opD_d.i == 0 || opD_d.i == 1);
        }

        else {
          if (DFG[i]->opPrecision == sfp) {
            opA.f = DFG[i]->opVal[0];
            opB.f = DFG[i]->opConst ? DFG[i]->opConstVal : DFG[i]->opVal[1];
          }

          else if (DFG[i]->opPrecision == dfp) {
            opA_d.f = DFG[i]->opVal_d[0];
            opB_d.f = DFG[i]->opConst ? DFG[i]->opConstVal_d : 
                                        DFG[i]->opVal_d[1];
          }
        }

        //------------------------------------------------------------------
        // Behavioral arithmetic functions
        //------------------------------------------------------------------


        if (DFG[i]->op == tod) 
          res_d.f = (double) opA.f; 

        else if (DFG[i]->op == tof) 
          res.f = (float) opA_d.f; 

        else if (DFG[i]->opPrecision == sfp) {
          switch(GetOpSym(DFG[i]->op)) {
            case '+' : res.f = opA.f + opB.f; break;
            case '-' : res.f = opA.f - opB.f; break;
            case '*' : res.f = opA.f * opB.f; break;
            case '/' : res.f = opA.f / opB.f; break;
            case '<' : res.f = opA.f < opB.f; break;
            case '>' : res.f = opA.f > opB.f; break;
            case 'm' : res.f = (int)opS.f ? opB.f : 
                                       opA.f; break; 
            case 'x' : res.f = opA.f > opB.f ? 
                               opA.f : opB.f; break; 
            case 'r' : // rom read 
                       // Hack DFG[i]->opConstVal to store ck(i.e. c0 .. c4)
                       k = (int) DFG[i]->opConstVal;
                       res.f = mem[k-1][opD.i*8+opC.i*4+opB.i*2+opA.i].f;
                       break; 
            default: {
              printf("Unknown op %d\n", DFG[i]->op); 
            }
          }
        }

        else if (DFG[i]->opPrecision == dfp) {
          switch(GetOpSym(DFG[i]->op)) {
            case '+' : res_d.f = opA_d.f + opB_d.f; break;
            case '-' : res_d.f = opA_d.f - opB_d.f; break;
            case '*' : res_d.f = opA_d.f * opB_d.f; break;
            case '/' : res_d.f = opA_d.f / opB_d.f; break;
            case '<' : res_d.f = opA_d.f < opB_d.f; break;
            case '>' : res_d.f = opA_d.f > opB_d.f; break;
            case 'm' : res_d.f = (int)opS_d.f ? opB_d.f :
                                           opA_d.f; break; 
            case 'x' : res_d.f = opA_d.f > opB_d.f ? 
                                 opA_d.f : opB_d.f; break; 
            case 'r' : // rom read 
                       // Hack DFG[i]->opConstVal to store ck(i.e. c0 .. c4)
                       k = (int) DFG[i]->opConstVal_d;
                       res_d.f = mem_d[k-1][opD_d.i*8+opC_d.i*4+opB_d.i*2+opA_d.i].f;
                       break; 
            default: {
              printf("Unknown op %d\n", DFG[i]->op); 
            }
          }
        }

        //------------------------------------------------------------------
        // Save arithmetic function result
        //------------------------------------------------------------------

        if (DFG[i]->op == tod) 
          DFG[i]->opVal_d[0] = res_d.f; 

        if (DFG[i]->op == tof) 
          DFG[i]->opVal[0] = res.f; 
        
        else if (DFG[i]->op == mx) 
          DFG[i]->opVal[3] = res.f; // opVal[3] has the mux output result!
        
        else if (DFG[i]->op == mxd) 
          DFG[i]->opVal_d[3] = res_d.f; // opVal[3] has the mux output result!

        else if (DFG[i]->op == rom) 
          DFG[i]->opVal[4] = res.f; 

        else if (DFG[i]->op == romd) 
          DFG[i]->opVal_d[4] = res_d.f; 

        else if (DFG[i]->opPrecision == sfp)
          DFG[i]->opVal[2] = res.f;

        else if (DFG[i]->opPrecision == dfp)
          DFG[i]->opVal_d[2] = res_d.f;

        //-------------------------------------------------------------------
        // proprogate operation result
        //-------------------------------------------------------------------
        for (j = 0; j < DFG[i]->opDestNu; j++) {
          destNu = DFG[i]->opDest[j];
          if (destNu > 0) {
            K = GetNodeSrcNu(DFG[destNu]->op);
            for (k = 0; k < K; k++) {
              if (i == DFG[destNu]->opSrc[k]) {

                if (!DFG[destNu]->opConst) {

                  // special case (result of tod assigned to a single value)
                  if (DFG[destNu]->op == tod)
                    DFG[destNu]->opVal[k] = 
                                          DFG[i]->op == rom ? DFG[i]->opVal[4] : 
                                          DFG[i]->op == mx  ? DFG[i]->opVal[3] : 
                                          DFG[i]->op == tof ? DFG[i]->opVal[0] : 
                                                              DFG[i]->opVal[2];
                  else if (DFG[destNu]->op == tof)
                    DFG[destNu]->opVal_d[k] = 
                                          DFG[i]->op == romd ? DFG[i]->opVal_d[4] : 
                                          DFG[i]->op == mxd  ? DFG[i]->opVal_d[3] : 
                                          DFG[i]->op == tod  ? DFG[i]->opVal_d[0] : 
                                                               DFG[i]->opVal_d[2];

                  else if (DFG[destNu]->opPrecision == sfp) {
                    DFG[destNu]->opVal[k] = 
                                          DFG[i]->op == tof ? DFG[i]->opVal[0] : 
                                          DFG[i]->op == rom ? DFG[i]->opVal[4] : 
                                          DFG[i]->op == mx  ? DFG[i]->opVal[3] : 
                                                              DFG[i]->opVal[2];
                  }

                  else if (DFG[destNu]->opPrecision == dfp) {
                    //-------------------------------------------------------
                    // Application specific case (app: mrbay)
                    // gt --> romd
                    //-------------------------------------------------------
                    if (DFG[destNu]->op == romd && DFG[i]->opPrecision == sfp) {

                      if (!strcmp(opTypeTable(DFG[i]->op), "compare")) 
                        DFG[destNu]->opVal_d[k] = DFG[i]->opVal[2]; 

                      //if (!strcmp(opTypeTable(DFG[i]->op), "memory")) 
                        //DFG[destNu]->opVal_d[k] = DFG[i]->opVal[4]; 
                    }
                    //-------------------------------------------------------
                    // Application specific case (app: mrbay) ends
                    // gt --> romd
                    //-------------------------------------------------------
                    else
                      DFG[destNu]->opVal_d[k] = 
                                            DFG[i]->op == tod  ? DFG[i]->opVal_d[0] : 
                                            DFG[i]->op == romd ? DFG[i]->opVal_d[4] : 
                                            DFG[i]->op == mxd  ? DFG[i]->opVal_d[3] : 
                                                                 DFG[i]->opVal_d[2];
                  }
                }
                else {

                  // Assume a 2-input node only
                  assert(K == 2);

                  if (DFG[destNu]->opPrecision == sfp) {
                    DFG[destNu]->opVal[0] = 
                                          DFG[i]->op == rom ? DFG[i]->opVal[4] : 
                                          DFG[i]->op == mx  ? DFG[i]->opVal[3] : 
                                                              DFG[i]->opVal[2];
                    DFG[destNu]->opVal[1] = DFG[i]->opConstVal;
                  }

                  else if (DFG[destNu]->opPrecision == dfp) {
                    DFG[destNu]->opVal_d[0] = 
                                          DFG[i]->op == romd ? DFG[i]->opVal_d[4] : 
                                          DFG[i]->op == mxd  ? DFG[i]->opVal_d[3] : 
                                                               DFG[i]->opVal_d[2];
                    DFG[destNu]->opVal_d[1] = DFG[i]->opConstVal_d;
                  }
                }
              }

              // a MUX node with one or two input constants
              else if (K == 3 && DFG[destNu]->opSrc[k] == -1) {

                // assume mux select is not a constant
                if (DFG[destNu]->opPrecision == sfp)
                  DFG[destNu]->opVal[k] = DFG[destNu]->opMuxVal[k-1];

                else if (DFG[destNu]->opPrecision == dfp)
                  DFG[destNu]->opVal_d[k] = DFG[destNu]->opMuxVal_d[k-1];
              }
            }
          }
        }
      }
    }

    //-------------------------------------------------------------
    // prepare FU input/output data and dump them for an untimed C model 
    // The time sequences is directed by FU goTime
    //-------------------------------------------------------------

    fprintf(cfp, "test %d\n", m);

    for (i = 0; i < NODE_NU; i++) {
      t = FUGoTimeIdx[i];
      op = DFG[t]->op;
      if (op == nop || op == nopc) continue;

      resourceNu = DFG[t]->opResourceNu;

      #ifdef FLOATING_POINT

      if (DFG[t]->op == mx) {
        opS.f = DFG[t]->opVal[0];
        opA.f = DFG[t]->opSrc[1] == -1 ? DFG[t]->opMuxVal[0] : DFG[t]->opVal[1];
        opB.f = DFG[t]->opSrc[2] == -1 ? DFG[t]->opMuxVal[1] : DFG[t]->opVal[2];
        res.f = DFG[t]->opVal[3];
      }
      else if (DFG[t]->op == mxd) {
        opS_d.f = DFG[t]->opVal_d[0];
        opA_d.f = DFG[t]->opSrc[1] == -1 ? DFG[t]->opMuxVal_d[0] : DFG[t]->opVal_d[1];
        opB_d.f = DFG[t]->opSrc[2] == -1 ? DFG[t]->opMuxVal_d[1] : DFG[t]->opVal_d[2];
        res_d.f = DFG[t]->opVal_d[3];
      }
      else if (DFG[t]->op == tod) {
        opA.f   = DFG[t]->opVal[0];
        res_d.f = DFG[t]->opVal_d[0];
      }
      else if (DFG[t]->op == tof) {
        opA_d.f   = DFG[t]->opVal_d[0];
        res.f = DFG[t]->opVal[0];
      }
      else if (DFG[t]->op == rom) {
        opA.f = DFG[t]->opVal[0];
        opB.f = DFG[t]->opVal[1];
        opC.f = DFG[t]->opVal[2];
        opD.f = DFG[t]->opVal[3];
        res.f = DFG[t]->opVal[4];
      }
      else if (DFG[t]->op == romd) {
        opA_d.f = DFG[t]->opVal_d[0];
        opB_d.f = DFG[t]->opVal_d[1];
        opC_d.f = DFG[t]->opVal_d[2];
        opD_d.f = DFG[t]->opVal_d[3];
        res_d.f = DFG[t]->opVal_d[4];
      }

      else {
        if (DFG[t]->opPrecision == sfp) {
          opA.f = DFG[t]->opVal[0];
          opB.f = DFG[t]->opConst ? DFG[t]->opConstVal : DFG[t]->opVal[1];
          res.f = DFG[t]->opVal[2];
        }

        else if (DFG[t]->opPrecision == dfp) {
          opA_d.f = DFG[t]->opVal_d[0];
          opB_d.f = DFG[t]->opConst ? DFG[t]->opConstVal_d : DFG[t]->opVal_d[1];
          res_d.f = DFG[t]->opVal_d[2];
        }
      }

      #else

      opA.i = DFG[t]->opVal[0];
      opB.i = DFG[t]->opConst ? DFG[t]->opConstVal : DFG[t]->opVal[1];
      res.i = DFG[t]->opVal[2];

      #endif

      if (DFG[t]->op == tod)
        fprintf(cfp, "%d: %s_i.%s%d_in0 = %08x(%.6f) res = %016llx(%.6f)\n",\
            t, ModuleName, opSymTable(op), resourceNu, opA.i, opA.f, res_d.i, res_d.f);

      else if (DFG[t]->op == tof)
        fprintf(cfp, "%d: %s_i.%s%d_in0 = %016llx(%.6f) res = %08x(%.6f)\n",\
            t, ModuleName, opSymTable(op), resourceNu, opA_d.i, opA_d.f, res.i, res.f);

      else if (DFG[t]->op == mx)
        fprintf(cfp, "%d: %s_i.%s%d_in0 = %08x(%.6f)  %s_i.%s%d_in1 = %08x(%.6f) %s_i.%s%d_sel = %0d res = %08x(%.6f)\n",\
            t, ModuleName, opSymTable(op), resourceNu, opA.i, opA.f, \
               ModuleName, opSymTable(op), resourceNu, opB.i, opB.f, \
               ModuleName, opSymTable(op), resourceNu, (int)opS.f, res.i, res.f);

      else if (DFG[t]->op == mxd)
        fprintf(cfp, "%d: %s_i.%s%d_in0 = %016llx(%.6f)  %s_i.%s%d_in1 = %016llx(%.6f) %s_i.%s%d_sel = %0d res = %016llx(%.6f)\n",\
            t, ModuleName, opSymTable(op), resourceNu, opA_d.i, opA_d.f, \
               ModuleName, opSymTable(op), resourceNu, opB_d.i, opB_d.f, \
               ModuleName, opSymTable(op), resourceNu, (int)opS_d.f, res_d.i, res_d.f);

      else if (DFG[t]->op == rom)
        fprintf(cfp, 
            //"%d: %s_i.%s%d_in0 = %08x(%.6f)  %s_i.%s%d_in1 = %08x(%.6f) %s_i.%s%d_in2 = %08x(%.6f)  %s_i.%s%d_in3 = %08x(%.6f) res = %08x(%.6f)\n",
            "%d: %s_i.%s%d_in0 = %0d  %s_i.%s%d_in1 = %0d %s_i.%s%d_in2 = %0d %s_i.%s%d_in3 = %0d res = %08x(%.6f)\n",
            t, ModuleName, opSymTable(op), resourceNu, (int)opA.f, \
               ModuleName, opSymTable(op), resourceNu, (int)opB.f, \
               ModuleName, opSymTable(op), resourceNu, (int)opC.f, \
               ModuleName, opSymTable(op), resourceNu, (int)opD.f, res.i, res.f);

      else if (DFG[t]->op == romd)
        fprintf(cfp, 
            //"%d: %s_i.%s%d_in0 = %016llx(%.6f)  %s_i.%s%d_in1 = %016llx(%.6f) %s_i.%s%d_in2 = %016llx(%.6f)  %s_i.%s%d_in3 = %016llx(%.6f) res = %016llx(%.6f)\n", 
            "%d: %s_i.%s%d_in0 = %0d  %s_i.%s%d_in1 = %0d %s_i.%s%d_in2 = %0d  %s_i.%s%d_in3 = %0d res = %016llx(%.6f)\n", 
            t, ModuleName, opSymTable(op), resourceNu, (int)opA_d.f, \
               ModuleName, opSymTable(op), resourceNu, (int)opB_d.f, \
               ModuleName, opSymTable(op), resourceNu, (int)opC_d.f, \
               ModuleName, opSymTable(op), resourceNu, (int)opD_d.f, res_d.i, res_d.f);

      else {
        if (DFG[t]->opPrecision == sfp)
          fprintf(cfp, "%d: %s_i.%s%d_in0 = %08x(%.6f)  %s_i.%s%d_in1 = %08x(%.6f) res = %08x(%.6f)\n", 
              t, ModuleName, opSymTable(op), resourceNu, opA.i, opA.f, \
                 ModuleName, opSymTable(op), resourceNu, opB.i, opB.f, res.i, res.f);

        else if (DFG[t]->opPrecision == dfp)
          fprintf(cfp, "%d: %s_i.%s%d_in0 = %016llx(%.6f)  %s_i.%s%d_in1 = %016llx(%.6f) res = %016llx(%.6f)\n", 
              t, ModuleName, opSymTable(op), resourceNu, opA_d.i, opA_d.f, \
                 ModuleName, opSymTable(op), resourceNu, opB_d.i, opB_d.f, res_d.i, res_d.f);
      }
    }
    fprintf(cfp, "\n");
  } // for testCount

  free(ports);
  fclose(fs);
  fclose(cfp);

  if (mem != NULL) {
    for (i = 0; i < WORDS; i++) free(mem[i]);
    free(mem);
  }

  if (mem_d != NULL) {
    for (i = 0; i < WORDS; i++) free(mem_d[i]);
    free(mem_d);
  }

  // free debug structure 
  for (i = 0; i < NODE_NU; i++) {
    if (DFG[i]->opSrc[0] < 0) continue;
    else {
      if (DFG[i]->opVal != NULL) free(DFG[i]->opVal);
      if (DFG[i]->opVal_d != NULL) free(DFG[i]->opVal_d);
    }
  }
}

