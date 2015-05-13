#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "Schedule.h"

void GetTotalMuxInputs(int muxpCnt, int muxCnt, int SharedRegNu) {
  enum operation op_type;
  int fu_nu, port_nu;
  int i, j;
  int size;
  MuxPtr Mux;
  RF_portPtr RegFileTable; 

  int totalMuxInputNu = 0;
  int maxMuxInputNu;
  int impMuxInputNu;
  int portNu;
  int destNu;

  int dest, dest_cnt;
  int tmp;
  int dataWidth;
  int *dest_nu  = NULL;
  MuxSelList m;
  varList p;
  int last, RF_PortNu, flag;
  int cnt = 0;

//-----------------------------------------------------------------------------
// MUXs from RF ports to a MUX input port
//-----------------------------------------------------------------------------
    for (op_type = add; op_type < add + OP_NU; op_type++) {
      size = (*RQ[op_type])->size; 

      for (fu_nu = 0; fu_nu < size; fu_nu++) {

        portNu = GetNodeSrcNu(op_type);

        for (port_nu = 0; port_nu < portNu; port_nu++) { 
           Mux = (MuxRAT[op_type][fu_nu]).Fu_portNu[port_nu];

           if (Mux == NULL) continue;

           maxMuxInputNu = 0; // reset for each fu port

           while (Mux != NULL) {
             maxMuxInputNu += 1;
             Mux = Mux->next;
           }
           maxMuxInputNu += 1;

           for (i = 0; i < NODE_NU; i++) {
             dest_cnt = 0;
             dest_nu  = NULL;

             for (j = 0; j < DFG[i]->opDestNu; j++) {

               dest = DFG[i]->opDest[j];

               if (dest > 0) {

                 if (port_nu < DFG[dest]->opSrcNu && DFG[dest]->opSrc[port_nu] == i &&  // 4/12/12
                     DFG[dest]->opResourceNu == (fu_nu + 1) && 
                     DFG[dest]->op == op_type) {

                   if (!(DFG[dest]->opConst && port_nu == 1)) {
                     if (DFG[i]->RegFileTable != NULL &&
                        (TotalRegFilePortNu(DFG[i]->RegFileTable, DFG[i]->opDestNu) != 1)) {
                       dest_cnt++;
                       dest_nu = (int *) realloc(dest_nu, sizeof(int) * dest_cnt);
                       dest_nu[dest_cnt-1] = dest;
                     } // multiple regfile ports
                   } // not a constant Mux input
                 } // shared registers connected to a FU
               } // if (dest > 0)
             } // for (j = 0; j < DFG[i]->opDestNu; j++)

             tmp = dest_cnt;

             if (dest_cnt > 1) {

               flag = 0;

               //----------------------------------------------------------------------
               // Check if multiple RF read ports share a FU input thru MUXs
               // Note that there are multiple destinations connected to the FU's port input
               //----------------------------------------------------------------------
               // for one destination of node i
               last = RF_PortNu = GetRegFilePortNu(i, dest_nu[--dest_cnt]);

               // for other destination(s) of node i
               while(dest_cnt-- > 0) {
                 RF_PortNu = GetRegFilePortNu(i, dest_nu[dest_cnt]);
                 if (last != RF_PortNu) {
                   //myprintf("******* Multiple RF read data ports actually share a FU input port\n"); 
                   flag = 1;
                 }
                 last = RF_PortNu;
               }

               dest_cnt = tmp;

               if (flag) {
                 cnt++;
                 while(dest_cnt-- > 0) {
                   RF_PortNu = GetRegFilePortNu(i, dest_nu[dest_cnt]);
                   totalMuxInputNu++;
                   maxMuxInputNu++;
                 }
                 last = RF_PortNu;
                 maxMuxInputNu--; // see the explanation below 
               }
             }
             free(dest_nu);
           }
 
           myprintf("Fanin at %s%d_p%d = %d;\n", 
               opSymTable(op_type), fu_nu, port_nu, maxMuxInputNu);
           if (CSP->fan < maxMuxInputNu) CSP->fan = maxMuxInputNu;
         } // for 
      }
    }
    myprintf("MUXs from RF ports to a MUX input port = %d(acc)\n", totalMuxInputNu);
//-----------------------------------------------------------------------------
// MUXs from multiple shared registers to each FU input
//
// Note if there are MUXs from RF ports to a MUX input port, that mux input is
// not counted. To do so, we can subtract the results after executing the following
// for loop from the number of MUXs from RF ports to a MUX input port(cnt).
//-----------------------------------------------------------------------------
  for (op_type = add; op_type < add + OP_NU; op_type++) {

    dataWidth = DataPathWidth(op_type, 0);

    size = (*RQ[op_type])->size;
    for (fu_nu = 0; fu_nu < size; fu_nu++) {

      // 7/30/11
      int portNu = GetNodeSrcNu(op_type);

      for (port_nu = 0; port_nu < portNu; port_nu++) {

         Mux = (MuxRAT[op_type][fu_nu]).Fu_portNu[port_nu];

         if (Mux == NULL) continue;

         while (Mux != NULL) {
           totalMuxInputNu += 1;
           Mux = Mux->next;
         }
         totalMuxInputNu += 1;
         //printf("o=%d f=%d p=%d %d(acc)\n", \
             op_type, fu_nu, port_nu, totalMuxInputNu);
      }
    }
  }

  totalMuxInputNu = totalMuxInputNu - cnt;
  CSP->muxp = totalMuxInputNu * 32;  // assume 32-bit mux

myprintf("MUXs from multiple shared registers to each FU input = %d(acc)\n", totalMuxInputNu);

//-----------------------------------------------------------------------------
// MUXs from multiple FU outputs to a register input
//-----------------------------------------------------------------------------
  for (i = muxpCnt; i < muxCnt; i++) {
    m = MuxSelTable[i];
    totalMuxInputNu += m->inputNu;
    CSP->muxr += m->inputNu * 32;  // assume 32-bit mux
    if (CSP->fan < m->inputNu) CSP->fan = m->inputNu;
  }
myprintf("MUXs from multiple FU outputs to a register input = %d(acc)\n", totalMuxInputNu);


//-----------------------------------------------------------------------------
// MUXs from multiple RF read addresses to a RF read port 
//-----------------------------------------------------------------------------
  for (i = 0; i < SharedRegNu; i++) {
    p = VarTable[i];
    while (p != NULL) {
      destNu = DFG[p->varID]->opDestNu;

      RegFileTable = DFG[p->varID]->RegFileTable;

      if (RegFileTable != NULL) {
        for (j = 0; j < destNu; j++) {
          if (RegFileTable[j].RegFileList != NULL) {
            if (RegFileTable[j].sharedCnt > 1) {
              totalMuxInputNu += RegFileTable[j].sharedCnt;
              //if (RegFileTable[i].sharedCnt > maxRegMuxInput)
                //maxRegMuxInput = RegFileTable[i].sharedCnt;
            }
          }
        }
      }
      p = p->next;
    }
  }
myprintf("MUXs from multiple RF read addresses to a RF read port  = %d %d(acc)\n", totalMuxInputNu, 32*totalMuxInputNu);

//-----------------------------------------------------------------------------
// MUXs for N-1 MUX select value (N > 2) 
//-----------------------------------------------------------------------------
  int totalMuxInputBits = totalMuxInputNu * 32;
  for (i = 0; i < muxCnt; i++) {
    m = MuxSelTable[i];
    if (m->inputNu > 2) { 
      totalMuxInputBits += m->inputNu * IntLog2(m->inputNu);
    }
  }
myprintf("MUXs(bits) for N-1 MUX select value (N > 2) = %d(acc)\n", totalMuxInputBits);

//-----------------------------------------------------------------------------
// record the total number of mux input Bits
//-----------------------------------------------------------------------------
  CSP->mux = totalMuxInputBits;

//-----------------------------------------------------------------------------
// record the total number of muxr inputs
//-----------------------------------------------------------------------------
// CSP->muxr 

//-----------------------------------------------------------------------------
// record the total number of muxp inputs
//-----------------------------------------------------------------------------
// CSP->muxp
}
