#include <stdlib.h> 
#include <stdio.h> 
#include "queue.h"
#include "Schedule.h"
#include "MUX.h"


// Resource-constrained (i.e. a fixed number of FUs) ASAP scheduling
int ASAP (int *ASAP_slots, int *Xconstraint, int *Kconstraint) {

  enum operation type, op;
  int i, j, k;
  int src1, src2;
  char useConst;
  char scheduled;
  char src1Dependency;
  char src2Dependency;
  int curNodeID;
  int isPortNode;
  int portNu;
  int resourceNu;
  int id;
  int mux_delay;

  int scheduledNodeNu = 0;
  int portScheduledNodeNu = 0;
  int idx = 0;
  int slot = 0;
  int scheduledPortNu;


  // Reset scheduling states
  for (i = 0; i < NODE_NU; i++) {
    DFG[i]->opResultDoneSlot = 0;
    DFG[i]->opResultDone = DFG[i]->op == nopc ? 1 : 0;
    DFG[i]->opScheduled = 0;
    DFG[i]->opScheduledSlot = 0;
  }

  CreateScheduleRAT();

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
      //id = DFG[i] -> opSrc[0] < 0 ? PortPriority[i] : NodePriority[i];

      //----------------------------------------
      // modified 5/31/11
      //----------------------------------------
      if (SortedPortPriority == NULL)
        //id = DFG[i] -> opSrc[0] < 0 ? i : NodePriority[i];
        id = DFG[i] -> op == nop ? i : NodePriority[i];
      else
        //id = DFG[i] -> opSrc[0] < 0 ? SortedPortPriority[i] : NodePriority[i];
        id = DFG[i] -> op == nop ? SortedPortPriority[i] : NodePriority[i];
      //----------------------------------------
      // modified 5/31/11
      //----------------------------------------

      //----------------------------------------
      // Get node id info
      //----------------------------------------
      type = DFG[id] -> op;
      scheduled = DFG[id]->opScheduled;
      useConst = DFG[id]->opConst;

      src1 = DFG[id] -> opSrc[0];
      isPortNode = (src1 < 0) ? 1 : 0;

      src1Dependency = !(isPortNode || DFG[src1]->opResultDone);

      if (!isPortNode && type != tod && type != tof) { // 4/12/12
        src2 = DFG[id] -> opSrc[1]; 
      }

      int src3Dependency = 0;
      int src4Dependency = 0;
      int src3, src4;

      if (type == mx || type == mxd) {

        src2Dependency = !(src2 == -1 || DFG[src2]->opResultDone);

        src3 = DFG[id] -> opSrc[2]; 
        src3Dependency = !(src3 == -1 || DFG[src3]->opResultDone);
      }

      else {

        if (type == rom || type == romd) { // Assume a 4-bit address bus
          src3 = DFG[id] -> opSrc[2]; 
          src3Dependency = !(isPortNode || DFG[src3]->opResultDone);
          src4 = DFG[id] -> opSrc[3]; 
          src4Dependency = !(isPortNode || DFG[src4]->opResultDone);
        }

        src2Dependency = (type != tod) && !(isPortNode || useConst || DFG[src2]->opResultDone);
      }

      //----------------------------------------
      // Schedule with source availability
      //----------------------------------------
      if (!scheduled && !src1Dependency && !src2Dependency \
          && !src3Dependency && !src4Dependency) {
        if (isPortNode && type != nopc) {

      //----------------------------------------
      // modified 5/31/11
      // last modified 7/6/11
      //----------------------------------------
          if (Xconstraint == NULL && Kconstraint == NULL)
            //scheduledPortNu = (SortedPortPriority == NULL) ? MAX_PORT_NU : 1 ;
            scheduledPortNu = PORT_NU; 
          else
            scheduledPortNu = Xconstraint[idx] + Kconstraint[idx];

          if (portNu >= scheduledPortNu) {
      //----------------------------------------
      // modified 5/31/11
      //----------------------------------------
            continue;
          } else {
            portNu++;
          }
        }
        else if (type != nopc)
        {
          myprintf("Try inserting Node %d Op %c Slot %d to SRAT\n",
              id, GetOpSym(type), slot + 1);

            resourceNu = InsertScheduleRAT(type, slot + 1);

          if (resourceNu >= 0) {
            DFG[id]->opResourceNu = resourceNu + 1; 
          }
          else 
            continue;
        }

        //----------------------------------------
        // Set node id to have been scheduled
        //----------------------------------------
        DFG[id]->opScheduled = 1;

        //----------------------------------------
        // 7/9/11
        // Consider registers insertion in front of any FUs,
        // and thus increment opScheduledTime 
        //
        // Consider break the mux chains (mux_delay >= 1) 
        //----------------------------------------
        //DFG[id]->opScheduledSlot = slot;
        
#if MUX_OP == 1
        if (type == div_) 
#elif MUX_OP == 2
       if (type == add) 
#elif MUX_OP == 3
       if (type == div_ || type == add) 
#elif MUX_OP == 7
       if (type == div_ || type == add  || type == mul) 
#elif MUX_OP == 14
       if (type == add  || type == mul  || type == sub) 
#elif MUX_OP == 15
       if (type == div_ || type == add  || 
           type == mul  || type == sub) 
#else
       if (0) 
#endif
          mux_delay = 2;
        else
          mux_delay = 1;

        DFG[id]->opScheduledSlot = isPortNode ? slot : slot + mux_delay;

        curNodeID = id;
        scheduledNodeNu++;

        myprintf("scheduled %d nodes ==> Node %d  type %c number %d has been allocated to slot %d\n", 
                //id, GetOpSym(type), DFG[id]->opResourceNu, slot);
                scheduledNodeNu, id, GetOpSym(type), DFG[id]->opResourceNu, isPortNode ? slot : slot + 1);
      }
    } // for

    RecycleScheduledResource(slot);

    idx++; portNu = 0;
    slot++;
 }

 //----------------------------------------
 // For use in List/Iterative scheduling
 //----------------------------------------
 for (i = 0; i < NODE_NU; i++) {
   ASAP_slots[i] = DFG[i]->opScheduledSlot;
 }


 CheckScheduling(slot, NULL, NULL);
 FreeScheduleRAT();

 return slot;
}
