#include <stdlib.h> 
#include <stdio.h> 
#include <assert.h> 
#include "queue.h"
#include "Schedule.h"

//#define DEBUG
int ALAP (int *ALAP_slots, int maxInputPortNu) {
  
  enum operation type, op;
  int i, j, k;
  int src1;
  //int src2;
  int dest;
  int destNu;
  char useConst;
  char Scheduled;
  char src1Dependency;
  //char src2Dependency;
  char destDependency;
  int slot;
  int isPortNode;
  //int portScheduledNodeNu = maxInputPortNu;
  int portScheduledNodeNu = 0;
  int resourceNu;
  int done;
  int *portSlots;
  //int *portNodes;
  int portSlot;
  int nodeNu;
  int slot1, slot2;
  int temp;
  int port_nu;
  int id;

  // Reset operator nodes only 4/2/11 
  for (i = 0; i < NODE_NU; i++) {
    if (DFG[i]->opSrc[0] >= 0) {
      DFG[i]->opScheduledSlot = 0;
      DFG[i]->opResultDone = 0;
      DFG[i]->opScheduled = 0;
    }
  }

  //slot = 1;
  slot = 0;
  while (1) {
    done = 1;
    for (i = NODE_NU - 1; i >= 0; i--)
      if (DFG[i]->opSrc[0] >= 0)
        done &= DFG[i]->opResultDone;
    if (done) break;

    for (i = NODE_NU - 1; i >= 0; i--) {

      id = NodePriority[i];
      type = DFG[id] -> op;
      Scheduled = DFG[id]->opScheduled;
      useConst = DFG[id]->opConst;
      destDependency = 0;

      src1 = DFG[id] -> opSrc[0];
      isPortNode = (src1 < 0) ? 1 : 0;
      destNu = DFG[id] -> opDestNu;
      for (j = 0; j < destNu; j++) {
        dest = DFG[id] -> opDest[j];
        destDependency += (dest < 0) ? 0 : (DFG[dest]->opResultDone ? 0 : 1); 
      }

      // Enable allocating resource to any node.
      if (!Scheduled && !destDependency) {

       // Schedule with source availability
        if (ResourceAvailable(type)) {

          if (isPortNode) { 
            // We don't schedule slots for the input ports right now
            continue;
          } 
          else 
          { 
            if ( (*RQ[type])->elemCount > 0) {

              //----------------------------------------
              // 7/9/11
              // Consider registers insertion in front of any FUs,
              // incrementing opScheduledTime
              //----------------------------------------
              resourceNu = GetResourceNu(slot + 1, type);

              DFG[id]->opResourceNu = resourceNu; 

              UpdateRAT(resourceNu - 1, type); 
            } else continue;
          }

          // Resource assignment
          DFG[id]->opScheduled = 1;

          // Time slot assignment
          //----------------------------------------
          // 7/9/11
          // Consider registers insertion in front of any FUs,
          // incrementing opScheduledTime
          //----------------------------------------
          //DFG[id]->opScheduledSlot = slot;
          DFG[id]->opScheduledSlot = isPortNode ? slot : slot + 1;

          #ifdef DEBUG
            printf("Node %d using resource type %d number %d has been allocated to slot %d\n", 
                  //id, GetOpSym(type), DFG[id]->opResourceNu, slot);
                  id, GetOpSym(type), DFG[id]->opResourceNu, isPortNode ? slot : slot + 1);
          #endif
        }
      }
    }

    RecycleScheduledResource(slot);
    slot++;
  }

  portSlots = (int *) malloc (sizeof(int) * maxInputPortNu);
  //portNodes = (int *) malloc (sizeof(int) * maxInputPortNu);

  //----------------------------------------
  // Referece Port_Priority first 1/27/11
  //----------------------------------------
  //for (i = 0; i < maxInputPortNu; i++) portNodes[i] = i;
  //for (i = 0; i < maxInputPortNu; i++) portNodes[i] = SortedPortPriority[i];

  for (i = 0; i < NODE_NU; i++) {
    if (DFG[i]->opSrc[0] < 0) {
      for (j = 0; j < DFG[i]->opDestNu; j++) {
        destNu = DFG[i]->opDest[j];
        portSlot = DFG[destNu]->opResultDoneSlot + DFG[i]->opLatency;
        if (j == 0) portSlots[i] = portSlot;

        // + 3/17/11
        //if (j && (portSlots[i] > portSlot)) portSlots[i] = portSlot;
        if (j && (portSlots[i] < portSlot)) portSlots[i] = portSlot;
      }
    }
  }


  // Sort portSlots from biggest to smallest
  for (i = 1; i < maxInputPortNu; i++) {
    for (j = 0; j < maxInputPortNu - i; j++) {
      slot1 = portSlots[j];
      slot2 = portSlots[j + 1];
      if (slot1 < slot2) {
        temp = portSlots[j];
        portSlots[j] = portSlots[j + 1];
        portSlots[j + 1] = temp;
        /*
        temp = portNodes[j]; 
        portNodes[j] = portNodes[j + 1];
        portNodes[j + 1] = temp;
        */
      }
    }
  }


  //---------------------------------------------------------------------------
  // Reschedule the operations in reverse order
  //---------------------------------------------------------------------------

  int delay = 0;
  int maxDelay = 0;

  // The longest ALAP delay for non-port node
  slot = portSlots[0];// + 1;

  for (i = 0; i < NODE_NU; i++) {

    if (DFG[i]->opSrc[0] >= 0) {

      DFG[i]->opScheduledSlot = 
        slot - DFG[i]->opScheduledSlot - DFG[i]->opLatency; 

      DFG[i]->opResultDoneSlot = 
        slot - DFG[i]->opResultDoneSlot + DFG[i]->opLatency; 
    }
  }

  for (i = 0; i < NODE_NU; i++) {
    //if (DFG[i]->opSrc[0] == SRC) {
    if (DFG[i]->opSrc[0] < 0) {
      for (j = 0; j < DFG[i]->opDestNu; j++) {
        destNu = DFG[i]->opDest[j];
        if (DFG[destNu]->opScheduledSlot <= DFG[i]->opResultDoneSlot) {
          delay = DFG[i]->opResultDoneSlot- DFG[destNu]->opScheduledSlot + 1;
          if (maxDelay < delay)
            maxDelay = delay;
        }
      }
    }
  }

  //printf("max delay = %d\n", maxDelay);

  for (i = 0; i < NODE_NU; i++) {

    if (DFG[i]->opSrc[0] >= 0) {

      DFG[i]->opScheduledSlot += maxDelay + 1;  // register insertion
      DFG[i]->opResultDoneSlot += maxDelay + 1;

      //DFG[i]->opScheduledSlot += maxDelay;
      //DFG[i]->opResultDoneSlot += maxDelay;
    }

    ALAP_slots[i] = DFG[i]->opScheduledSlot;
  }

  slot += maxDelay;

  myprintf("---------------- ALAP Scheduling done -------------------\n");
  for (i = 0; i < NODE_NU; i++) {
    myprintf("Node %3d <- slot b %3d e %3d type %3d resource %3d\n",
        i, DFG[i]->opScheduledSlot, DFG[i]->opResultDoneSlot,
       DFG[i]->op, DFG[i]->opResourceNu);
  }

  CheckScheduling(slot, NULL, NULL);

  free(portSlots);
  //free(portNodes);

  //return (slot);
  return (slot + 1);
}
