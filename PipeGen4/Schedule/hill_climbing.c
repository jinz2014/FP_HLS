#include "Schedule.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>



#ifdef Test_Iterative_Scheduling

  #define MAX_PORT_NU 0
  int NODE_NU = 11;

  opNode **DFG;

  int DBG = 0;

#endif

#define MAX_COST 1000000

//===========================================================
// Cost described in the textbook
//===========================================================
float Cost(int *ASAP_slots, int *ALAP_slots) {
  int i, j, k;
  float cost = 0;
  float EOC, MaxEOC;
  float p;
  unsigned int start; 
  unsigned int end;
  enum operation op_type;
  //if (DFG[i]->op == op_type && ASAP_slots[i] <= ALAP_slots[i] && !DFG[i]->opScheduled) { 
  
  float *opFuCost = malloc (sizeof(float) * OP_NU);
  opFuCost[0] = 10;
  opFuCost[1] = 5;
  opFuCost[2] = 20;
  opFuCost[3] = 10;

  for (op_type = add; op_type < add + OP_NU; op_type++) {

    // Set [start, end] boundary for operation op_type
    start = 0xffffffff; end = 0;
    for (i = 0; i < NODE_NU; i++) {
      if (DFG[i]->op == op_type && ASAP_slots[i] <= ALAP_slots[i]) {
        if (ASAP_slots[i] < start) start = ASAP_slots[i];
        if (ALAP_slots[i] > end) end = ALAP_slots[i];
      }
    }

    // Max EOC for [start, end]
    if (start != 0xffffffff && end != 0) {
      //if(DBG) printf("op=%d [start=%d end=%d]\n", op_type, start, end);
      MaxEOC = 0;
      for (j = start; j <= end; j++) {  
        EOC = 0;
        for (k = MAX_PORT_NU; k < NODE_NU; k++) {
          if (ASAP_slots[k] <= j && j <= ALAP_slots[k] && DFG[k]->op == op_type) {
            // operator cost for the same operator
            p = (float) 1 / (ALAP_slots[k] - ASAP_slots[k] + 1);
            //if(DBG) printf("P(slot=%d node=%d op=%d) = %f\n", j, k, op_type, p);
            EOC += p; 
          }
        }
        EOC = EOC * opFuCost[op_type];
        if (EOC > MaxEOC) MaxEOC = EOC; 
        //if(DBG) printf("FDS MaxEOC = %f, slot=%d, op=%d\n", MaxEOC, j, op_type);
      }
    }

    // Total EOC cost for all the operations
    cost += MaxEOC;
  }

  //printf("FDS Total cost = %f\n\n", cost);
  return cost;
}

void Schedule_Adjust(int *ASAP_slots, int *ALAP_slots, int nu, int slot) {
  int i, j, k;
  int actual, orig;

  assert(ASAP_slots[nu] <= slot);
  assert(ALAP_slots[nu] >= slot);

  /*
  printf("Debug Mobility\n");
  for (i = MAX_PORT_NU; i < NODE_NU; i++) {
    printf("Node %d <%d %d>\n", i, ASAP_slots[i], ALAP_slots[i]);
  }
  */

  DFG[nu]->opScheduledSlot = slot;
  DFG[nu]->opScheduled = 1;
  //DFG[nu]->opResultDoneSlot = DFG[nu]->opScheduledSlot + DFG[nu]->opLatency;

  //========================================================================
  // adjust itself
  //========================================================================
  ASAP_slots[nu] = ALAP_slots[nu] = slot;

  //========================================================================
  // adjust successors
  //========================================================================
  Update_ASAP_Slots(nu, ASAP_slots, ALAP_slots, 1);


  //========================================================================
  // adjust predecessor
  //========================================================================
  Update_ALAP_Slots(nu, ASAP_slots, ALAP_slots, 1);
}

void Iterative_Scheduling(int *ASAP_slots, int *ALAP_slots)
{
  int i, j, k, slot;
  int count;
  float move_cost, work_cost; 
  float MaxGain, MaxGainIndex;
  float gain_cost;
  List List_p;
  int temp_slot;

  // ===============================================================
  // Randomly select a time step in between the mobility
  // ===============================================================
  srand(time(NULL));

  // ===============================================================
  // initialization begin
  // ===============================================================
  int *current_ASAP_slots = (int *) malloc (sizeof(int) * NODE_NU);
  int *current_ALAP_slots = (int *) malloc (sizeof(int) * NODE_NU);
  int *work_ASAP_slots    = (int *) malloc (sizeof(int) * NODE_NU);
  int *work_ALAP_slots    = (int *) malloc (sizeof(int) * NODE_NU);
  int *move_ASAP_slots    = (int *) malloc (sizeof(int) * NODE_NU);
  int *move_ALAP_slots    = (int *) malloc (sizeof(int) * NODE_NU);

  int *Op    = (int *) malloc (sizeof(int) * (NODE_NU - MAX_PORT_NU));
  int *Step  = (int *) malloc (sizeof(int) * (NODE_NU - MAX_PORT_NU));
  int *Gain  = (int *) malloc (sizeof(int) * (NODE_NU - MAX_PORT_NU));

  // Scheduled slots
  /*
  int *tempSlot  = (int *) malloc (sizeof(int) * NODE_NU);
  int *moveSlot  = (int *) malloc (sizeof(int) * NODE_NU);
  int *origSlot  = (int *) malloc (sizeof(int) * NODE_NU);
  int *bestSlot  = (int *) malloc (sizeof(int) * NODE_NU);
  */

  int *tempSlot  = (int *) calloc (NODE_NU, sizeof(int)) ;
  int *moveSlot  = (int *) calloc (NODE_NU, sizeof(int)) ;
  int *origSlot  = (int *) calloc (NODE_NU, sizeof(int)) ;
  int *bestSlot  = (int *) calloc (NODE_NU, sizeof(int)) ;

  int start, end, mobility;
  int *gain_costs, *gain_costs_index;

  int iter;
  /**********************************/
  /* benchmark-specific             */
  /**********************************/
  int MinIterCost = MAX_COST; 
  int minCost;
  int iter_temp;

  List UnlockOpsList_head = NULL;

  for (i = 0; i < NODE_NU - MAX_PORT_NU; i++) {
    Op[i] = -1; Step[i] = -1;
  }

  // ===============================================================
  // Before iteration save the initial DFG schedule 
  // ===============================================================
  for (i = MAX_PORT_NU; i < NODE_NU; i++) {
    origSlot[i] = DFG[i]->opScheduledSlot;
  }

  // ===============================================================
  // Start Interation
  // ===============================================================

  for (iter = 0; iter < 500; iter++) {

    memcpy(current_ASAP_slots, ASAP_slots, NODE_NU * sizeof(int));
    memcpy(current_ALAP_slots, ALAP_slots, NODE_NU * sizeof(int));

    // restart from initial schedule
    for (i = MAX_PORT_NU; i < NODE_NU; i++) {
      DFG[i]->opScheduledSlot = origSlot[i];
    }

    do {

      // S_work = S_current;
      memcpy(work_ASAP_slots, current_ASAP_slots, NODE_NU * sizeof(int));
      memcpy(work_ALAP_slots, current_ALAP_slots, NODE_NU * sizeof(int));

      // Save the current DFG schedule
      for (i = MAX_PORT_NU; i < NODE_NU; i++) {
        tempSlot[i] = DFG[i]->opScheduledSlot;
      }

      count = 0;

      //======================================================
      // All operations in current work Schedule
      //======================================================
      UnlockOpsList_head = CreateUnlockOpsList
        (UnlockOpsList_head, work_ASAP_slots, work_ALAP_slots);

      while (UnlockOpsList_head != NULL) {
        Gain[count] = -10000000;

        List_p = UnlockOpsList_head;

        //======================================================
        // Find the max cost_gain from each operation i whose 
        // mobility is > 1.
        //======================================================
        while (List_p != NULL) {
          // for each operation i
          i = List_p->nodeID;

          // for each possible destination of operation i
          start = work_ASAP_slots[i];
          end   = work_ALAP_slots[i];
          mobility = end - start + 1;

          gain_costs = (int *) malloc (sizeof(int) * mobility);
          gain_costs_index = (int *) malloc (sizeof(int) * mobility);

          for (j = start; j <= end; j++) {

            // Save inital scheduled slot
            for (k = MAX_PORT_NU; k < NODE_NU; k++) {
              moveSlot[k] = DFG[k]->opScheduledSlot;
            }
            memcpy(move_ASAP_slots, work_ASAP_slots, NODE_NU * sizeof(int));
            memcpy(move_ALAP_slots, work_ALAP_slots, NODE_NU * sizeof(int));
            assert(move_ASAP_slots[i] <= move_ALAP_slots[i]);

            /*
            printf("**************************************\n");
            printf("ITS tries moving node %d from %d to %d\n", 
                //i, work_ASAP_slots[i], j);
                i, DFG[i]->opScheduledSlot, j);
            printf("**************************************\n");
            */

            //======================================================
            // Cost difference
            //======================================================
            //work_cost = Cost(work_ASAP_slots, work_ALAP_slots);
            work_cost = PipeFuCost(work_ASAP_slots, work_ALAP_slots);

            Schedule_Adjust(move_ASAP_slots, move_ALAP_slots, i, j);
            //move_cost = Cost(move_ASAP_slots, move_ALAP_slots);
            move_cost = PipeFuCost(move_ASAP_slots, move_ALAP_slots);

            gain_cost = work_cost - move_cost;

            gain_costs[j - start]       = gain_cost;
            gain_costs_index[j - start] = j;

            // Resume inital scheduled slot
            for (k = MAX_PORT_NU; k < NODE_NU; k++) {
              DFG[k]->opScheduledSlot = moveSlot[k];
            }
          }

          //======================================================
          // Assume some gain_cost values are equal
          // Sort gain decreasingly
          //======================================================
          bubblesort(gain_costs, gain_costs_index, 0, mobility);
          // Get the number of maximum gain cost
          gain_cost = gain_costs[0];
          k = 1;
          while (mobility-- > 1) {
            if (gain_costs[k++] != gain_cost) break;
          }

          assert(k <= end - start + 1);
          //======================================================
          // randomly select a number among k values
          //======================================================
          k = rand() % k;
          j = gain_costs_index[k];

          if (gain_cost > Gain[count]) {
            Op[count]   = i;
            Step[count] = j;
            Gain[count] = gain_cost;
          }

          free(gain_costs_index);
          free(gain_costs);

          List_p = List_p -> next;
        }

        //PrintDFG(work_ASAP_slots, work_ALAP_slots);

        //printf("ITS locks op %d and slot %d count = %d\n", Op[count], Step[count], count); 
        Schedule_Adjust(work_ASAP_slots, work_ALAP_slots, Op[count], Step[count]);

        //PrintDFG(work_ASAP_slots, work_ALAP_slots);

        UnlockOpsList_head = ListDelete(UnlockOpsList_head, Op[count]);
        //printf("Print UnlockOPList after deletion of node %d\n", Op[count]);
        //ListPrint(UnlockOpsList_head);

        count++;
      }

      MaxGain      = Max_Cum_Gain(Gain, count);
      MaxGainIndex = Max_Cum_Gain_Index(Gain, count);

      // Resume the current schedule
      for (i = MAX_PORT_NU; i < NODE_NU; i++) {
        DFG[i]->opScheduledSlot = tempSlot[i];
      }

      // Change the current schedule if MaxGain >= 0
      if (MaxGain >= 0) {
        for (i = MAX_PORT_NU; i < NODE_NU; i++) {
          DFG[i]->opScheduled = 0;
        }
        for (j = 0; j <= MaxGainIndex; j++) {
          //printf("ITS selects op %d and slot %d\n", Op[j], Step[j]); 
          Schedule_Adjust(current_ASAP_slots, current_ALAP_slots, Op[j], Step[j]);
        }
      }

      // All the unscheduled nodes have mobility of 1 ?
      for (i = MAX_PORT_NU; i < NODE_NU; i++) {
        if (DFG[i]->opScheduled == 0) {
          //printf("Unscheduled node %d <%d %d>\n", i, current_ASAP_slots[i], current_ALAP_slots[i]);
          assert (current_ASAP_slots[i] <= current_ALAP_slots[i]);

          if (current_ASAP_slots[i] == current_ALAP_slots[i]) {
            DFG[i]->opScheduledSlot = current_ASAP_slots[i];
          } else {

            minCost = MAX_COST; 
            for (k = current_ASAP_slots[i]; k <= current_ALAP_slots[i]; k++) {
              DFG[i]->opScheduledSlot = k;
              if (minCost > PipeFuCost(current_ASAP_slots, current_ALAP_slots)) {
                minCost = PipeFuCost(current_ASAP_slots, current_ALAP_slots);
                MaxGainIndex = k;
              }
            }
            DFG[i]->opScheduledSlot = MaxGainIndex;
          }
        }
      }

      //PrintDFG(ASAP_slots, ALAP_slots);
      CheckScheduling(0, ASAP_slots, ALAP_slots);

    } while (MaxGain >= 0);

    //myprintf("locally optimal schedule\n");
    //PrintDFG(ASAP_slots, ALAP_slots);

    // Save the locally optimal schedule
    if (PipeFuCost(ASAP_slots, ALAP_slots) < MinIterCost) {
      iter_temp = iter; 
      MinIterCost = PipeFuCost(ASAP_slots, ALAP_slots);
      for (i = MAX_PORT_NU; i < NODE_NU; i++) {
        bestSlot[i] = DFG[i]->opScheduledSlot;
      }
    }
  } // for N iterations

  //printf("--------- Interative Scheduling done after 500 iters-------------------\n");

  myprintf ("Best DFG at iteration %d\n", iter_temp);
  for (i = MAX_PORT_NU; i < NODE_NU; i++) {
    DFG[i]->opScheduledSlot = bestSlot[i];
    DFG[i]->opResultDoneSlot = bestSlot[i] + DFG[i]->opLatency;
  }

  //PrintDFG(ASAP_slots, ALAP_slots);

  PipeFuCost(ASAP_slots, ALAP_slots);

  free(current_ASAP_slots);
  free(current_ALAP_slots);
  free(work_ASAP_slots);
  free(work_ALAP_slots);
  free(move_ASAP_slots);
  free(move_ALAP_slots);
  free(Op);
  free(Step);
  free(Gain);
  free(tempSlot);
  free(moveSlot);
  free(bestSlot);
  free(origSlot);
} 

List CreateUnlockOpsList(List UnlockOpsList_head, int *ASAP_slots, int *ALAP_slots) {
  int i;
  for (i = MAX_PORT_NU; i < NODE_NU; i++) {
    if (ASAP_slots[i] != ALAP_slots[i]) {
      UnlockOpsList_head = ListInsert(UnlockOpsList_head, i, DFG[i]->opPriority, LENGTH);
    }
  }
  //printf("After UnlockOpsList creation:\n");
  //ListPrint(UnlockOpsList_head);
  return UnlockOpsList_head;
}

int Max_Cum_Gain(int Gain[], int count) {
  int i, j;
  int Cum_Gain;
  int Max = -10000000;

  for (i = 0; i < count; i++) {
    Cum_Gain = 0;
    for (j = 0; j <= i; j++)
      Cum_Gain += Gain[j];
    if (Cum_Gain >= Max) {
      Max = Cum_Gain; 
    }
  }
  return Max;
}

int Max_Cum_Gain_Index(int Gain[], int count) {
  int i, j;
  int Cum_Gain;
  int MaxIndex = -1;
  int Max = -10000000;

  for (i = 0; i < count; i++) {
    Cum_Gain = 0;
    for (j = 0; j <= i; j++)
      Cum_Gain += Gain[j];
    if (Cum_Gain >= Max) {
      Max = Cum_Gain; 
      MaxIndex = i;
    }
  }
  return MaxIndex;
}

//===========================
#ifdef Test_Iterative_Scheduling
//===========================
int main() {

  int i;

  // HAL example p218 High-Level Synthesis
  int ASAP_slots[11] = {0, 0, 0, 0, 1, 1, 2, 3, 1, 0, 1};
  int ALAP_slots[11] = {0, 0, 1, 2, 1, 2, 2, 3, 3, 2, 3};

  DFG = (opNode **) malloc(sizeof(opNode *) * NODE_NU);
  for (i = 0; i < NODE_NU; i++) {
    DFG[i] = (opNode *) malloc(sizeof(opNode));
    DFG[i]->opLatency = 0;
  }

  #include "HAL.txt"

  //-------------------------------
  // ASAP as initial scheduled DFG
   DFG[0]->opScheduledSlot = 0;
   DFG[1]->opScheduledSlot = 0;
   DFG[2]->opScheduledSlot = 0;
   DFG[3]->opScheduledSlot = 0;
   DFG[4]->opScheduledSlot = 1;
   DFG[5]->opScheduledSlot = 1;
   DFG[6]->opScheduledSlot = 2;
   DFG[7]->opScheduledSlot = 3;
   DFG[8]->opScheduledSlot = 1;
   DFG[9]->opScheduledSlot = 0;
   DFG[10]->opScheduledSlot = 1;

  for (i = 0; i < NODE_NU; i++) {
    DFG[i]->opPriority = i;
    DFG[i]->opScheduled = 1;
    DFG[i]->opResultDoneSlot = DFG[i]->opScheduledSlot + DFG[i]->opLatency;
  }
  //-------------------------------

  Iterative_Scheduling(ASAP_slots, ALAP_slots);

  free(DFG);

  return 0;
}
#endif

