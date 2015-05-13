#include <stdio.h> 
#include <assert.h> 
#include "Schedule.h"

//------------------------------------------------------------------
// Check resource usage conflict
//
// To solve the conflicts:
// 1) Adjust FU's latency under time and area constraints
// 2) Increase initiation interval until there is no resource onflict
// 3) Add more resources 
//------------------------------------------------------------------

int CheckConflict() {
  int i, j, k;
  int cycle = 1;                     // Actual II
  int slot1, slot2, slot_diff;       // The first and second time slot and theirdifference
  int count;
  int conflict;
  int iop;
  int resourceNu1, resourceNu2;      // The first and second operation's resource number
  enum operation op1, op2;
  int isPortNode1, isPortNode2; 

  /* Clear conflicts count */
  for (i = 0; i < OP_NU; i++)
    table[i].count = 0;

  cycle = minII;
  myprintf("current minimum II is %d\n", cycle);

  /* Find all the conflicts for each operation */
  for (j = 0; j < NODE_NU; j++) {
    isPortNode1 = DFG[j]->opSrc[0] < 0 ? 1 : 0;
    op1 = DFG[j]->op;
    slot1 = DFG[j]->opScheduledSlot;
    resourceNu1 = DFG[j]->opResourceNu;

    // Check FU conflict
    for (k = j + 1; k < NODE_NU; k++) {
      isPortNode2 = DFG[j]->opSrc[0] < 0 ? 1 : 0;
      slot2 = DFG[k]->opScheduledSlot;
      op2 = DFG[k]->op;
      resourceNu2 = DFG[k]->opResourceNu;
      slot_diff = slot2 - slot1;

      if ((slot_diff != 0) && (slot_diff % cycle == 0) && 
          (op1 == op2) && resourceNu1 == resourceNu2) { 

        // We can't allocate resources for the port conflict.
        if (isPortNode1 && isPortNode2) {
          myprintf("Oops, input ports conflict!\n");
          myprintf("Conflict: Node %3d <- slot %3d <> Node %3d <- slot %3d\n", \
                  j, slot1, k, opSymTable(op2), slot2);
          return -1;
        }

        myprintf("Conflict: Node %3d %s <- slot %3d <> Node %3d %s <- slot %3d : both use resource nu %d\n", \
                j, opSymTable(op1), slot1, \
                k, opSymTable(op2), slot2,
                resourceNu1);

        iop = (int) op1;
        count = table[iop].count;
        table[iop].conflicts[count] = slot_diff;
        table[iop].count = ++count;
      }
    }
  }

  /* Find the actual minimum II for the pipelined data path */
  for (op1 = add; op1 < add + OP_NU; op1++) {
    // printf("op = %d cycle = %d\n", op1, cycle);
    iop = (int) op1;
    if (table[iop].count) {
      while(1) {
        conflict = 0;
        for (j = 0; j < table[iop].count; j++) {
          if ((table[iop].conflicts[j] % cycle) == 0) {
            conflict = 1;
            break;
          }
        }

        /* if conflict equals zero, then there is no more conflicts */
        if (conflict) {
          cycle++;
        } else {
          table[iop].II = cycle;
          //printf("II is %d for operation %d\n", cycle, iop);
          break;
        }
      } 
    }
  }

  myprintf("II is %d for all the operations\n", cycle);

  // We assume there is no resource conflict in the minimum-resource schedule
  #ifdef RC_SCHEDULE
  assert(minII == cycle);
  #endif
  return cycle;
}
