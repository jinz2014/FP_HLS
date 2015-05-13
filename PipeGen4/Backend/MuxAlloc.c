#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "Schedule.h"


//-------------------------------------------------------------------------
// Interconnection allocation main function 
//-------------------------------------------------------------------------
int InterconnectionAlloc (int *Map, int SharedRegNu) {

  int i, j, k;
  int ri, rj;
  int dest0, dest1;
  int flag0, flag1;
  int offset;
  int fu_nu, port_nu;
  enum operation op_type;
  MuxPtr nextMux;
  MuxPtr Mux;
  int MuxCnt = 0;
  int MuxInputNu;
  int p0, p1, p;

  CreateMuxRAT();

  //---------------------------------------------------------
  //
  //---------------------------------------------------------
  CreateFuRAT(Map, SharedRegNu);
  
  //---------------------------------------------------------
  // Find the register pair that is connected to the same FU 
  //
  // Map[ri] != Map[rj] && Map[ri] && Map[rj]
  //
  // case 1: the register pair are mapped to different shared registers. 
  // case 2: the register pair are mapped to the same shared register. 
  //---------------------------------------------------------

  // ri and rj represent operation output variables
  for (ri = 0; ri < NODE_NU; ri++) {
    for (rj = ri + 1; rj < NODE_NU; rj++) {

      for (i = 0; i < DFG[ri]->opDestNu; i++) {
        // More than one destination 
        dest0 = DFG[ri]->opDest[i];
        for (j = 0; j < DFG[rj]->opDestNu; j++) {
          dest1 = DFG[rj]->opDest[j];

          //---------------------------------------------------------
          // The register pair is connected to the same FU and 
          // they are mapped to two different shared registers
          // Note ri != rj
          //---------------------------------------------------------
          if (dest0 > 0 && dest1 > 0 &&
              Map[ri] != Map[rj] &&
              Map[ri] && Map[rj] &&
              DFG[dest0]->opResourceNu == DFG[dest1]->opResourceNu &&
              DFG[dest0]->op == DFG[dest1]->op) { // same floating precision implied

          //---------------------------------------------------------
          // Check if the register pair has a common FU port after
          // they are mapped to the same FU.
          //---------------------------------------------------------
            p0 = GetNodeSrcNu(DFG[dest0]->op);
            p1 = GetNodeSrcNu(DFG[dest1]->op);
            assert(p0 == p1);
            p = p0;
            for (port_nu = 0; port_nu < p; port_nu++) {

              if (DFG[dest0]->opSrc[port_nu] == ri && 
                  DFG[dest1]->opSrc[port_nu] == rj) {

                // operation type
                op_type = DFG[dest0]->op;

                // FU number
                offset = 0;
                fu_nu = DFG[dest0]->opResourceNu - offset - 1;

                //-----------------------------------------------------------------------
                // 6/23/11
                // 
                // MUX input constant optimizations
                //
                // 1. Allocate no mux if two inputs have the same constant
                // 2. Check if there are redundant Mux input constants even if the
                //    the constant values are different. Because the same MUX may have
                //    already been allocated. We assume that mux data field
                //    (Mux->Mi->constNu) contains nonzero value(s) if the mux
                //    input is a constant. Otherwise it is 0.
                //
                // 8/1/11
                // 
                // 3. FMUX32 or FMUX64 input constants check are ignored.
                //-----------------------------------------------------------------------
                if (port_nu == 1 && DFG[dest0]->opConst && DFG[dest1]->opConst) {

                  if (DFG[dest0]->opConstVal == DFG[dest1]->opConstVal) {

                    myprintf("P1: node %d and %d have the same constant values %f\n", \
                    dest0, dest1, DFG[dest0]->opConstVal);
                    continue;
                  }
                  else {
                    flag0 = flag1 = 0;
                    myprintf("P1: For type = %s fu_nu = %d ",opSymTable(op_type), fu_nu);
                    myprintf("check any redundant mux input constant values\n");
                    Mux = (MuxRAT[op_type][fu_nu]).Fu_portNu[port_nu];
                    while (Mux != NULL) {
                      for (k = 0; k < 2; k++) { 
                        //-------------------------------------------------------------
                        //-------------------------------------------------------------
                        if (DFG[dest0]->opPrecision == sfp) {
                          if (DFG[dest0]->opConstVal == *(Mux->Mi->constNu + k))
                            flag0 = 1;
                          if (DFG[dest1]->opConstVal == *(Mux->Mi->constNu + k))
                            flag1 = 1;
                        }
                        //-------------------------------------------------------------
                        //-------------------------------------------------------------
                        if (DFG[dest0]->opPrecision == dfp) {
                          if (DFG[dest0]->opConstVal_d == *(Mux->Mi->constNu_d + k))
                            flag0 = 1;
                          if (DFG[dest1]->opConstVal_d == *(Mux->Mi->constNu_d + k))
                            flag1 = 1;
                        }
                      }
                      Mux = Mux->next;
                    }
                  }
                  if (flag0 && flag1) continue; // redundant mux when both flags are set! 
                }

                myprintf("P1: Common port number %d for node %d and %d sharing %s\n", \
                    port_nu, dest0, dest1, opSymTable(op_type));


                // Check Mux allocation and allocate a Mux if it is not allocated.
                myprintf("CheckMuxAlloc...\n");
                nextMux = CheckMuxAlloc(Map, op_type, fu_nu, port_nu, ri, rj, 
                                        i, j, 0, &MuxCnt);

                // Insert the allocated Mux into Mux list 
                myprintf("MuxListInsert...\n");
                MuxListInsert(op_type, fu_nu, port_nu, nextMux);
                myprintf("\n");
              }
            }
          }
        }
      }
    }
  }

  //=====================================================================================
  //
  // Allocate FU port ONE's MUX
  // 
  // FU port ONE may contain constants
  //
  // It deals with two cases when Map[ri] == Map[rj] && Map[ri] && Map[rj]
  //   1. both inputs are constants
  //   2. only one input is a constant
  //=====================================================================================
  for (ri = 0; ri < NODE_NU; ri++) {
    for (rj = ri; rj < NODE_NU; rj++) {

      for (i = 0; i < DFG[ri]->opDestNu; i++) {
        //----------------------------------------------------------
        // Including multiple destinations, but some nodes will be
        // visited several times.
        //----------------------------------------------------------
        dest0 = DFG[ri]->opDest[i];
        for (j = 0; j < DFG[rj]->opDestNu; j++) {
          dest1 = DFG[rj]->opDest[j];

          //---------------------------------------------------------
          // The register pair is connected to the same FU and 
          // they're mapped to the same shared register. So
          // Mux is not allocated on FU port 0.
          // But Mux may be allocated on FU port 1.
          //---------------------------------------------------------

          if (dest0 > 0 && dest1 > 0 &&
              Map[ri] == Map[rj] && Map[ri] && Map[rj] && 
              DFG[dest0]->opResourceNu == DFG[dest1]->opResourceNu &&
              DFG[dest0]->op == DFG[dest1]->op) { // same floating precision implied 

            //-------------------------------------------------------------
            // Here mux is not needed at a shared FU's input port 
            // because two inputs to the FU are mapped to the same register.
            //-------------------------------------------------------------

            // operation type
            op_type = DFG[dest0]->op;

            // FU number
            offset = 0; 
            fu_nu = DFG[dest0]->opResourceNu - offset - 1;

            // Constant reduction (the if condition also implies that we are looking at FU
            // port ONE
            if (DFG[dest0]->opConst && DFG[dest1]->opConst) {

              if (DFG[dest0]->opPrecision == sfp && DFG[dest1]->opPrecision == sfp &&
                  DFG[dest0]->opConstVal == DFG[dest1]->opConstVal) {

                myprintf("P2: node %d and %d have the same single constant values %f\n", \
                dest0, dest1, DFG[dest0]->opConstVal);
                continue;
              }

              else if (DFG[dest0]->opPrecision == dfp && DFG[dest1]->opPrecision == dfp &&
                       DFG[dest0]->opConstVal_d == DFG[dest1]->opConstVal_d) {
                myprintf("P2: node %d and %d have the same double constant values %f\n", \
                dest0, dest1, DFG[dest0]->opConstVal_d);
                continue;
              }

              else {
                flag0 = flag1 = 0;
                myprintf("P2: For type = %s fu_nu = %d ",opSymTable(op_type), fu_nu);
                myprintf("check any redundant mux input constant values\n");
                Mux = (MuxRAT[op_type][fu_nu]).Fu_portNu[1];
                while (Mux != NULL) {
                  for (k = 0; k < 2; k++) { 
                    //-------------------------------------------------------------
                    //-------------------------------------------------------------
                    if (DFG[dest0]->opPrecision == sfp) {
                      if (DFG[dest0]->opConstVal == *(Mux->Mi->constNu + k))
                        flag0 = 1;
                      if (DFG[dest1]->opConstVal == *(Mux->Mi->constNu + k))
                        flag1 = 1;
                    }
                    //-------------------------------------------------------------
                    //-------------------------------------------------------------
                    if (DFG[dest0]->opPrecision == dfp) {
                      if (DFG[dest0]->opConstVal_d == *(Mux->Mi->constNu_d + k))
                        flag0 = 1;
                      if (DFG[dest1]->opConstVal_d == *(Mux->Mi->constNu_d + k))
                        flag1 = 1;
                    }
                  }

                  Mux = Mux->next;
                }
              }
              if (flag0 && flag1) 
                continue;
              else
                port_nu = 1;

            } else if (
                (DFG[dest0]->opConst && DFG[dest1]->opSrc[1] == rj) || 
                (DFG[dest0]->opSrc[1] == ri && DFG[dest1]->opConst)) {
              port_nu = 1;
            }
            else
              continue; 

            //-----------------------------------------------------------
            // 6/29/11
            // Map[ri] == Map[rj] means: 
            // 1) ri == rj 
            // 2) ri != rj and Map[ri] == Map[rj]
            //
            // Note on case 1: 
            // A node's multiple destinatins may require a mux for their
            // input ports. In the  example below, the two "*" operations
            // are shared by a multiplier, then
            //
            //                           3.0 4.0
            // D   3.0 D  4.0              \/ 
            //  \   /  \   /  ====>   D    mux
            //   (*)    (*)           \   /
            //                         (*)
            //
            // Here ri(rj) is the node D's id. 
            // If the loop variable rj is set to ri + 1 in the for loop,
            // then this mux cannot be generated because node's src[1] equals
            // src[0] when src[1] is a constant
            //
            // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
            // for (ri = 0; ri < NODE_NU; ri++) {
            //   for (rj = ri; rj < NODE_NU; rj++) {
            // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
            //
            // Map[ri] != Map[rj] ==> ri != rj
            // So no problem when rj is set to ri + 1 in the for loop.
            //-----------------------------------------------------------
            if (ri == rj)
              myprintf("P2: Mux inputs come from the same register\n");
            else
              myprintf("P2: Mux inputs come from different registers\n");

            myprintf("P2: Common port number %d for node %d and %d sharing %s\n", \
                    port_nu, dest0, dest1, opSymTable(op_type));

            // Check Mux allocation and allocate a Mux appropriately
            myprintf("CheckMuxAlloc...\n");
            nextMux = CheckMuxAlloc(Map, op_type, fu_nu, port_nu, ri, rj, 
                                    i, j, 1, &MuxCnt);

            // Insert the Mux into Mux list
            myprintf("MuxListInsert...\n");
            MuxListInsert(op_type, fu_nu, port_nu, nextMux);
          }
        }
      }
    }
  }
  
  MuxRATPrint();

  //MuxInputNu = GenerateHDL(Map, SharedRegNu, ScheduleName); 
  return MuxCnt;
}

void FreeTables(int SharedRegNu) {
  FreeMuxRAT();
  FreeFuRAT(SharedRegNu);
  FreeVarTable(SharedRegNu);
}
//----------------------------------------------------------------
//
//----------------------------------------------------------------
MuxPtr CheckMuxAlloc(int *Map, int op_type, int fu_nu, int port_nu, 
                     int ri, int rj, int di, int dj, 
                     int constFlag, int *MuxCnt) {

  MuxPtr head, Mux;

  myprintf("op_type = %s fu_nu = %d port_nu = %d ri = %d(Map = %d) rj = %d(Map = %d)\n", 
          opSymTable(op_type), fu_nu, port_nu, 
          ri, Map[ri], rj, Map[rj]);

  head = (MuxRAT[op_type][fu_nu]).Fu_portNu[port_nu];

  Mux = CheckMuxList(Map, head, ri, rj, di, dj, 
                     fu_nu, port_nu, constFlag, MuxCnt);

  return Mux; 
}

//-------------------------------------------------------------------
// The head pointer implies that the register pair has a common FU
//-------------------------------------------------------------------
MuxPtr CheckMuxList(int *Map, MuxPtr head, int ri, int rj, int di, int dj, 
                    int fu_nu, int port_nu, int constFlag, int *MuxCnt) {

  //!
  // 4/7/2012 initialization because 
  // if (DFG[DFG[ri]->opDest[di]]->opPrecision == sfp or dfp)
  //!
  float mux_i_c1 = 0, mux_i_c2 = 0; 
  double mux_i_c1_d = 0, mux_i_c2_d = 0; 

  int mux_i_r1, mux_i_r2, 
      mux_i_d1, mux_i_d2, 
      mux_i_m1, mux_i_m2, 
      mux_i_f1, mux_i_f2;
  int mux_o_r, mux_o_m, mux_o_fu;
  int Ri, Rj;
  int Di, Dj;
  int ri_SharedFlag, rj_SharedFlag;
  MuxPtr p;
  MuxPtr parentMux = NULL;
  int *currRegNu;
  float *currConstNu;
  double *currConstNu_d;

  //---------------------------------------------------------------
  // Assume a constant value of zero mustn't exist in the DFG
  //---------------------------------------------------------------
  
  // physical FU's logical source numbers
  Ri = ri;
  Rj = rj;
  //Ri = (mux_i_c1 == 0) ri : -1;
  //Rj = (mux_i_c2 == 0) rj : -1;

  // physical FU's logical destination number 
  // corresponding to logical source numbers
  Di = DFG[ri]->opDest[di];
  Dj = DFG[rj]->opDest[dj];

  // if the operation (ri, di) in DFG(not shared FU) has a constant input
  if (DFG[Di]->opPrecision == sfp) {
    mux_i_c1 = (port_nu == 1 && DFG[Di]->opConst) ?  DFG[Di]->opConstVal : 0;
    assert (DFG[Dj]->opPrecision == sfp);
  }

  if (DFG[Di]->opPrecision == dfp) {
    mux_i_c1_d = (port_nu == 1 && DFG[Di]->opConst) ?  DFG[Di]->opConstVal_d : 0;
    assert (DFG[Dj]->opPrecision == dfp);
  }

  // if the operation (rj, dj) in DFG(not shared FU) has a constant input
  if (DFG[Dj]->opPrecision == sfp) { 
    mux_i_c2 = (port_nu == 1 && DFG[Dj]->opConst) ?  DFG[Dj]->opConstVal : 0;
    assert (DFG[Dj]->opPrecision == sfp);
  }

  if (DFG[Dj]->opPrecision == dfp) {
    mux_i_c2_d = (port_nu == 1 && DFG[Dj]->opConst) ?  DFG[Dj]->opConstVal_d : 0;
  }


  // There is no Mux and allocate a Mux.
  if (head == NULL) {
    mux_i_r1 = Ri;
    mux_i_r2 = Rj;
    mux_i_d1 = Di;
    mux_i_d2 = Dj;
    mux_i_m1 = -1;
    mux_i_m2 = -1;
    mux_i_f1 = -1;
    mux_i_f2 = -1;
    mux_o_r = -1;
    mux_o_m = -1;
    mux_o_fu = fu_nu;
  }
  else {
    p = head;
    ri_SharedFlag = 0;
    rj_SharedFlag = 0;

    // check how to allocate the mux by comparing it with all the
    // allocated muxes.
    while (p != NULL) {

      currRegNu   = p->Mi->regNu;
      currConstNu = p->Mi->constNu;
      currConstNu_d = p->Mi->constNu_d;

      if (constFlag) {
        if (currRegNu[0] == ri || currRegNu[1] == ri)
          ri_SharedFlag = 1;
        if (currRegNu[0] == rj || currRegNu[1] == rj)
          rj_SharedFlag = 1;
      } 
      else {

        //------------------------------------------------------------------
        // if the constant value is the same as any constant of an 
        // allocated mux's left or right input, then a chained mux will
        // be generated later 
        //------------------------------------------------------------------
        if (DFG[DFG[ri]->opDest[di]]->opPrecision == sfp) {
          if (mux_i_c1 && mux_i_c2) {
            if (mux_i_c1 == currConstNu[0] || mux_i_c1 == currConstNu[1])
              ri_SharedFlag = 1;

            if (mux_i_c2 == currConstNu[0] || mux_i_c2 == currConstNu[1])
              rj_SharedFlag = 1;

          } else if (mux_i_c1) {
            if (mux_i_c1 == currConstNu[0] || mux_i_c1 == currConstNu[1])
              ri_SharedFlag = 1;

            // make sure the mux input rj connects a register, not a constant
            if (Map[currRegNu[0]] == Map[rj] && currConstNu[0] == 0 ||
                Map[currRegNu[1]] == Map[rj] && currConstNu[1] == 0)
              rj_SharedFlag = 1;

          } else if (mux_i_c2) {
            // make sure the mux input ri connects a register, not a constant
            if (Map[currRegNu[0]] == Map[ri] && currConstNu[0] == 0 ||
                Map[currRegNu[1]] == Map[ri] && currConstNu[1] == 0)
              ri_SharedFlag = 1;

            if (mux_i_c2 == currConstNu[0] || mux_i_c2 == currConstNu[1])
              rj_SharedFlag = 1;

          } else {
            if (Map[currRegNu[0]] == Map[ri] || Map[currRegNu[1]] == Map[ri])
              ri_SharedFlag = 1;

            if (Map[currRegNu[0]] == Map[rj] || Map[currRegNu[1]] == Map[rj])
              rj_SharedFlag = 1;
          }
        }

        if (DFG[DFG[ri]->opDest[di]]->opPrecision == dfp) {
          if (mux_i_c1_d && mux_i_c2_d) {
            if (mux_i_c1_d == currConstNu_d[0] || mux_i_c1_d == currConstNu_d[1])
              ri_SharedFlag = 1;

            if (mux_i_c2_d == currConstNu_d[0] || mux_i_c2_d == currConstNu_d[1])
              rj_SharedFlag = 1;

          } else if (mux_i_c1_d) {
            if (mux_i_c1_d == currConstNu_d[0] || mux_i_c1_d == currConstNu_d[1])
              ri_SharedFlag = 1;

            // make sure the mux input connects a register, not a constant
            if (Map[currRegNu[0]] == Map[rj] && currConstNu_d[0] == 0 ||
                Map[currRegNu[1]] == Map[rj] && currConstNu_d[1] == 0)
              rj_SharedFlag = 1;

          } else if (mux_i_c2_d) {
            // make sure the mux input connects a register, not a constant
            if (Map[currRegNu[0]] == Map[ri] && currConstNu_d[0] == 0 ||
                Map[currRegNu[1]] == Map[ri] && currConstNu_d[1] == 0)
              ri_SharedFlag = 1;

            if (mux_i_c2_d == currConstNu_d[0] || mux_i_c2_d == currConstNu_d[1])
              rj_SharedFlag = 1;

          } else {
            if (Map[currRegNu[0]] == Map[ri] || Map[currRegNu[1]] == Map[ri])
              ri_SharedFlag = 1;

            if (Map[currRegNu[0]] == Map[rj] || Map[currRegNu[1]] == Map[rj])
              rj_SharedFlag = 1;
          }
        }
      }

      p = p->next;
    }

    p = head;

    // Same Mux inputs with no allocation needed
    if (ri_SharedFlag && rj_SharedFlag) {
      return NULL;
    }

    else if (ri_SharedFlag) { // rj_SharedFlag = 0 is implied

      // Get new mux port 1 input source in a chained muxs
      parentMux = GetMuxID(p);
      mux_i_r1 = Ri;
      mux_i_r2 = Rj;
      mux_i_d1 = Di;
      mux_i_d2 = Dj;
      mux_i_m1 = parentMux->m;
      mux_i_m2 = -1;
      mux_i_f1 = -1;
      mux_i_f2 = -1;
      mux_o_r = -1;
      mux_o_m = -1;
      mux_o_fu = fu_nu;
    }

    else if (rj_SharedFlag) {
      // Get new mux port 2 input source in a chained muxs
      parentMux = GetMuxID(p);
      mux_i_r1 = Ri;
      mux_i_r2 = Rj;
      mux_i_d1 = Di;
      mux_i_d2 = Dj;
      mux_i_m1 = -1;
      mux_i_m2 = parentMux->m;
      mux_i_f1 = -1;
      mux_i_f2 = -1;
      mux_o_r = -1;
      mux_o_m = -1;
      mux_o_fu = fu_nu;
    }

    else {
      mux_i_r1 = Ri;
      mux_i_r2 = Rj;
      mux_i_d1 = Di;
      mux_i_d2 = Dj;
      mux_i_m1 = -1;
      mux_i_m2 = -1;
      mux_i_f1 = -1;
      mux_i_f2 = -1;
      mux_o_r = -1;
      mux_o_m = -1;
      mux_o_fu = fu_nu;
    }
  }

  return MuxAlloc(Map, parentMux,
                  mux_i_r1, mux_i_r2, 
                  mux_i_d1, mux_i_d2, 
                  mux_i_c1, mux_i_c2, 
                  mux_i_c1_d, mux_i_c2_d, 
                  mux_i_m1, mux_i_m2, 
                  mux_i_f1, mux_i_f2, 
                  mux_o_r, mux_o_m, 
                  mux_o_fu, MuxCnt);
}

//------------------------------------------------------------------------- 
//
//------------------------------------------------------------------------- 
void MuxListInsert(int op_type, int fu_nu, int port_nu, MuxPtr nextMux) {

  MuxPtr p0, p1;
  MuxPtr head;

  // No Mux allocated
  if (nextMux == NULL) return;

  p0 = head = (MuxRAT[op_type][fu_nu]).Fu_portNu[port_nu];

  if (head == NULL) {
    head = nextMux;
    myprintf("Create head Mux ID %d \n", nextMux->m);
    //nextMux->next = NULL;
  } else {
    while (p0 != NULL) {
      p1 = p0;
      p0 = p0->next; 
    }

    // Append the Mux at the end of the list
    myprintf("Append Mux ID %d \n", nextMux->m);
    p1->next = nextMux;
    nextMux->next = NULL;
  }

  (MuxRAT[op_type][fu_nu]).Fu_portNu[port_nu] = head;
}

void MuxRATPrint() {

  enum operation op_type;
  int fu_nu, port_nu;
  int portNu;
  MuxPtr Mux, head;
  int i;
  int size;

  myprintf("------------ MuxRAT ---------------------------\n");

  for (op_type = add; op_type < add + OP_NU; op_type++) {

    size = (*RQ[op_type])->size;

    for (fu_nu = 0; fu_nu < size; fu_nu++) {

      portNu = GetNodeSrcNu(op_type);

      for (port_nu = 0; port_nu < portNu; port_nu++) {

         Mux = (MuxRAT[op_type][fu_nu]).Fu_portNu[port_nu];

         while (Mux != NULL) {

           myprintf("MuxRAT: op = %3d fu_nu = %3d port_nu = %3d\n",
                   op_type, fu_nu, port_nu);

           myprintf("Mux number: %d\n", Mux->m);

           myprintf("Mux partition group number: %d\n", Mux->grp);

           myprintf("i SP Constant Number: ");
           for (i = 0; i < 2; i++) myprintf("%f ", *(Mux->Mi->constNu + i)); 

           myprintf("i DP Constant Number: ");
           for (i = 0; i < 2; i++) myprintf("%f ", *(Mux->Mi->constNu_d + i)); 

           myprintf("\ni Register Number: ");
           for (i = 0; i < 2; i++) myprintf("%d ", *(Mux->Mi->regNu + i)); 

           myprintf("\ni Destination Number: ");
           for (i = 0; i < 2; i++) myprintf("%d ", *(Mux->Mi->destNu + i)); 

           myprintf("\ni Mux Number: ");
           for (i = 0; i < 2; i++) myprintf("%d ", *(Mux->Mi->muxNu + i)); 

           myprintf("\ni Fu Number: ");
           for (i = 0; i < 2; i++) myprintf("%d ", *(Mux->Mi->fuNu + i)); 

           myprintf("\no Mux Number: ");
           myprintf("%d ", *(Mux->Mo->muxNu));

           myprintf("\no Fu Number: ");
           myprintf("%d ", *(Mux->Mo->fuNu));

           myprintf("\n\n");
           Mux = Mux->next;
         }
      }
    }
  }
}

//-------------------------------------------------------------------
// If the input port of the newly allocated Mux is the same
// as that of any Mux in the Mux list, find the last Mux output port 
// that the input port of the new Mux can be connected to.
//
// Return: the MuxID of the last Mux which meets the above requirement
//         is returned.
//-------------------------------------------------------------------
MuxPtr GetMuxID(MuxPtr head) {
  MuxPtr p, parentMux;
  p = head;

  while (p != NULL) {
    if (*(p->Mo->muxNu) == -1) {
      assert(*(p->Mo->fuNu) != -1);
      *(p->Mo->fuNu) = -1;
      break;
    }
    p = p->next;
  }
  return (p);
}
  
//--------------------------------------------------------------------------
// For each shared register, find the set of FU that
// write the result into the register.
//--------------------------------------------------------------------------
ResourceList ResourceListInsert(ResourceList head, enum operation op, int resourceNu) {

  ResourceList resource;
  ResourceList p0, p1, p2;

  p0 = p2 = head;

  /* Check resource op and number are unique */  
  while (p2 != NULL) {
    // +1/5/11
    // Add port nodes only when they are mapped to different physical ports
    if (op == nop && GetInputPortNu(p2->nu) == GetInputPortNu(resourceNu) ||
        p2->nu == resourceNu && p2->op == op)

    // -1/5/11
    //if (p2->nu == resourceNu && p2->op == op)
      return head;
    else
      p2 = p2->next;
  }

  resource = malloc(sizeof(struct Resource));
  resource->op = op;
  resource->nu = resourceNu;

  if (head == NULL) {
    //myprintf("Empty: Inserting resource type %d nu %d \n", op, resourceNu);
    head = resource;
    resource->next = NULL;
  } else {
    while (p0 != NULL) {
      p1 = p0;
      p0 = p0->next;
    }

    //myprintf("Append: Inserting resource type %d nu %d \n", op, resourceNu);
    p1->next = resource;
    resource->next = NULL;
  }   
  return (head);
}

void ResourceListPrint(ResourceList head) {
  ResourceList p = head;
  while (p != NULL) {
    myprintf("--- op = %s nu = %3d\n", opSymTable(p->op), p->nu);
    p = p->next;
  }
}


//-------------------------------------------------
// The structure of the FuRAT:
//
//      sr: shared register
//      Fu: Fu number and operation
//
//      +---+      
//      +sr0+  not used 
//      +---+       +---+
//      +sr1+  ---  +Fu + -- Resource Lists
//      +---+       +---+ 
//      +sr2+  ..... ............
//      +---+
//      +sr3+  ..... ............
//      +---+
//-------------------------------------------------
void CreateFuRAT(int *Map, int SharedRegNu) {

  enum operation op_type;
  int resourceNu;
  int regNu;
  int i;

  FuRAT = (ResourceList *) malloc (sizeof(ResourceList) * (SharedRegNu + 1));
  for (i = 0; i <= SharedRegNu; i++) FuRAT[i] = NULL;

  for (regNu = 1; regNu <= SharedRegNu; regNu++) {
    for (i = 0; i < NODE_NU; i++) {
      if (regNu == Map[i]) {
        op_type = DFG[i]->op;
        resourceNu = DFG[i]->opResourceNu;
        FuRAT[regNu] = ResourceListInsert(FuRAT[regNu], op_type, resourceNu);
      }
    }
  }

  for (i = 1; i <= SharedRegNu; i++) {
    myprintf("Shared register %d resource list:\n", i);
    ResourceListPrint(FuRAT[i]);
  }
}


//-------------------------------------------------
// The structure of the MuxRAT:
//
//      op: non-port FU operation type
//      f: FU number
//      p: FU's port number
//
//      op0         +---+
//      +---+       +p0 + -- Mux Lists
//      +f0 +       +---+  
//      +   +  ---        
//      +   +       +---+
//      +   +       +p1 + -- Mux Lists
//      +---+       +---+ 
//      +f1 +       ..... ............
//      +   +  ---  
//      +   +       ..... ............
//      +---+
//      +f2 +       ..... ............
//      +   +  ---  
//      +   +       ..... ............
//      +---+
//      
//      op1         +---+ 
//      +---+       +p0 + -- Mux Lists
//      +f0 +       +---+
//      +   +  ---       
//      +   +       +---+
//      +   +       +p1 + -- Mux Lists
//      +---+       +---+ 
//      +f1 +       ..... ............
//      +   +  ---  
//      +   +       ..... ............     
//      +---+
//      +f2 +       ..... ............
//      +   +  ---  
//      +   +       ..... ............       
//      +---+
//
//      ....   ...  ....  ............
//      ....   ...  ....  ............
//-------------------------------------------------
void CreateMuxRAT() {

  int size;
  int fu_nu, port_nu;
  int portNu;
  enum operation op_type; 
  
  MuxRAT = (Fu**) malloc (sizeof(Fu*) * OP_NU);

  for (op_type = add; op_type < add + OP_NU; op_type++) {

    // Number of non-port FUs
    size = (*RQ[op_type])->size;

    MuxRAT[op_type] = (Fu*) malloc (sizeof(Fu) * size);

    for (fu_nu = 0; fu_nu < size; fu_nu++) {

      // 7/30/11 
      portNu = GetNodeSrcNu(op_type);

      // Two input ports of each FU
      (MuxRAT[op_type][fu_nu]).Fu_portNu = 
        (MuxPtr *) malloc (sizeof(MuxPtr) * portNu);
      // 
      for (port_nu = 0; port_nu < portNu; port_nu++)
        (MuxRAT[op_type][fu_nu]).Fu_portNu[port_nu] = NULL;
    }
  }
}

//-------------------------------------------------
// Allocate a new Mux and configure its i/o ports
//-------------------------------------------------
MuxPtr MuxAlloc(int *Map, MuxPtr parentMux, 
                int mux_i_r1, int mux_i_r2, 
                int mux_i_d1, int mux_i_d2, 
                float mux_i_c1, float mux_i_c2, 
                double mux_i_c1_d, double mux_i_c2_d, 
                int mux_i_m1, int mux_i_m2,
                int mux_i_f1, int mux_i_f2,
                int mux_o_r,  int mux_o_m, 
                int mux_o_fu, int *MuxCnt) {

  float *constNu;
  double *constNu_d;
  int *destNu, *regNu, *muxNu, *fuNu;

  MuxPtr MP;

  (*MuxCnt)++;

  // Allocate a Mux between register pairs and FU
  MP     = (MuxPtr) malloc (sizeof(struct Mux));
  MP->Mi = (MiPtr) malloc (sizeof(MuxInputs));
  MP->Mo = (MoPtr) malloc (sizeof(MuxOutputs));
  MP->m  = *MuxCnt;
  MP->grp  = 1;   // 1 group by default
  MP->next = NULL; 

  MP->Mi->constNu = (float *) malloc (2 * sizeof(float)); 
  MP->Mi->constNu_d = (double *) malloc (2 * sizeof(double)); 

  MP->Mi->destNu = (int *) malloc (2 * sizeof(int)); 

  MP->Mi->regNu = (int *) malloc (2 * sizeof(int)); 

  MP->Mi->muxNu = (int *) malloc (2 * sizeof(int)); 

  MP->Mi->fuNu = (int *) malloc (2 * sizeof(int)); 

  MP->Mo->regNu = (int *) malloc (sizeof(int)); 

  MP->Mo->muxNu = (int *) malloc (sizeof(int)); 

  MP->Mo->fuNu =  (int *) malloc (sizeof(int)); 

  constNu = MP->Mi->constNu;
  constNu[0] = mux_i_c1;
  constNu[1] = mux_i_c2;

  constNu_d = MP->Mi->constNu_d;
  constNu_d[0] = mux_i_c1_d;
  constNu_d[1] = mux_i_c2_d;

  regNu = MP->Mi->regNu;
  regNu[0] = mux_i_r1;
  regNu[1] = mux_i_r2;

  destNu = MP->Mi->destNu;
  destNu[0] = mux_i_d1;
  destNu[1] = mux_i_d2;

  muxNu = MP->Mi->muxNu;
  muxNu[0] = mux_i_m1;
  muxNu[1] = mux_i_m2;

  fuNu = MP->Mi->fuNu;
  fuNu[0] = mux_i_f1;
  fuNu[1] = mux_i_f2;

  *(MP->Mo->fuNu) = mux_o_fu;
  *(MP->Mo->regNu) = mux_o_r;
  *(MP->Mo->muxNu) = mux_o_m;

  // Change parent Mux output destination 
  // to the current Mux ID number MuxCnt
  if (parentMux != NULL)
    *(parentMux->Mo->muxNu) = *MuxCnt;

  return MP;
}
  
void FreeFuRAT (int SharedRegNu) {
  int regNu;
  ResourceList head, p, p1, p2;

  // Free Fu list
  for (regNu = 1; regNu <= SharedRegNu; regNu++) {
    p = head = FuRAT[regNu];
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
        free(p1);
        p = head;
      } else {
        free(p);
        break; 
      }
    }
  }

  free(FuRAT);
}

void FreeMuxList() {

  enum operation op_type; 
  int fu_nu, port_nu;
  int portNu;
  int size;
  MuxPtr head, p, p1, p2;

  for (op_type = add; op_type < add + OP_NU; op_type++) {
    size = (*RQ[op_type])->size;
    for (fu_nu = 0; fu_nu < size; fu_nu++) {
      portNu = GetNodeSrcNu(op_type);
      for (port_nu = 0; port_nu < portNu; port_nu++) {
        head = (MuxRAT[op_type][fu_nu]).Fu_portNu[port_nu];
        if (head != NULL) {
          p = head;
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
              free(p1->Mi->constNu);
              free(p1->Mi->constNu_d);
              free(p1->Mi->regNu );
              free(p1->Mi->destNu );
              free(p1->Mi->muxNu );
              free(p1->Mi->fuNu  );
              free(p1->Mo->regNu );
              free(p1->Mo->muxNu );
              free(p1->Mo->fuNu  );
              free(p1->Mi);
              free(p1->Mo);
              free(p1);
              p = head;
            } else {
              free(p->Mi->constNu);
              free(p->Mi->constNu_d);
              free(p->Mi->regNu );
              free(p->Mi->destNu );
              free(p->Mi->muxNu );
              free(p->Mi->fuNu  );
              free(p->Mo->regNu );
              free(p->Mo->muxNu );
              free(p->Mo->fuNu  );
              free(p->Mi);
              free(p->Mo);
              free(p);
              break; 
            }
          }
        }
      }
    }
  }
}

void FreeMuxRAT() {

  enum operation op_type;
  int fu_nu, port_nu;
  int size;

  FreeMuxList();

  for (op_type = add; op_type < add + OP_NU; op_type++) {

    size = (*RQ[op_type])->size;

    for (fu_nu = 0; fu_nu < size; fu_nu++)
      free((MuxRAT[op_type][fu_nu]).Fu_portNu);

    free(MuxRAT[op_type]);
  }

  free(MuxRAT);
}


