#include <stdio.h>
#include <assert.h>
#include "Schedule.h"
#include "MUX.h"

// Just a 2-stage partition 
void MuxChainPartition() {

  enum operation op_type;
  int portNu;
  int size, fu_nu, port_nu, mux_id;
  MuxPtr Mux, Mux_tmp;
  const int GRP=2;
  int i, g, grp_nu;
  int f[GRP];

  int MuxChainCnt;

 //--------------------------------------------------------------------------
 // mux chain register insertion 8/25/2012 
 //
 // If operation o is not included, then the group IDs of all the MUXes that
 // targets the FU of operation o are the same.
 //--------------------------------------------------------------------------

 for (op_type = add; op_type < add + OP_NU; op_type++) {
#if MUX_OP == 1
   if (op_type == div_) {
#elif MUX_OP == 2
   if (op_type == add) {
#elif MUX_OP == 3
   if (op_type == div_ || op_type == add) {
#elif MUX_OP == 7
   if (op_type == div_ || op_type == add  || op_type == mul) {
#elif MUX_OP == 14
   if (op_type == add  || op_type == mul  || op_type == sub) {
#elif MUX_OP == 15
   if (op_type == div_ || op_type == add  || 
       op_type == mul  || op_type == sub) {
#else
   if (1) { return;
#endif

     size = (*RQ[op_type])->size; // number of FUs for each operation type 

     for (fu_nu = 0; fu_nu < size; fu_nu++) {

       portNu = GetNodeSrcNu(op_type);

       for (port_nu = 0; port_nu < portNu; port_nu++) {

         Mux_tmp = Mux = (MuxRAT[op_type][fu_nu]).Fu_portNu[port_nu];

         MuxChainCnt = 0;
         while (Mux != NULL) {
           MuxChainCnt++;
           Mux = Mux->next;
         }

         Mux = Mux_tmp;
         grp_nu = 0;

         for (i = 0; i < GRP; i++) f[i] = (MuxChainCnt / GRP);
         f[GRP-1] = f[GRP-1] + (MuxChainCnt % GRP);

         for (g = 0; g < GRP; g++) {
           for (i = 0; i < f[g]; i++) {
             Mux->grp = grp_nu;
             Mux = Mux->next;
           }
           grp_nu++;
         }
         // check group assignment
         MuxRATPrint();
       }
     }
   }
 }
}

// Descriptions of register addition 
//
// Assumption
//   Based on a 2-1 MUX chain
//
// For each op_type in the OP set(e.g. add, sub)
//   For each fu_nu of type op_type
//     For each port port_nu of the fu
//        For each MUX on the group2 path that ends at port_nu
//           Get the shared register number at MUX input2 if MUX input2 is not a constant.
//           For each variable that is mapped to the shared register number,    
//             1. Get a pair of src and dst nodes whose edge corresponds to the variable
//             2. In the MuxSelTable, find the MUX whose select time (when MUX sel=1) 
//                equals the scheduled time of the dst node plus 1(the previous cycle).
//             3. Check if the MUX can select the content of the shared register before the 
//                content is overwritten(since it is a shared register). 
//                We first calculate the difference between the MUX select time and the shared register write time.
//                if the difference is larger than DII, then further examine which RF regsiter port number
//                MUX port 2 reads.  
//             4. If the content is not overwritten, then we don't add another register for the shared
//                register. Otherwise, add another register.
//

void AddMuxRegister(int *Map) {

  enum operation op_type;
  int fu_nu, port_nu, mux_id;
  int i, j, size;
  MuxPtr p, head; 
  int sharedReg;
  int Ri, Rj;
  float Ci, Cj;
  double Ci_d, Cj_d;

  for (op_type = add; op_type < add + OP_NU; op_type++) {
# if MUX_OP == 1
    if (op_type == div_) {
# elif MUX_OP == 2
    if (op_type == add) {
# elif MUX_OP == 3
    if (op_type == div_ || op_type == add) {
# elif MUX_OP == 7
    if (op_type == div_ || op_type == add  || op_type == mul) {
# elif MUX_OP == 14
    if (op_type == add  || op_type == mul  || op_type == sub) {
# elif MUX_OP == 15
    if (op_type == div_ || op_type == add  || 
        op_type == mul  || op_type == sub) {
# else
    if (1) { return;
# endif

      size = (*RQ[op_type])->size; // number of FUs for each operation type 

      for (fu_nu = 0; fu_nu < size; fu_nu++) {

        int portNu = GetNodeSrcNu(op_type);

        for (port_nu = 0; port_nu < portNu; port_nu++) {

          p = head = (MuxRAT[op_type][fu_nu]).Fu_portNu[port_nu];

          while (p != NULL) {

            Ri = *(p->Mi->regNu);     // logical register number of mux input0
            Rj = *(p->Mi->regNu + 1); // logical register number of mux input1
            Ci = *(p->Mi->constNu);     // logical register number of mux input0
            Cj = *(p->Mi->constNu + 1); // logical register number of mux input1
            Ci_d = *(p->Mi->constNu_d);     // logical register number of mux input0
            Cj_d = *(p->Mi->constNu_d + 1); // logical register number of mux input1
            if (p->grp == 1) { 
              if (Cj == 0 && Cj_d == 0) {
                sharedReg = Map[Rj];
                mux_id = p->m;
                CheckMuxRegister(op_type, fu_nu, port_nu, sharedReg, mux_id);
              }
            }
            p = p->next;
          }
        }
      }
    }
  }
}

// all register r write slots. which slot corresponds to the event that
// r will be read by fu ?
//
// A list of VarID in VarTable[sharedReg]
//
void CheckMuxRegister(enum operation op_type, int fu_nu, int port_nu, int sharedReg, int mux_id) {

 int i, t;
 int src, dst;
 varList v, l;
 MuxSelList m, p;
 int wenTime;
 int ofst, mulp;

 myprintf("====== Check Mux Register======\n");

 myprintf("op=%s fu=%d port=%d shreg=%d mux_id=%d:\n", 
     opSymTable(op_type), fu_nu, port_nu, sharedReg, mux_id);

 v = VarTable[sharedReg-1];

 while (v != NULL) {

   src = v->varID;

   for (i = 0; i < DFG[src]->opDestNu; i++) {

     dst = DFG[src]->opDest[i];
     if (dst != SINK) {

       if (DFG[dst]->opResourceNu == fu_nu + 1 && 
           DFG[dst]->op == op_type &&
           DFG[dst]->opSrc[port_nu] == src) {

         assert(v->startTime == DFG[src]->opResultDoneSlot);

         m = p = MuxSelTable[mux_id - 1];
         while (p != NULL) {
           if (p->startTime == DFG[dst]->opScheduledSlot - 1) {
             myprintf("startTime %d selVal %d inputNu %d\n", 
               p->startTime, p->selVal, p->inputNu);
             assert (p->selVal == 1);
             m = p;
             break;
           }
           p = p -> next;
         }

         myprintf("find reg_write & mux_read path: node%d ---> node%d\n", src, dst);

         assert(v->startTime < m->startTime);

         // register write time and mux select time when s=1
         myprintf("Treg_wen=%d Tmux_sel=%d\n", v->startTime, m->startTime);

         if (m->startTime - v->startTime > minII) {
           //
           ofst = (m->startTime - v->startTime) % minII ? 1 : 0;
           mulp = (m->startTime - v->startTime) / minII;
           if (mulp + ofst != GetRegFilePortNu(src, dst)) {
             myprintf("case0: add extra r%d\n", sharedReg);
             MuxSelTable[mux_id-1]->addMuxReg = 1;
           }
           continue;
         }

         l = VarTable[sharedReg-1];

         while (l != NULL) {

           wenTime = l->startTime;

           if (wenTime != v->startTime) {

             if (wenTime < v->startTime) {
               t = wenTime ;
               do {
                 t = t + minII;
                 if (t < m->startTime && t > v->startTime) break;
               } while (t < m->startTime);

               if (t < m->startTime) {
                 myprintf("case1: add extra r%d\n", sharedReg);

                 // we only care about if we add an extr reg to MUX mux_id 
                 // though there may be more than one time when sel = 1 for MUX mux_id
                 // (m->addMuxReg) vs.  MuxSelTable[mux_id]->addMuxReg
                 MuxSelTable[mux_id-1]->addMuxReg = 1;
               }
             }
               
             else if (wenTime > v->startTime && wenTime < m->startTime) {
               myprintf("case2: add extra r%d\n", sharedReg);
               MuxSelTable[mux_id-1]->addMuxReg = 1;
             }

             else { // wenTime > m->startTime
               t = wenTime ;
               do {
                 t = t - minII;
                 if (t < m->startTime && t > v->startTime) break;
               } while (t > v->startTime);

               if (t > v->startTime) {
                 myprintf("case3: add extra r%d\n", sharedReg);
                 MuxSelTable[mux_id-1]->addMuxReg = 1;
               }
             }
           } // !=
           l = l->next;
         } // while
       }
     }
   }
   v = v->next;
 }
}
                



