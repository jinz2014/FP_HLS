#include <stdio.h>
#include <assert.h>
#include "Schedule.h"
#include "MUX.h"


void GenerateSymbolicFSM (FILE *vfp, int *Map, int SharedRegNu, 
    int *MuxCnt, int *MuxCntTable, 
    int delay, char *ScheduleName) {

  MuxSelList *head = NULL;
  int i;

  // 9/1/2012 
  // divide the MUXs into two group
  MuxChainPartition();


  //fprintf(vfp, "\n--------------Mux sel --------------\n");
  
  if (*MuxCnt != 0) {
    CreateMuxSelTable(*MuxCnt);
    myprintf("MuxSelTable size is %d\n", *MuxCnt);

    for (i = 0; i < NODE_NU; i++) {
      if (DFG[i]->opSrc[0] < 0) {
        // port node wouldn't generate any MUXP 
        GenerateSymbolicMuxSel2(i, Map, MuxCntTable);
      } else {
        GenerateSymbolicMuxSel1(i, Map);
        GenerateSymbolicMuxSel2(i, Map, MuxCntTable);
      }
    }

    // Debug MuxSelList
    PrintMuxSelList(MuxCnt);

    // 
    AddMuxRegister(Map);
  }
}

int MuxDestMatch (int nodeNu, MuxPtr p, int i) {
  int k;
  int parentMuxID;
  MuxPtr child;
  MuxPtr tmp = p;
  
  if (nodeNu == *(tmp->Mi->destNu + i))
    return 1;
  else {
    while (*(tmp->Mo->fuNu) == -1) {

      parentMuxID = tmp->m;
      child = tmp->next;

      for (k = 0; k < 2; k++) {
        if (*(child->Mi->muxNu+k) == parentMuxID) {
          if (nodeNu == *(child->Mi->destNu + k))
            return 1;
        }
      }
      tmp = child;
    }
    return 0;
  }
}

void GenerateSymbolicMuxSel(MuxPtr muxPtr, int sel, int nodeNu) {

  int i;
  MuxPtr child, parent;
  ////int parentMuxID;
  int startTime;
  int delay;
  int selTime;
  enum operation op_type = DFG[nodeNu]->op;

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
    delay = 2 - muxPtr->grp;
  else
    delay = 1;

  startTime = DFG[nodeNu]->opScheduledSlot - delay;

  myprintf("Phase1: mux%d sel = %d at time slot %d\n", 
                muxPtr->m, sel, startTime); 

  MuxSelListInsert(muxPtr->m - 1, startTime, sel, 2);

  while (*(muxPtr->Mo->fuNu) == -1) {

    parent= muxPtr;
    //parentMuxID = parent->m;
    child = muxPtr->next;

    for (i = 0; i < 2; i++) {
      if (*(child->Mi->muxNu+i) == parent->m) {

        if (parent->grp < child->grp) {
          selTime = startTime + 1;
        }
        else if (parent->grp == child->grp) {
          selTime = startTime;
        }
        else {
          printf("mux group error!");
          exit(1);
        }

        myprintf("Phase1 recursive: mux%d sel = %d at time slot %d\n", 
                      child->m, i, selTime/*startTime*/);

        MuxSelListInsert(child->m - 1, selTime/*startTime*/, i, 2);
      }
    }
    muxPtr = child;
    startTime = selTime; 
  }
}

//----------------------------------------------------------
// Phase1 symbolic Mux sel
//----------------------------------------------------------
void GenerateSymbolicMuxSel1(int nodeNu, int *Map) {
  MuxPtr tmp, p, head; 
  int op_type, fu_nu, port_nu;
  int offset;
  int r, ri, rj;
  int isConst;
  int Ri, Rj;
  int flag0, flag1;
  int flag;
  int i;

  float constVal;
  float Ci, Cj;
  double constVal_d;
  double Ci_d, Cj_d;
  

  op_type = DFG[nodeNu]->op;
  offset = 0; 
  fu_nu = DFG[nodeNu]->opResourceNu - offset - 1;


  isConst  = DFG[nodeNu]->opConst;
  constVal = DFG[nodeNu]->opConstVal;     // single
  constVal_d = DFG[nodeNu]->opConstVal_d; // double

  // Get Fu(i)'s sources 
  //ri = DFG[nodeNu]->opSrc[0];
  //rj = DFG[nodeNu]->opSrc[1];

  int portNu = GetNodeSrcNu(op_type);

  //for (port_nu = 0; port_nu < 2; port_nu++) {
  for (port_nu = 0; port_nu < portNu; port_nu++) {

    flag0 = flag1 = 0;
    flag = 0;
    r = DFG[nodeNu]->opSrc[port_nu];
    p = head = (MuxRAT[op_type][fu_nu]).Fu_portNu[port_nu];

    while (p != NULL) {

      Ri = *(p->Mi->regNu);     // logical register number of mux input0
      Rj = *(p->Mi->regNu + 1); // logical register number of mux input1
      Ci = *(p->Mi->constNu);     // logical register number of mux input0
      Cj = *(p->Mi->constNu + 1); // logical register number of mux input1
      Ci_d = *(p->Mi->constNu_d);     // logical register number of mux input0
      Cj_d = *(p->Mi->constNu_d + 1); // logical register number of mux input1

      myprintf("Node%d Ri=%d Rj=%d Map[Ri]=%d Map[Rj]=%d\n", 
          nodeNu, Ri, Rj, Map[Ri], Map[Rj]);

      if (Ri == Rj) { // P2 (Map[Ri] = Map[Rj]) in MuxAlloc.c

        myprintf("Ri(%d) == Rj(%d) r = %d port = %d\n", 
          Ri, Rj, r, port_nu);

        if (DFG[nodeNu]->opPrecision == sfp) {
          assert(Ci != Cj);

          if (constVal == Ci && Ci != 0) {
            GenerateSymbolicMuxSel(p, 0, nodeNu); 
            flag = 1;
          } 

          if (constVal == Cj && Cj != 0) {
            GenerateSymbolicMuxSel(p, 1, nodeNu); 
            flag = 1;
          }

          if (Ci == 0 && r == Ri) {
            GenerateSymbolicMuxSel(p, 0, nodeNu); 
            flag = 1;
          } 

          if (Cj == 0 && r == Rj) {
            GenerateSymbolicMuxSel(p, 1, nodeNu); 
            flag = 1;
          }

          assert(flag == 1); // the four conditions are complete ?
        }

        if (DFG[nodeNu]->opPrecision == dfp) {

          assert(Ci_d != Cj_d);

          if (constVal_d == Ci_d && Ci_d != 0) {
            GenerateSymbolicMuxSel(p, 0, nodeNu); 
            flag = 1;
          } 

          if (constVal_d == Cj_d && Cj_d != 0) {
            GenerateSymbolicMuxSel(p, 1, nodeNu); 
            flag = 1;
          }

          if (Ci_d == 0 && r == Ri) {
            GenerateSymbolicMuxSel(p, 0, nodeNu); 
            flag = 1;
          } 

          if (Cj_d == 0 && r == Rj) {
            GenerateSymbolicMuxSel(p, 1, nodeNu); 
            flag = 1;
          }

          assert(flag == 1);
        }

      } else if (Map[Ri] == Map[Rj]) {

        myprintf("Map[%d] == Map[%d] r = %d port = %d constVal=%f Ci=%f Cj=%f\n", 
          Ri, Rj, r, port_nu, constVal, Ci, Cj);

        if (DFG[nodeNu]->opPrecision == sfp) {
          //assert(Ci != 0 || Cj != 0);

          if (r == Ri) {
            assert(port_nu == 1);
            GenerateSymbolicMuxSel(p, 0, nodeNu); 
            flag = 1;
          }

          if (r == Rj) {
            assert(port_nu == 1);
            GenerateSymbolicMuxSel(p, 1, nodeNu); 
            flag = 1;
          }
        }

        if (DFG[nodeNu]->opPrecision == dfp) {
          //assert(Ci_d != 0 || Cj_d != 0);

          if (r == Ri) {
            assert(constVal_d == Ci_d);
            GenerateSymbolicMuxSel(p, 0, nodeNu); 
            flag = 1;
          }

          if (r == Rj) {
            assert(constVal_d == Cj_d);
            GenerateSymbolicMuxSel(p, 1, nodeNu); 
            flag = 1;
          }
        }

      }
      else {
       
        myprintf("(Map[Ri=%d] = %d) != (Map[Rj = %d] = %d) & (Map[r = %d] = %d) & port = %d\n", 
            Ri, Map[Ri], Rj, Map[Rj], r, Map[r], port_nu);
        if (isConst == 1 && port_nu == 1) {
          myprintf("is const flag0/1\n");
          for (i = 0; i < 2; i++) {

            if (DFG[nodeNu]->opPrecision == sfp)
              if (constVal == *(p->Mi->constNu + i))
                 GenerateSymbolicMuxSel(p, i, nodeNu); 

            if (DFG[nodeNu]->opPrecision == dfp)
              if (constVal_d == *(p->Mi->constNu_d + i))
                 GenerateSymbolicMuxSel(p, i, nodeNu); 
          }
          flag = 1;
        } else {

          if (Map[r] == Map[Ri] \
              && nodeNu == *(p->Mi->destNu))  {
              //&& MuxDestMatch(nodeNu, p, 0)) 
            GenerateSymbolicMuxSel(p, 0, nodeNu); 
            //flag0 = 1;
            flag = 1;
          }

          if (Map[r] == Map[Rj] \
              && nodeNu == *(p->Mi->destNu + 1)) {
              //&& MuxDestMatch(nodeNu, p, 1)) 
            GenerateSymbolicMuxSel(p, 1, nodeNu); 
            //flag1 = 1;
            flag = 1;
          }
        }
      } // else
      p = p->next;
    } // while
    
    //------------------------------------------------------------
    // It is possible that a node's id is not equal to any mux's
    // destination number *(pi->Mi->destNu), and yet the mux's 
    // output is connected to the FU at the node due to elimination
    // of redundant MUX(s).
    //
    // We consider generation of mux selection signals for constants
    // only if the current node contains a constant input and the port 
    // number is 1. By comparing the recorded mux input constant 
    // values with the constant the current node contains, we determine
    // the mux input number and thus the mux select value.
    // 
    //------------------------------------------------------------

    if (flag == 0) {
      p = head;
      myprintf("not const flag0/1\n");

      while (p != NULL) {

        //r  = port_nu ? rj : ri;
        Ri = *(p->Mi->regNu); 
        Rj = *(p->Mi->regNu + 1);
        

        if (Map[r] == Map[Ri]) {
          GenerateSymbolicMuxSel(p, 0, nodeNu); 
        }

        if (Map[r] == Map[Rj]) {
          GenerateSymbolicMuxSel(p, 1, nodeNu); 
        }
        p = p -> next;
      }
    }
    //------------------------------------------
    //
    //------------------------------------------
  }
}

//----------------------------------------------------------
// Phase2 symbolic Mux sel
//----------------------------------------------------------
/* Based on the shared register */
void GenerateSymbolicMuxSel2(int i, int *Map, int *MuxCntTable) {
  ResourceList p, head; 
  int op_type, resourceNu, sharedRegNu;
  int muxInputNu;
  int muxNu;
  int selCnt = 0;
  int startTime;

  // Access FuRAT
  if (Map[i] < 0) return;

  sharedRegNu = Map[i];
  p = head = FuRAT[sharedRegNu];

  op_type = DFG[i]->op;
  resourceNu = DFG[i]->opResourceNu;

  // A count member may be added to FuRAT to eliminate list traversal
  while (p != NULL) {
    //===================================================================
    //+ 1/20/11 
    // If node operation matches:
    // if it is a port node 
    //    check if a logical port matches a physical port by comparing
    //    the port numbers
    // if it is not a port node 
    //    check if a FU node matches a physical resource number
    //===================================================================
    if (p->op == op_type && ((op_type == nop && (GetInputPortNu(p->nu) == 
        GetInputPortNu(resourceNu))) || (p->nu == resourceNu))) {

    //- 1/20/11
    //if (p->op == op_type && p->nu == resourceNu) {

      // Find the MUX input number based on node i's op and resource number
      muxInputNu = selCnt;
    } 
    selCnt++;
    p = p->next;
  }

  if (selCnt >= 2) {
    muxNu = MuxCntTable[sharedRegNu];

    //----------------------------------------------------------
    // 7/9/11
    // Consider register insertion in front of any FUs,
    // startTime = ?
    //----------------------------------------------------------
    startTime = DFG[i]->opResultDoneSlot;

    myprintf("Phase2: DFG[%d] mux%d sel = %d at time slot %d\n",
            i, muxNu, muxInputNu, startTime);

    MuxSelListInsert(muxNu - 1, startTime, muxInputNu, selCnt);
  }
}

/* Use VarTable from register allocation implementation */
void GenerateSymbolicRegisterWen(FILE *vfp, int SharedRegNu) {
  int i;
  varList p, head;
  int wenTime;
  
  fprintf(vfp, "// --------------Register Wen --------------\n");
  for (i = 0; i < SharedRegNu; i++) {
    head = VarTable[i];
    if ((head->endTime - head->startTime) > minII && head->endTime != 10000) {
      fprintf(vfp, "RegWriteCtl #(%d, %d) r%d_ctl ", IntLog2(minII), minII, i+1);
      fprintf(vfp, "(clk, rst, stall, state[%d], r%d_fen);\n", head->startTime, i+1); 
    }
  }
  fprintf(vfp, "\n");


  for (i = 0; i < SharedRegNu; i++) {

    //fprintf(vfp, "assign r%d_wen = ", i + 1);
    fprintf(vfp, "assign r%d_wen = (", i + 1);
    p = head = VarTable[i];

    while (p != NULL) {

      wenTime = p->startTime;

      myprintf("load r%d at time slot %d\n", i + 1, wenTime);

      if (p == head)
        fprintf(vfp, "state[%2d]", wenTime);
      else
        fprintf(vfp, " | state[%2d]",wenTime);

      p = p->next;
    }

    if ((head->endTime - head->startTime) > minII && head->endTime != 10000)
      fprintf(vfp, " | r%d_fen", i+1);

    fprintf(vfp, ") & ~stall;\n");
  }
}

/* Another version using Node 
void CollectRegWen2(FILE *vfp, int *Map, int SharedRegNu) {
  int i;
  int wenTime;
  fprintf(vfp, "\n--------------Register Wen --------------\n");
  // Including the output result register
  for (i = 0; i < NODE_NU; i++) {
    wenTime = DFG[i]->opResultDoneSlot;
    fprintf(vfp, "load r%d at time slot %d\n", Map[i], wenTime);
  }
}*/

FSM_outputs_ptr *GenerateMuxSel1(FILE *vfp, int Phase1MuxCnt, int *MuxCnt) {
  MuxSelList p, head;
  int muxNu;
  int startTime;
  char idx;
  int m, last_m; // m is multiple of minII
  int sum;
  int cntFlag;
  const int MAX_MULTIPLE = 1000;
  
  //FSM_outputs_ptr *MuxSel1CtlTable = CreateCntEnableTable(Phase1MuxCnt);

  // A MUX control table for MuxSel1 and MuxSel2
  FSM_outputs_ptr *MuxSelCtlTable = CreateCntEnableTable(*MuxCnt);

  fprintf(vfp, "//--------------Mux Sel 1 --------------\n");
  for (muxNu = 0; muxNu < Phase1MuxCnt; muxNu++) {

    p = head = MuxSelTable[muxNu];

    if (p != NULL) {

      cntFlag = 0;
      sum = 0;
      last_m = MAX_MULTIPLE;
      idx = 'a';
      //fprintf(vfp, "counter ");

      while (p != NULL) {
        // The start time of all the variables for each shared register 
        // is in increasing order.
        startTime = p->startTime;

        // Debug 
        printf("Mux %d at time slot %d\n", muxNu + 1, startTime);

        m = startTime / minII;

        if (m > last_m) {

          cntFlag = 1; 

          GenerateCounter(vfp, sum, "mux", muxNu + 1, idx, cntFlag);
          idx++;

          //fprintf(vfp, "counter ");

          //MuxSel1CtlTable = UpdateCntEnableTable(MuxSel1CtlTable, muxNu, m);
          MuxSelCtlTable = UpdateCntEnableTable(MuxSelCtlTable, muxNu, last_m, minII);

          sum = 0;
        }

        // Control pattern in decimal 
        sum += (1 << (minII - 1 - startTime % minII));
        last_m = m;

        p = p->next;
      }

      //MuxSel1CtlTable = UpdateCntEnableTable(MuxSel1CtlTable, muxNu, m);
      MuxSelCtlTable = UpdateCntEnableTable(MuxSelCtlTable, muxNu, last_m, minII);

      GenerateCounter(vfp, sum, "mux", muxNu + 1, idx, cntFlag);
    }
  }
  //return MuxSel1CtlTable;
  return MuxSelCtlTable;
}


FSM_outputs_ptr *GenerateMuxSel2(FILE *vfp, FSM_outputs_ptr *MuxSelCtlTable, 
                                 int Phase1MuxCnt, int *MuxCnt, 
                                 char *ScheduleName) {
  MuxSelList p, head, end;
  int muxNu;
  int startTime, last_startTime;
  char idx;
  int m, last_m; // m is multiple of minII
  int sum;
  int cntFlag;
  int selVal, inputNu;
  const int MAX_MULTIPLE = 1000;
  FILE *vfp1 = NULL;
  
  //FSM_outputs_ptr *MuxSel2CtlTable = CreateCntEnableTable(*MuxCnt - Phase1MuxCnt);

  // Modelsim do file
  char fileName[100];
  char *fileNamePtr;


  fprintf(vfp, "//--------------Mux Sel 2--------------\n");

  for (muxNu = Phase1MuxCnt; muxNu < *MuxCnt; muxNu++) {

    p = head = MuxSelTable[muxNu];

    if (p != NULL) {

      cntFlag = 0;
      sum = 0;
      last_m = MAX_MULTIPLE;
      idx = 'a';
      inputNu  = p->inputNu;

      fileNamePtr = GetFileName(fileName, "_ROM", ".v", ScheduleName);

      if (vfp1 == NULL)
        vfp1 = fopen(fileNamePtr , "w");
      else
        vfp1 = fopen(fileNamePtr , "a");

      while (p != NULL) {
        // The start time of all the variables for each shared register 
        // is in increasing order.
        startTime = p->startTime;
        selVal    = p->selVal;

        m = startTime / minII;

        if (m > last_m) {

          cntFlag = 1; 

          end = p;
          GenerateRom(vfp1, "mux", muxNu + 1, idx, inputNu, cntFlag, head, end);

          head = p;

          GenerateCounter(vfp, 0, "muxM", muxNu + 1, idx, cntFlag);
          GenerateRomInstance(vfp , muxNu + 1, idx, inputNu, cntFlag);

          idx++;

          printf("1 Debug Mux %d sel = %d multiple = %d at time slot %d\n", 
              muxNu + 1, selVal, last_m, last_startTime);
          //MuxSel2CtlTable = UpdateCntEnableTable(MuxSel2CtlTable, muxNu, m);
          MuxSelCtlTable = UpdateCntEnableTable(MuxSelCtlTable, muxNu, last_m, minII);
        }

        last_m = m;
        last_startTime = startTime;
        p = p->next;
      }

      end = p;
      //MuxSel2CtlTable = UpdateCntEnableTable(MuxSel2CtlTable, muxNu, m);
      printf("2 Debug Mux %d sel = %d multiple = %d at time slot %d\n", 
              muxNu + 1, selVal, last_m, last_startTime);
      MuxSelCtlTable = UpdateCntEnableTable(MuxSelCtlTable, muxNu, last_m, minII);
      GenerateRom(vfp1, "mux", muxNu + 1, idx, inputNu, cntFlag, head, end);
      GenerateCounter(vfp, 0, "muxM", muxNu + 1, idx, cntFlag);
      GenerateRomInstance(vfp , muxNu + 1, idx, inputNu, cntFlag);

      fclose(vfp1); 
    }
  }

  //return MuxSel2CtlTable;
  return MuxSelCtlTable;
}

FSM_outputs_ptr *CollectRegFileRen(FILE *vfp, int SharedRegNu) {
  varList p, head;
  int regNu;
  int destNu;
  int startTime, endTime, endTime_tmp;
  char idx;
  int m, last_m; // m is multiple of minII
  int sum;
  int cntFlag;
  int i;
  const int MAX_MULTIPLE = 1000;
  
  FSM_outputs_ptr *RegRenCtlTable = CreateCntEnableTable(SharedRegNu);

  fprintf(stdout, "\n--------------Register File Ren --------------\n");
  for (regNu = 0; regNu < SharedRegNu; regNu++) {

    p = head = VarTable[regNu];

    if (p != NULL) {

      cntFlag = 0;
      sum = 0;
      last_m = MAX_MULTIPLE;
      idx = 'a';

      while (p != NULL) {
        // The start time of all the variables for each shared register 
        // is in increasing order.
        startTime = p->startTime;
        endTime = p->endTime;

        // even if it is a multi-destination node
        if (endTime - startTime <= minII || endTime == 10000) break;

        // Debug 
        fprintf(stdout, "read r(RF)%d at time slot %d\n", regNu + 1, endTime);

        destNu = DFG[p->varID]->opDestNu;
        for (i = 0; i < destNu; i++) {
          endTime = DFG[DFG[p->varID]->opDest[i]]->opScheduledSlot;
          RegRenCtlTable = UpdateCntEnableTable(RegRenCtlTable, regNu, endTime, 1);
        }
        p = p->next;
      }
    }
  }
  return RegRenCtlTable;
}



FSM_outputs_ptr *CollectRegWen(FILE *vfp, int SharedRegNu) {
  varList p, head;
  int regNu;
  int startTime;
  char idx;
  int m, last_m; // m is multiple of minII
  int sum;
  int cntFlag;
  const int MAX_MULTIPLE = 1000;
  
  FSM_outputs_ptr *RegWenCtlTable = CreateCntEnableTable(SharedRegNu);

  fprintf(stdout, "\n--------------Register Wen --------------\n");
  for (regNu = 0; regNu < SharedRegNu; regNu++) {

    p = head = VarTable[regNu];

    if (p != NULL) {

      cntFlag = 0;
      sum = 0;
      last_m = MAX_MULTIPLE;
      idx = 'a';

      while (p != NULL) {
        // The start time of all the variables for each shared register 
        // is in increasing order.
        startTime = p->startTime;

        // Debug 
        fprintf(stdout, "load r%d at time slot %d\n", regNu + 1, startTime);

        m = startTime / minII;

        if (m > last_m) {

          cntFlag = 1; 

          GenerateCounter(vfp, sum, "reg", regNu + 1, idx, cntFlag);
          idx++;

          RegWenCtlTable = UpdateCntEnableTable(RegWenCtlTable, regNu, last_m, minII);

          sum = 0;
        }

        // Control pattern in decimal 
        sum += (1 << (minII - 1 - startTime % minII));
        last_m = m;

        p = p->next;
      }

      RegWenCtlTable = UpdateCntEnableTable(RegWenCtlTable, regNu, last_m, minII);

      GenerateCounter(vfp, sum, "reg", regNu + 1, idx, cntFlag);
    }
  }
  return RegWenCtlTable;
}

void GenerateControlWires(FILE *vfp, 
                          int *MuxCnt, 
                          int SharedRegNu) {

  int i, j;
  MuxSelList p, head;

  //fprintf(vfp, "//--------------Mux Select --------------\n");

  // Version 1   Use assign statements
  /*
  for (i = 0; i < *MuxCnt; i++) {

    fprintf(vfp, "assign s%-2d = ", i + 1);
    p = head = MuxSelTable[i];

    while (p != NULL) {

      assert(p->inputNu >= 2);

      if (p->inputNu == 2) { 
        if (p == head)
          fprintf(vfp, "state[%2d]", p->startTime);
        else
          fprintf(vfp, " | state[%2d]", p->startTime);
      }
      else if (p->inputNu > 2) { // N-1 Mux (N > 2)
        fprintf(vfp, "state[%2d] ? %d'd%d : ", p->startTime, 
                IntLog2(p->inputNu), p->selVal);
        if (p->next == NULL)
          fprintf(vfp, "%d'd0", IntLog2(p->inputNu));
      }
      p = p->next;
    }
    fprintf(vfp, ";\n");
  }
  */

  // Version 2   Use always statements
  int size, first_cond, first_if;

  for (i = 0; i < *MuxCnt; i++) {

    fprintf(vfp, "always @ (*) begin\n");

    p = head = MuxSelTable[i];

    // We can use the first MUX to determine the number of MUX inputs
    // for each of the MUXes in the list
    if (p->inputNu == 2) { 
      fprintf(vfp, " s%-3d = ", i + 1);
      while (p != NULL) {
        if (p->inputNu == 2) { 
          if (p == head)
            fprintf(vfp, "state[%2d]", p->startTime);
          else
            fprintf(vfp, " | state[%2d]", p->startTime);
        }
        p = p->next;
      }
      fprintf(vfp, ";\n");
      fprintf(vfp, "end\n"); // always-end pair
    }

    else if (p->inputNu > 2) { 
      size = IntLog2(p->inputNu);
      first_if = 1;
      for (j = 1; j < p->inputNu; j++) { // 
        fprintf(vfp, "%s",  first_if ? "  if (" : "  else if (");
        first_cond = 1;
        while (p != NULL) { 
          if (p->selVal == j) {
            if (first_cond) {
              fprintf(vfp, " state[%2d] ", p->startTime);
              first_cond = 0;
            }
            else {
              fprintf(vfp, "|| state[%2d] ", p->startTime);
            }
          }
          p = p->next;
        } // while
        fprintf(vfp, ")\n");
        fprintf(vfp, "    s%-3d = %d'd%d;\n", i+1, size, j);
        first_if = 0;
        p = head;    // restart list read from the head
      } // for
      fprintf(vfp, "  else\n");
      fprintf(vfp, "    s%-3d = %d'd0;\n", i+1, size);
      fprintf(vfp, "end\n"); // always-end pair
    }

    else {
      assert (p->inputNu >= 2);
    }
  }



  //fprintf(vfp, "//--------------Register Wen --------------\n");

  GenerateSymbolicRegisterWen(vfp, SharedRegNu);

  fprintf(vfp, "\n");

}

/*
void GenerateControlWires(FILE *vfp, FSM_outputs_ptr *MuxSelCtlTable, 
    FSM_outputs_ptr *RegWenCtlTable, int *MuxCnt, int SharedRegNu) {

  int i, j;
  char idx;
  int muxInputNu;
  int cntNu;
  ResourceList fu, head;

  // Generate assign statements for Mux select 
  for (i = 0; i < *MuxCnt; i++) {
    fprintf(vfp, "assign s%-2d = ", i + 1);
    cntNu = MuxSelCtlTable[i]->cntNu;
    if ( cntNu > 1) {
      idx = 'a';
      for (j = 0; j < cntNu; j++) {
        fprintf(vfp, "m%d%c_sel", i + 1, idx++);
        if (j != cntNu - 1) fprintf(vfp, " | ");
      }
      fprintf(vfp, ";\n");
    }
    else
      fprintf(vfp, "m%d_sel;\n", i + 1);
  }

  fprintf(vfp, "\n");

  // Generate assign statements for register wen
  for (i = 0; i < SharedRegNu; i++) {
    fprintf(vfp, "assign r%d_wen = ", i + 1);

    fu = head = FuRAT[i + 1];
    muxInputNu = 0;

    while (fu != NULL) {
      fu = fu->next;
      muxInputNu++;
    }

    cntNu = RegWenCtlTable[i]->cntNu;
    if (cntNu > 1) {
      idx = 'a';
      for (j = 0; j < cntNu; j++) {
        fprintf(vfp, "r%d%c_wen_tmp", i + 1, idx++);
        if (j != cntNu - 1) fprintf(vfp, " | ");
      }
      fprintf(vfp, ";\n");
      //fprintf(vfp, ");\n");
    }
    else
      //fprintf(vfp, "r%d_wen_tmp);\n", i + 1);
      fprintf(vfp, "r%d_wen_tmp;\n", i + 1);
  }

  fprintf(vfp, "\n");

  // Generate assign statements for counter stall 
  GenerateCounterStall(vfp, *MuxCnt, MuxSelCtlTable, "mux");
  GenerateCounterStall(vfp, SharedRegNu, RegWenCtlTable, "reg");
} */

// Use Node 
void GenerateSymbolicFUGo(FILE *vfp) {
  int i;
  int goTime;
  int op, resourceNu;
  fprintf(vfp, "\n-------------- FU go --------------\n");
  for (i = 0; i < NODE_NU; i++) {
    goTime = DFG[i]->opScheduledSlot;
    op = DFG[i]->op;
    resourceNu = DFG[i]->opResourceNu;
    fprintf(vfp, "start %s%d at time slot %d\n", 
        opSymTable(op), resourceNu, goTime);
  }
}

void GenerateFUGo(FILE *vfp) {
  int op_type;
  int offset, fu_nu, size;

  //fprintf(vfp, "\n-------------- Wire FU go --------------\n");
  for (op_type = add; op_type < add + OP_NU; op_type++) {

    /* We cannot let input-port FU's go always set high
    for (fu_nu = 0; fu_nu < offset; fu_nu++)
      fprintf(vfp, "assign %s%d_go = 1'b1;\n", 
                   opSymTable(op_type), fu_nu + 1);
     */

    offset = 0; //(PQ[op_type] == NULL) ? 0 : (*PQ[op_type])->size;
    size = (*RQ[op_type])->size;

    for (fu_nu = 0; fu_nu < size; fu_nu++) {
      fprintf(vfp, "wire %s%d_go0;\n", opSymTable(op_type), fu_nu + 1 + offset);
      fprintf(vfp, "wire %s%d_go1;\n", opSymTable(op_type), fu_nu + 1 + offset);
    }

    for (fu_nu = 0; fu_nu < size; fu_nu++) {
      fprintf(vfp, "assign %s%d_go = %s%d_go0 & %s%d_go1;\n", 
      opSymTable(op_type), fu_nu + 1 + offset, 
      opSymTable(op_type), fu_nu + 1 + offset, 
      opSymTable(op_type), fu_nu + 1 + offset);
    } 
    fprintf(vfp, "\n");
  }

  fprintf(vfp, "\n");
}

/*
 	Phase 1: Control path Mux from shared registers' ready to FU go
 */
void GeneratePhase1CMux(FILE *vfp, int *Map, int *MuxCnt) {

  enum operation op_type;
  int fu_nu, port_nu;
  MuxPtr Mux;
  int i;
  int size, offset;
  int ml, p;

  for (op_type = add; op_type < add + OP_NU; op_type++) {
    size = (*RQ[op_type])->size;
    for (fu_nu = 0; fu_nu < size; fu_nu++) {
      for (port_nu = 0; port_nu < 2; port_nu++) {

         Mux = (MuxRAT[op_type][fu_nu]).Fu_portNu[port_nu];

         while (Mux != NULL) {
           (*MuxCnt)++;
           ml = 0;
           fprintf(vfp, "MUX2 #(1) cmux%d (\n", Mux->m);
           // Same as mux sel
           fprintf(vfp, "  .S(s%d),\n", Mux->m);

           for (i = 0; i < 2; i++) 
             if (*(Mux->Mi->constNu + i) != 0 &&
                 *(Mux->Mi->muxNu + i) == -1)
               // Always enable FU port for constant input
               fprintf(vfp, "  .I%d(1'b1),\n", i + 1);

           for (i = 0; i < 2; i++) 
             if (*(Mux->Mi->constNu + i) == 0 &&
                 *(Mux->Mi->muxNu + i) == -1) {
               // Shared register data ready 
               fprintf(vfp, "  .I%d(r%d_rdy),\n", 
                       i + 1, Map[*(Mux->Mi->regNu + i)]);
             }

           for (i = 0; i < 2; i++) 
             // Multi-level Muxes
             if (*(Mux->Mi->muxNu + i) != -1) {
               fprintf(vfp, "  .I%d(cmux%d_in%d),\n", 
                       i + 1, Mux->m, i);
               ml = 1;
               p = i;
             }

           // Mux whose input is connected to FU will be generated in Phase 2

           if (*(Mux->Mo->muxNu) != -1) 
             fprintf(vfp, "  .Z(cmux%d_out)\n);\n\n", *(Mux->Mo->muxNu));

           if (*(Mux->Mo->fuNu) != -1) {
             offset = 0; //(PQ[op_type] == NULL) ? 0 : (*PQ[op_type])->size;
             fprintf(vfp, "  .Z(%s%d_go%d)\n);\n\n", 
                     opSymTable(op_type), fu_nu + 1 + offset, port_nu);
           }

           if (ml)  // connect mux output the next level mux input
             fprintf(vfp, "assign cmux%d_in%d = cmux%d_out;\n\n", 
                     Mux->m, p, Mux->m);

           Mux = Mux->next;
         }
      }
    }
  }
}

/* 
  Phase 2: Control path Mux from FU's ready to shared registers wen

  wire s($muxNuCnt)[$(muxSelWidth - 1) : 0];
	MUX($inputNu) mux($muxNuCnt)(
	 .S(s($muxNuCnt)),
	 .I($inputNu)(($opSym)($nu)_out),
	 .Z(r($regNu)_en));
*/
void GeneratePhase2CMux(FILE *vfp, int SharedRegNu, int *MuxCnt, int *MuxCntTable) {
  int muxInputNu;
  int regNu;
  double fval;
  int ival;
  int muxSelWidth;
  ResourceList fu, head;

  for (regNu = 1; regNu <= SharedRegNu; regNu++) {
    muxInputNu = 0;
    fu = head = FuRAT[regNu];

    while (fu != NULL) {
      fu = fu->next;
      muxInputNu++;
    }

    if (muxInputNu >= 2) {

      (*MuxCnt)++;

      muxSelWidth = IntLog2(muxInputNu);
      MuxCntTable[regNu] = *MuxCnt;
      
      // Already declared in Phase2Mux
      //fprintf(vfp, "wire [%d : 0] s%d;\n",    muxSelWidth - 1, *MuxCnt);
      fprintf(vfp, "wire          r%d_en;\n", regNu);

      fprintf(vfp, "MUX%d #(1) cmux%d (\n", muxInputNu, *MuxCnt);
      fprintf(vfp, "  .S(s%d),\n", *MuxCnt);

      fu = head;
      muxInputNu = 0;
      while (fu != NULL) {
        muxInputNu++;
        fprintf(vfp, "  .I%d(%s%d_rdy),\n", 
                muxInputNu, opSymTable(fu->op), fu->nu);
        fu = fu->next;
      }
      
      fprintf(vfp, "  .Z(r%d_en)\n);\n\n", regNu);
    }
  }
}

//-----------------------------------------------------------------------------
// Wire each shared register's ready which is directly connected 
// to the FU's go 
//-----------------------------------------------------------------------------
void GeneratePhase1CWire(FILE *vfp, int *Map) {

  enum operation op_type;
  int fu_nu, port_nu;
  int i, j;
  int size, offset;
  int dest;
  MuxPtr Mux;

  for (op_type = add; op_type < add + OP_NU; op_type++) {
    size = (*RQ[op_type])->size; // Non-port nodes 
    for (fu_nu = 0; fu_nu < size; fu_nu++) {
      for (port_nu = 0; port_nu < 2; port_nu++) {

         Mux = (MuxRAT[op_type][fu_nu]).Fu_portNu[port_nu];

         if (Mux == NULL) {  // Indicate FU port is not connected

           // FU number offset
           offset = 0; //(PQ[op_type] == NULL) ? 0 : (*PQ[op_type])->size;

           // Find all the shared registers which are connected to the FU 
           for (i = 0; i < NODE_NU; i++) {
             // More than one destination 
             for (j = 0; j < DFG[i]->opDestNu; j++) {
               dest = DFG[i]->opDest[j];
               if (dest > 0) {
                 if (DFG[dest]->opSrc[port_nu] == i && 
                     DFG[dest]->opResourceNu == (fu_nu + 1 + offset) && 
                     DFG[dest]->op == op_type) {
                   if ((DFG[dest]->opConst && port_nu == 1))
                     // Always enable FU port for constant input
                     fprintf(vfp, "assign %s%d_go%d = 1;\n",   
                             opSymTable(op_type), fu_nu + 1 + offset, port_nu);
                   else {
                     fprintf(vfp, "assign %s%d_go%d = r%d_rdy;\n", 
                             opSymTable(op_type), fu_nu + 1 + offset, port_nu, Map[i]);

                     // Set register clear based on one-to-one correspondence 
                     fprintf(vfp, "assign r%d_clr = %s%d_go;\n", 
                             Map[i], opSymTable(op_type), fu_nu + 1 + offset);
                   }
                   break;
                 }
               }
             }
           }
         }
      }
    }
  }
}

//-----------------------------------------------------------------------------
// BUG: Ignore register write timing
// Wire FU's ready to each shared register's wen input 
// if the number of its sources is 1
//-----------------------------------------------------------------------------
void GeneratePhase2CWire(FILE *vfp, int SharedRegNu) {

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
    if (muxInputNu < 2)
      fprintf(vfp, "assign r%d_wen = %s%d_rdy;\n", 
              regNu, opSymTable(head->op), head->nu);
  }
  fprintf(vfp, "\n");
}

/*
 	Phase 1: Control path DeMux from FU go to shared registers' wen

  DEMUX #(1) DEMUX instance(
  .I(demux_in),
  .O1(demux_out1),
  .O2(demux_out2)
)
 */
void GeneratePhase1DeMux(FILE *vfp, int *Map) {

  enum operation op_type;
  int fu_nu, port_nu;
  MuxPtr Mux;
  int i;
  int size, offset;
  int ml, p;

  for (op_type = add; op_type < add + OP_NU; op_type++) {
    size = (*RQ[op_type])->size;
    for (fu_nu = 0; fu_nu < size; fu_nu++) {
      for (port_nu = 0; port_nu < 2; port_nu++) {

         Mux = (MuxRAT[op_type][fu_nu]).Fu_portNu[port_nu];

         while (Mux != NULL) {
           ml = 0;
           fprintf(vfp, "DEMUX2 #(1) demux%d (\n", Mux->m);

           for (i = 0; i < 2; i++) 
             if (*(Mux->Mi->constNu + i) != 0 &&
                 *(Mux->Mi->muxNu + i) == -1)
               // Connected to the ground.(Modelsim Error)
               // fprintf(vfp, "  .O%d(1'b0),\n", i + 1);
               fprintf(vfp, "  .O%d(),\n", i + 1);

           fprintf(vfp, "  .S(s%d),\n", Mux->m);

           for (i = 0; i < 2; i++) 
             if (*(Mux->Mi->constNu + i) == 0 &&
                 *(Mux->Mi->muxNu + i) == -1) {
               fprintf(vfp, "  .O%d(r%d_clr),\n", 
                       i + 1, Map[*(Mux->Mi->regNu + i)]);
             }

           for (i = 0; i < 2; i++) 
             // Multi-level Muxes
             if (*(Mux->Mi->muxNu + i) != -1) {
               fprintf(vfp, "  .O%d(demux%d_out%d),\n", 
                       i + 1, Mux->m, i);
               ml = 1;
               p = i;
             }


           // Mux whose input is connected to FU will be generated in Phase 2

           if (*(Mux->Mo->muxNu) != -1) 
             fprintf(vfp, "  .I(demux%d_in)\n);\n\n", *(Mux->Mo->muxNu));

           if (*(Mux->Mo->fuNu) != -1) {
             offset = 0; //(PQ[op_type] == NULL) ? 0 : (*PQ[op_type])->size;
             fprintf(vfp, "  .I(%s%d_go)\n);\n\n", 
                     opSymTable(op_type), fu_nu + 1 + offset);
           }

           if (ml)  // connect 
             fprintf(vfp, "assign demux%d_in = demux%d_out%d;\n\n", 
                     Mux->m, Mux->m, p);

           Mux = Mux->next;
         }
      }
    }
  }
}

void GenerateControlFSM(FILE *vfp) {

  FILE *fp;
  char ch;

  // File copy 
  fp = fopen("vlib/FSM_TimeState.v", "r");
  while((ch = getc(fp)) != EOF) putc(ch, vfp);
  fclose(fp);

    fprintf(vfp, "reg [%0d:0] state_cnt;\n", IntLog2(minII)-1);
    fprintf(vfp, "reg       state_ctl_din;\n");

  // DII interval (Depth is DII) (dio declared in copied file)
  //
  // The shift content is 1 as we use state_in[0]
  // Or the shift content is 1 << (minII - 1) if we use dio 
  if (minII > 1) {
    /*
    //fprintf(vfp, "shift #(%d, 1) shift_i (clk, state_wen, dio, dio, state_in);\n\n", minII); // 5/10/12
    fprintf(vfp, "shiftCtl #(%d, 1) shift_i (clk, rst, state_wen, dio, dio, state_in);\n\n", minII);

    fprintf(vfp, "reg [31:0] test_cnt;\n");
    fprintf(vfp, "reg       hist;\n"); // the previous value of state_ctl_din
    fprintf(vfp, "\n");
    fprintf(vfp, "always @ (posedge clk) begin\n");
    fprintf(vfp, "  if (rst) begin\n");
    fprintf(vfp, "    test_cnt <= 0;\n");
    fprintf(vfp, "    hist <= 1;\n");
    fprintf(vfp, "  end\n");
    fprintf(vfp, "  else if (!state_ctl_din && hist) begin\n");
    fprintf(vfp, "    test_cnt <= test_cnt + 1'b1;\n");
    fprintf(vfp, "    hist <= state_ctl_din;\n");
    fprintf(vfp, "  end\n");
    fprintf(vfp, "  else if (state_ctl_din != hist) begin\n");
    fprintf(vfp, "    hist <= state_ctl_din;\n");
    fprintf(vfp, "    test_cnt <= test_cnt;\n");
    fprintf(vfp, "  end\n");
    fprintf(vfp, "end\n");
    */
    fprintf(vfp, "assign state_din = go_in & state_ctl_din; //state_in[0];\n");
    fprintf(vfp, "\n");
    fprintf(vfp, "always @ (posedge clk) begin\n");
    fprintf(vfp, "  if (rst) begin\n");
    fprintf(vfp, "    state_cnt <= 0;\n");
    fprintf(vfp, "    state_ctl_din <= 1'b1;\n");
    fprintf(vfp, "  end\n");
    //fprintf(vfp, "  else if (test_cnt < TEST_NU && state_cnt == %d && state_wen) begin\n", minII-1);
    fprintf(vfp, "  else if (go) begin\n");
    fprintf(vfp, "    if (state_cnt == %d'd%0d) begin\n", IntLog2(minII), minII-1);
    fprintf(vfp, "      state_cnt <= 0;\n");
    fprintf(vfp, "      state_ctl_din <= 1'b1;\n");
    fprintf(vfp, "    end\n");
    fprintf(vfp, "    else begin\n");
    fprintf(vfp, "      state_cnt <= state_cnt + 1'b1;\n");
    fprintf(vfp, "      state_ctl_din <= 1'b0;\n");
    fprintf(vfp, "    end\n");
    fprintf(vfp, "  end\n");
    fprintf(vfp, "end\n");
  }
  else if (minII == 1) {
    fprintf(vfp, "assign state_din = state_wen;\n\n");
  }


}


FSM_outputs_ptr *CreateCntMarkTable(int compNu, FSM_outputs_ptr *MuxSelCtlTable) {

  int i, j;
  int cntNu;

  FSM_outputs_ptr *MuxSelCtlMarkTable = CreateCntEnableTable(compNu);
  for (i = 0; i < compNu; i++) {
    cntNu = MuxSelCtlTable[i]->cntNu;
    for (j = 0; j < cntNu; j++)
      MuxSelCtlMarkTable = UpdateCntEnableTable(MuxSelCtlMarkTable, i, 0, 0);
  }
  return MuxSelCtlMarkTable; 
}

void FindSameEnableTime(FILE *vfp, int nu, int compNu, char *instanceName,
                        FSM_outputs_ptr *MuxSelCtlTable, 
                        FSM_outputs_ptr *MuxSelCtlMarkTable, 
                        int *Map, int time) {
  int i, j, k;
  int cntNu;

  for (i = nu; i < compNu; i++) {
    cntNu = MuxSelCtlTable[i]->cntNu;
    for (j = 0; j < cntNu; j++) {
      myprintf("FindSame: %sCtlTable[%d]->enableTime[%d] = %d\n",
          instanceName, i, j, MuxSelCtlTable[i]->enableTime[j]);

      if (MuxSelCtlTable[i]->enableTime[j] == time) {

        //------------------------------------------------------------
        // Separate RF read 12/26
        // begins
        if (instanceName[0] == 'R') {
          GenerateRegFileRen(vfp, Map, i + 1, time);

        // ends
        //------------------------------------------------------------
        } else {
          if (cntNu > 1)
            fprintf(vfp, "%6s%c%d%c_enable = 1'b1;\n", 
                " ", instanceName[0], i + 1, j + 'a');
          else
            fprintf(vfp, "%6s%c%d_enable = 1'b1;\n", 
                " ", instanceName[0], i + 1);
        }
        MuxSelCtlMarkTable[i]->enableTime[j] = 1;
      }
    }
  }
}

/* */
void GenerateEnableDefault(FILE *vfp, char *instanceName, 
                           FSM_outputs_ptr *MuxSelCtlTable, int compNu) {
  int i, j; 
  int cntNu;

  for (i = 0; i < compNu; i++) {
    cntNu = MuxSelCtlTable[i]->cntNu;
    for (j = 0; j < cntNu; j++) {
      if (cntNu > 1)
        fprintf(vfp, "%2s%c%d%c_enable = 1'b0;\n", 
            " ", instanceName[0], i + 1, j + 'a');
      else
        fprintf(vfp, "%2s%c%d_enable = 1'b0;\n", 
            " ", instanceName[0], i + 1);
    }
  }
}
  
// Generate FSM outputs 
void GenerateControlEnable(FILE *vfp, 
                           FSM_outputs_ptr *MuxSelCtlTable, 
                           FSM_outputs_ptr *RegWenCtlTable, 
                           FSM_outputs_ptr *RegRenCtlTable, 
                           int *MuxCnt, int SharedRegNu,
                           int *Map) {

  int i, j;
  int cntNu;
  int time;

  // Case start
  //fprintf(vfp, "always @ (TimeState)\n");
  fprintf(vfp, "always @ (*)\n");
  fprintf(vfp, "begin\n");

  // Default values set to 0
  GenerateEnableDefault(vfp, "mux", MuxSelCtlTable, *MuxCnt);
  GenerateEnableDefault(vfp, "reg", RegWenCtlTable, SharedRegNu);
  //GenerateEnableDefault(vfp, "Reg", RegRenCtlTable, Map, SharedRegNu);
  GenerateDefaultRegFileRen(vfp, Map);


  // modelsim reports error for (* parallel case *) 
  fprintf(vfp, "\n  if (go || run) begin\n");  // add
  fprintf(vfp, "\n  case (TimeState) // synthesis parallel_case\n");

  // Case state list
  FSM_outputs_ptr *MuxSelCtlMarkTable = CreateCntMarkTable(*MuxCnt, MuxSelCtlTable);
  FSM_outputs_ptr *RegWenCtlMarkTable = CreateCntMarkTable(SharedRegNu, RegWenCtlTable);
  FSM_outputs_ptr *RegRenCtlMarkTable = CreateCntMarkTable(SharedRegNu, RegRenCtlTable);
  
  for (i = 0; i < *MuxCnt; i++) {

    cntNu = MuxSelCtlTable[i]->cntNu;

    for (j = 0; j < cntNu; j++) {

      if (MuxSelCtlMarkTable[i]->enableTime[j]) continue;
      time = MuxSelCtlTable[i]->enableTime[j];
      //fprintf(vfp, "%4sS%d:\n", " ", time);
      fprintf(vfp, "%4s'd%d:\n", " ", time);
      fprintf(vfp, "%4sbegin\n", " ");

      if (cntNu > 1)
        fprintf(vfp, "%6sm%d%c_enable = 1'b1;\n", " ", i + 1, j + 'a');
      else
        fprintf(vfp, "%6sm%d_enable = 1'b1;\n", " ", i + 1);

      FindSameEnableTime(vfp, i + 1, *MuxCnt, "mux", MuxSelCtlTable, 
          MuxSelCtlMarkTable,  Map, time);
      FindSameEnableTime(vfp, 0, SharedRegNu, "reg", RegWenCtlTable,  
          RegWenCtlMarkTable, Map, time);
      FindSameEnableTime(vfp, 0, SharedRegNu, "Reg", RegRenCtlTable,  
          RegRenCtlMarkTable, Map, time);

      fprintf(vfp, "%4send\n\n", " ");
    }
  }

  for (i = 0; i < SharedRegNu; i++) {

    cntNu = RegWenCtlTable[i]->cntNu;

    for (j = 0; j < cntNu; j++) {

      if (RegWenCtlMarkTable[i]->enableTime[j]) continue;

      time = RegWenCtlTable[i]->enableTime[j];
      //fprintf(vfp, "%4sS%d:\n", " ", time);
      fprintf(vfp, "%4s'd%d:\n", " ", time);
      fprintf(vfp, "%4sbegin\n", " ");

      if (cntNu > 1)
        fprintf(vfp, "%6sr%d%c_enable = 1'b1;\n", " ", i + 1, j + 'a');
      else
        fprintf(vfp, "%6sr%d_enable = 1'b1;\n", " ", i + 1);

      FindSameEnableTime(vfp, i + 1, SharedRegNu, "reg", RegWenCtlTable,  RegWenCtlMarkTable, Map, time);
      //printf("time = %d FindSameEnableTime in RegWen, \n", time);

      // 12/24
      //FindSameEnableTime(vfp, i + 1, SharedRegNu, "Reg", RegRenCtlTable,  RegRenCtlMarkTable, time);
      FindSameEnableTime(vfp, 0, SharedRegNu, "Reg", RegRenCtlTable,  RegRenCtlMarkTable, Map, time);

      fprintf(vfp, "%4send\n\n", " ");
    }
  }

  for (i = 0; i < SharedRegNu; i++) {

    cntNu = RegRenCtlTable[i]->cntNu;
    //printf("register file cnt = %d for %d\n", cntNu, i);

    for (j = 0; j < cntNu; j++) {

      if (RegRenCtlMarkTable[i]->enableTime[j]) continue;

      time = RegRenCtlTable[i]->enableTime[j];
      //fprintf(vfp, "%4sS%d:\n", " ", time);
      fprintf(vfp, "%4s'd%d:\n", " ", time);
      fprintf(vfp, "%4sbegin\n", " ");

      printf("register file cnt = %d for %d\n", cntNu, i);

      /*
      if (cntNu > 1)
        fprintf(vfp, "%6sR%d%c_enable = 1'b1;\n", " ", i + 1, j + 'a');
      else
        fprintf(vfp, "%6sR%d_enable = 1'b1;\n", " ", i + 1);
       */
      // 12/27
      GenerateRegFileRen(vfp, Map, i + 1, time);

      FindSameEnableTime(vfp, i + 1, SharedRegNu, "Reg", RegRenCtlTable,  
                         RegRenCtlMarkTable, Map, time);

      fprintf(vfp, "%4send\n\n", " ");
    }
  }

  // Case end
  fprintf(vfp, "%4sdefault: ;\n", " ");
  //fprintf(vfp, "%4sbegin\n", " ");
  //GenerateEnableDefault(vfp, "mux", MuxSelCtlTable, *MuxCnt);
  //GenerateEnableDefault(vfp, "reg", RegWenCtlTable, SharedRegNu);
  //GenerateEnableDefault(vfp, "Reg", RegRenCtlTable, SharedRegNu);
  //fprintf(vfp, "%4send\n", " ");
  fprintf(vfp, "%2sendcase\n", " ");
  fprintf(vfp, "  end\n"); // if statement 
  fprintf(vfp, "end\n\n");

  FreeCntEnableTable (MuxSelCtlMarkTable, *MuxCnt);
  FreeCntEnableTable (RegWenCtlMarkTable, SharedRegNu);
  FreeCntEnableTable (RegRenCtlMarkTable, SharedRegNu);
}

void GenerateRegFileRen(FILE *vfp, int *Map, int regNu, int time) {
  RF_portNuPtr head;
  int DestNu;
  int RF_PortNu;
  int endTime;
  int idx;
  int i, k;

  // Note the node's ID value is not neccesarily the shared register
  // number minus 1 !
  for (i = 0; i < NODE_NU; i++) {
    if (Map[i] == regNu && DFG[i]->RegFileTable != NULL) {
      // search the end time that is equal to time
      RF_PortNu = 0;
      DestNu = DFG[i]->opDestNu;
      for (k = 0; k < DestNu; k++) {
        idx = 0;
        // search RegFileList
        head = (DFG[i]->RegFileTable[k]).RegFileList;
        if (head != NULL) RF_PortNu++;
        while (head != NULL) {
          endTime = DFG[*(head->sharedDestNuPtr)]->opScheduledSlot;
          if (time == endTime) {
            if ((DFG[i]->RegFileTable[k]).sharedCnt == 1)
              fprintf(vfp, "%6sR%d_enable_%d = 1'b1;\n", 
                           " ", Map[i], RF_PortNu);
            else
              fprintf(vfp, "%6sR%d_enable_%d%c = 1'b1;\n", 
                           " ", Map[i], RF_PortNu, idx + 'a');
          }
          idx++; 
          head = head->next;
        } // while
      } // for  
    }
  }
}

void GenerateDefaultRegFileRen(FILE *vfp, int *Map) {
  RF_portNuPtr head;
  int DestNu;
  int RF_PortNu;
  int idx;
  int i, k;

  // Note the node's ID value is not neccesarily the shared register
  // number minus 1 !
  for (i = 0; i < NODE_NU; i++) {
    if (DFG[i]->RegFileTable != NULL) {
      RF_PortNu = 0;
      DestNu = DFG[i]->opDestNu;

      for (k = 0; k < DestNu; k++) {
        // search RegFileList
        idx = 0;
        head = (DFG[i]->RegFileTable[k]).RegFileList;
        if (head != NULL) RF_PortNu++;
        while (head != NULL) {
          if ((DFG[i]->RegFileTable[k]).sharedCnt == 1)
            fprintf(vfp, "%2sR%d_enable_%d = 1'b0;\n", " ", Map[i], RF_PortNu);
          else {
            fprintf(vfp, "%2sR%d_enable_%d%c = 1'b0;\n", 
                         " ", Map[i], RF_PortNu, idx + 'a');
            idx++;
          }
          head = head->next;
        } // while
      } // for  
    }
  }
}
