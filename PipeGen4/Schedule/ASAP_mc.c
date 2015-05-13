#include <stdlib.h> 
#include <stdio.h> 
#include "queue.h"
#include "Schedule.h"

//#define DEBUG
int ASAP (int *ASAP_slots, /*int testGen, */ int *Xconstraint, int *Kconstraint) {

  enum operation type, op;
  int i, j, k;
  int src1, src2;
  char useConst;
  char Scheduled;
  char src1Dependency;
  char src2Dependency;
  int curNodeID;
  int isPortNode;
  int portNu;
  int resourceNu;
  int id;

  int scheduledNodeNu = 0;
  int portScheduledNodeNu = 0;
  int idx = 0;
  int slot = 0;
  int scheduledPortNu;


  // Reset scheduling states
  for (i = 0; i < NODE_NU; i++) {
    DFG[i]->opResultDoneSlot = 0;
    DFG[i]->opResultDone = 0;
    DFG[i]->opScheduled = 0;
    DFG[i]->opScheduledSlot = 0;
  }

  while (1) {

    portNu = 0; // add 4/2/11

    if (scheduledNodeNu == NODE_NU && DFG[curNodeID]->opResultDone)
      break;
    
    //--------------------------------------------------------------------------------------
    // ASAP scheduling
    //--------------------------------------------------------------------------------------
    for (i = 0; i < NODE_NU; i++) {

      //----------------------------------------
      // Referece Port_Priority first 1/27/11
      //----------------------------------------
      if (SortedPortPriority == NULL)
        id = DFG[i] -> opSrc[0] < 0 ? i : NodePriority[i];
      else
        id = DFG[i] -> opSrc[0] < 0 ? SortedPortPriority[i] : NodePriority[i];

      type = DFG[id] -> op;
      Scheduled = DFG[id]->opScheduled;
      useConst = DFG[id]->opConst;

      src1 = DFG[id] -> opSrc[0];
      isPortNode = (src1 < 0) ? 1 : 0;
      if (!isPortNode) src2 = DFG[id] -> opSrc[1]; 
      src1Dependency = !(isPortNode || DFG[src1]->opResultDone);
      src2Dependency = !(isPortNode || useConst || DFG[src2]->opResultDone);

      // Enable allocating resource to any node.
      if (!Scheduled && !src1Dependency && !src2Dependency) {
        // Schedule with source availability
        if (ResourceAvailable(type)) {
          if (isPortNode) { 
            /*
            #ifdef PORT_CONSTRAINT
            if (portNu >= Xconstraint[idx] + Kconstraint[idx]) {
              continue;
            #else
              // a little different from the original version as
              // portNu starts from 0
            if (portNu < portNu) {
              portNu++;
            #endif
            } else { 
            #ifdef PORT_CONSTRAINT
              portNu++;
            #else
              continue;
            #endif
            }
            */

            //----------------------------------------
            // last modified 7/6/11
            //----------------------------------------
            if (Xconstraint == NULL && Kconstraint == NULL)
              //scheduledPortNu = (SortedPortPriority == NULL) ? MAX_PORT_NU : 1 ;
              scheduledPortNu = PORT_NU; 
            else
              scheduledPortNu = Xconstraint[idx] + Kconstraint[idx];

            if (portNu >= scheduledPortNu) {
              continue;
            } else { 
              portNu++;
            }
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

              myprintf("ASAP: resource Nu : %d\n", resourceNu);

              DFG[id]->opResourceNu = resourceNu; 
              UpdateRAT(resourceNu - 1, type); 
            } 
            else continue;
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

          curNodeID = id;
          scheduledNodeNu++;

          myprintf("Node %d  type %d number %d has been allocated to slot %d\n", 
                  //id, GetOpSym(type), DFG[id]->opResourceNu, slot);
                  id, GetOpSym(type), DFG[id]->opResourceNu, isPortNode ? slot : slot + 1);
        }
      }
    } // for

    RecycleScheduledResource(slot);
    idx++; portNu = 0;
    slot++;
 }


 for (i = 0; i < NODE_NU; i++) {
   ASAP_slots[i] = DFG[i]->opScheduledSlot;
 }

 myprintf("--------- ASAP Scheduling done -------------------\n");
 myprintf("The number of time slots are %d\n", slot);

 for (i = 0; i < NODE_NU; i++) {
   myprintf("Node %3d <- slot b: %3d e: %3d type %3d resource %3d\n", 
       i, DFG[i]->opScheduledSlot, DFG[i]->opResultDoneSlot, 
       DFG[i]->op, DFG[i]->opResourceNu);
 }

 //PrintScheduledDFG(slot);

 CheckScheduling(slot, NULL, NULL);

 return slot;
}
