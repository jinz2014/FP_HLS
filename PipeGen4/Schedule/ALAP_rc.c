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
  int isPortNode;
  int offset;
  int resourceNu;
  int done;
  int *portSlots;
  // int *portNodes;
  int portSlot;
  int nodeNu;
  int slot1, slot2;
  int temp;
  int port_nu;
  int id;
  int slot = 0;
  int portScheduledNodeNu = 0;

  //--------------------------------------------------
  // Reset operator nodes only 4/2/11 
  //
  // So we use ASAP and ALAP in pair
  //--------------------------------------------------
  for (i = 0; i < NODE_NU; i++) {
    if (DFG[i]->opSrc[0] >= 0) {
      DFG[i]->opScheduledSlot = 0;
      DFG[i]->opResultDone = 0;
      DFG[i]->opScheduled = 0;
    }
  }

  CreateScheduleRAT();

  slot = 0;

  while (1) {
    done = 1;
    for (i = NODE_NU - 1; i >= 0; i--)
      if (DFG[i]->opSrc[0] >= 0)
        done &= DFG[i]->opResultDone;
    if (done) break;

    //-----------------------------------------------------
    // Assumption: 
    // We start scheduling from the node whose destination 
    // is SINK and whose id is NODE_NU - 1
    //-----------------------------------------------------
    for (i = NODE_NU - 1; i >= 0; i--) {

      //----------------------------------------
      // Referece Node_Priority first
      //----------------------------------------
      id = NodePriority[i];

      //----------------------------------------
      // Get node id info
      //----------------------------------------
      type = DFG[id] -> op;
      Scheduled = DFG[id]->opScheduled;
      useConst = DFG[id]->opConst;
      destDependency = 0;

      src1 = DFG[id] -> opSrc[0];
      isPortNode = (src1 < 0) ? 1 : 0;
      destNu = DFG[id] -> opDestNu;

      //----------------------------------------
      // Result done for all the destinations of the node id
      //----------------------------------------
      for (j = 0; j < destNu; j++) {
        dest = DFG[id] -> opDest[j];
        destDependency += (dest < 0) ? 0 : (DFG[dest]->opResultDone ? 0 : 1); 
      }

      // Enable allocating resource to any node.
      if (!Scheduled && !destDependency) {

        // We don't schedule slots for the input ports right now
        if (isPortNode) { 
          continue;
        } 
        else 
        { 
          //resourceNu = InsertScheduleRAT(type, slot);
          resourceNu = InsertScheduleRAT(type, slot + 1);  // register insertion
          if (resourceNu >= 0) {
            DFG[id]->opResourceNu = resourceNu + 1; 
          }
          else 
            continue;
        }

      //----------------------------------------
      // node id has been scheduled
      //----------------------------------------
        DFG[id]->opScheduled = 1;
        //DFG[id]->opScheduledSlot = slot;
        DFG[id]->opScheduledSlot = isPortNode ? slot : slot + 1; // register insertion
        myprintf("Node %d using resource type %d number %d has been allocated to slot %d\n", 
                 //id, type, DFG[id]->opResourceNu, slot);
                 id, GetOpSym(type), DFG[id]->opResourceNu, isPortNode ? slot : slot + 1);
      }
    }

    RecycleScheduledResource(slot);
    slot++;
  }

  
  /* Print non-port Nodes
  for (i = 0; i < NODE_NU; i++) {
    if (DFG[i]->opSrc[0] >= 0)
      printf("Node %3d <- slot b %3d e %3d\n", 
          i, DFG[i]->opScheduledSlot, DFG[i]->opResultDoneSlot);
  }
  */

  portSlots = (int *) malloc (sizeof(int) * maxInputPortNu);
  //portNodes = (int *) malloc (sizeof(int) * maxInputPortNu);

  //----------------------------------------
  // Referece Port_Priority first 1/27/11
  //----------------------------------------
  // for (i = 0; i < maxInputPortNu; i++) portNodes[i] = SortedPortPriority[i];

  for (i = 0; i < NODE_NU; i++) {
    if (DFG[i]->opSrc[0] < 0) {
      for (j = 0; j < DFG[i]->opDestNu; j++) {
        destNu = DFG[i]->opDest[j];
        portSlot = DFG[destNu]->opResultDoneSlot + DFG[i]->opLatency;
        if (j == 0) portSlots[i] = portSlot;

        //  Get the longest port delay 3/17/11
        if (j && (portSlots[i] < portSlot)) portSlots[i] = portSlot;
      }
    }
  }

  //---------------------------------------------------------------------------
  // Reschedule the operations in reverse order
  //---------------------------------------------------------------------------
  int maxPortSlot = 0;
  int delay = 0;
  int maxDelay = 0;

  // The longest ALAP delay for non-port node
  for (i = 1; i < maxInputPortNu; i++) {
    if (portSlots[i] > maxPortSlot) {
      maxPortSlot = portSlots[i];
    }
  }

  slot = maxPortSlot;

  for (i = 0; i < NODE_NU; i++) {

    if (DFG[i]->opSrc[0] >= 0) {

      DFG[i]->opScheduledSlot = 
        slot - DFG[i]->opScheduledSlot - DFG[i]->opLatency; 

      DFG[i]->opResultDoneSlot = 
        slot - DFG[i]->opResultDoneSlot + DFG[i]->opLatency; 
    }
  }

  for (i = 0; i < NODE_NU; i++) {
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

  for (i = 0; i < NODE_NU; i++) {

    if (DFG[i]->opSrc[0] >= 0) {

      DFG[i]->opScheduledSlot += maxDelay + 1;  // register insertion

      DFG[i]->opResultDoneSlot += maxDelay + 1;
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
  FreeScheduleRAT();
  free(portSlots);
  //free(portNodes);

  //return (slot);
  return (slot + 1);
}
