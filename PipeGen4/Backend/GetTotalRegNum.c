#include <stdio.h>
#include "Schedule.h"

void GetTotalRegNum(int pipeline_depth, int SharedRegNu) {
  enum operation op_type;
  int i, j;
  int size;
  int dataWidth;
  int dataDepth;
  int lifeTime;
  varList p;
  int totalRegBits = 0;

// -----------------------------------------------------------
// FU output registers. We don't count FU input registers
// See GenerateOutputAssign.c
// -----------------------------------------------------------

// -----------------------------------------------------------
// Output counters and ready registers
// See GenerateOutputAssign.c
// -----------------------------------------------------------
myprintf("output counters and rdy register bits %d\n", CSP->reg);

// -----------------------------------------------------------
// Mux chain registers
// See GenerateFlowHDL.c
// -----------------------------------------------------------
CSP->reg += CSP->mux_reg * 32;
myprintf("MUX chain registers %d(acc)\n", CSP->reg);

// -----------------------------------------------------------
// Shared registers
// -----------------------------------------------------------
  for (i = 0; i < SharedRegNu; i++) {
    p = VarTable[i];

    while (p != NULL) {

      dataWidth = DataPathWidth(DFG[p->varID]->op, 1);

      lifeTime = p->endTime - p->startTime;
      if (lifeTime > minII && p->endTime != 10000) {
        dataDepth = (lifeTime / minII) + ((lifeTime % minII) ? 1 : 0);
        totalRegBits += dataWidth * dataDepth;

        // additional counters
        totalRegBits += IntLog2(minII);
      }
      else { 
        totalRegBits += dataWidth * 1;
        break;  // no duplicate of a simple register  
      }
      p = p->next;
    }
  }
  myprintf("shared register bits %d\n", totalRegBits);


// -----------------------------------------------------------
// Registered input ports
// -----------------------------------------------------------
  totalRegBits += PORT_NU * 32;

  myprintf("input port register bits %d(acc)\n", totalRegBits);

// -----------------------------------------------------------
// States 
// -----------------------------------------------------------
  totalRegBits += pipeline_depth;

  myprintf("state register bits %d(acc)\n", totalRegBits);

// -----------------------------------------------------------
// shift register control (state_cnt and state_ctl_din)
// -----------------------------------------------------------
  totalRegBits += IntLog2(minII) + 1;

  myprintf("shift control register bits %d(acc)\n", totalRegBits);


  CSP->reg += totalRegBits;
}
