//=======================================================================
// 3/5/12   add comments to the codes in function InsertScheduleRAT
//=======================================================================
#include "queue.h"
#include "Schedule.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

void SetMinII(int maxInputPortNu) {

  int i;

  // global
  minII = maxInputPortNu / PORT_NU + ((maxInputPortNu % PORT_NU) ? 1 : 0); // add 12/19


  //printf("1: minimum II is %d\n", minII);

}

// Get number of physical FUs of type op_type
int PipeFuNu(enum operation op_type) {

  int maxFuNu;
  int cost = 0;
  int i, j;

  // At most minII slots
  int *FuNu = (int *) malloc(sizeof(int) * minII);

  maxFuNu = -10000000;

  for (i = 0; i < minII; i++) FuNu[i] = 0;

  for (i = 0; i < NODE_NU; i++) {
    if (DFG[i]->op == op_type) {
      j = DFG[i]->opScheduledSlot % minII;
      FuNu[j]++;
    }
  }

  for (i = 0; i < minII; i++) 
    if (FuNu[i] > maxFuNu)
      maxFuNu = FuNu[i];

  assert(maxFuNu <= 34);
  assert(maxFuNu != -10000000);

  //printf("Max operator number of %c is %d\n", GetOpSym(op_type), maxFuNu);
  //printf("%d ", maxFuNu);

  free(FuNu);
  return maxFuNu;
}

int PipeFuCost(int *ASAP_slots, int *ALAP_slots) {

  int fuNu;
  //int sumMaxFuNu = 0;
  int cost = 0;
  int i, j;
  enum operation op_type;

  int *opFuCost = malloc (sizeof(int) * OP_NU);

  opFuCost[add] = 516; 
  /*
  // cost is Virtex5 LX330 logic resources usage
  switch (OP_NU) {
    case 1 : 
      opFuCost[add] = 516; 
      break;
    case 2 : 
      opFuCost[add] = 516;
      opFuCost[mul] = 264;
      break;
    case 3 : 
      opFuCost[add] = 516;
      opFuCost[mul] = 264;
      opFuCost[div_] = 731;
      break;
    case 4 : 
      opFuCost[add] = 516;
      opFuCost[mul] = 264;
      opFuCost[div_] = 731;
      opFuCost[sub] = 516;
      break;
    default:
      printf("OP_NU is in the range [1, 4]\n");
      exit(1);
  }
  */

  for (op_type = add; op_type < add + OP_NU; op_type++) {
    fuNu = PipeFuNu(op_type); 
    cost += fuNu * opFuCost[op_type];
    //cost += fuNu;
  }

  free(opFuCost);
  return cost;
}

void PipeFuAlloc() {

  int i, j;
  int maxFuNu;
  int nodeID;
  enum operation op_type;
  List *Lists;
  List head;

  //printf("PipeFuAlloc:\n");
  int *FuNu = (int *) malloc(sizeof(int) * minII);
  Lists = (List *) malloc (sizeof(List) * minII);

  for (op_type = add; op_type < add + OP_NU; op_type++) {

    for (i = 0; i < minII; i++) Lists[i] = NULL;
    for (i = 0; i < minII; i++) FuNu[i] = 0;
    maxFuNu = -10000000;

    for (i = 0; i < NODE_NU; i++) {
      if (DFG[i]->op == op_type) {
        j = DFG[i]->opScheduledSlot % minII;
        // Each list keeps the conflicting FUs of op_type
        Lists[j] = ListInsert(Lists[j], i, 0, NONE);
        FuNu[j]++;
      }
    }

    // Get max number of FUs of op_type
    for (i = 0; i < minII; i++) 
      if (FuNu[i] > maxFuNu) maxFuNu = FuNu[i]; 

    // We use queue for back-end generation
    UpdateQueue(RQ[op_type], maxFuNu);

    /* Debug
    for (i = 0; i < minII; i++) {
      printf("hole %d\n", i);
      ListPrint(Lists[i]);
      printf("\n");
    }
    */

    // Assign FU resource number to operation of op_type
    for (i = 1; i <= maxFuNu; i++) {
      for (j = 0; j < minII; j++) {
        if (Lists[j] != NULL) {
          nodeID = Lists[j]->nodeID;
          DFG[nodeID]->opResourceNu = i;
          Lists[j] = ListDelete(Lists[j], nodeID);
        }
      }

      /* Debug
      printf("After List delete:\n");
      for (j = 0; j < minII; j++) {
        ListPrint(Lists[j]);
        printf("\n");
      }
      */
    }
  } // for (op_type = add; ...)

  CheckConflict(); // check pipelined FUs 
  free(FuNu);
  free(Lists);
}

//#define RESOURCE_DEBUG
//--------------------------------------------------------------------
// The function can simply return the result from Dequeue(RQ[type]).
// However, by checking the history of the current operations' resource 
// allocation, less resources may be needed to meet the minII.
//--------------------------------------------------------------------
int GetResourceNu(int slot, int type) {

  int i, j, k;
  int size;
  int resourceNu;
  int offset;
  int *conflictVector;
  int minSharedCount;
  int begin, end;

  assert(RQ[type] != NULL);
  assert(RAT[type] != NULL);

  #ifdef RESOURCE_DEBUG 
  printf("slot = %d type = %d\n", slot, type);
  DumpRAT(); 
  #endif

  // We have no choice but to allocate the only FU
  size = (*RQ[type])->size;
  if (size == 1) return Dequeue(RQ[type]);

  // A conflict vector indicates which resources have conflict 
  // with the one at the current slot for the same operation, 
  conflictVector = (int *) calloc (size, sizeof(int));

  for (i = 0; i < NODE_NU; i++) {
    // Non-port nodes only
    if (DFG[i]->opSrc[0] < 0) continue;

    if (DFG[i]->opScheduled && DFG[i]->op == type) {

      offset = 0; //(PQ[type] == NULL) ? 0 : (*PQ[type])->size;
      resourceNu = DFG[i]->opResourceNu - offset - 1;
      assert(resourceNu < size);

      #ifdef RESOURCE_DEBUG
      printf("GetResourceNu: Node %d offset %d resource Nu : %d\n", i, offset, resourceNu);
      #endif

      if (slot != DFG[i]->opScheduledSlot && 
          !((slot - DFG[i]->opScheduledSlot) % minII))
        conflictVector[resourceNu] |= 1;

    }
  }
      #ifdef RESOURCE_DEBUG
      printf("Conflict vector: ");
      for (i = 0; i < size; i++)
      printf("%d ",  conflictVector[i]);
      printf("\n");
      #endif

  // Get the queue's begin and end indicators
  begin = (*RQ[type])->begin;
  end = (*RQ[type])->end;

  // Check if the resources are available in the queue
  for (i = 0; i < size; i++) {

    if (!conflictVector[i]) {
      /* for (j = begin; j < end ; (++j) % size) 
        if (i + 1 == (*RQ[type])->q[j]) 
          break; */
      j = begin;
      do {
        if (i + 1 == (*RQ[type])->q[j]) break;
        j = (++j) % size;
      } while (j != end);

      if (begin != end && j == end)
        conflictVector[i] = 1;
    }    
  }

  #ifdef RESOURCE_DEBUG
  printf("Conflict vector: ");
  for (i = 0; i < size; i++)
  printf("%d ",  conflictVector[i]);
  printf("\n");
  #endif

  // Get total number of operations of type, say, add in DFG.
  minSharedCount = CountOpNode(type);

  // Find the resource number with minimum shared usage count
  resourceNu = -1;
  for (i = 0; i < size; i++) {
    if (!conflictVector[i]) {
      if (minSharedCount > RAT[type][i]) {
        minSharedCount = RAT[type][i];
        //printf("minShareCount: %d\n", minSharedCount); 
        resourceNu = i;
      }
    }
  }

  #ifdef RESOURCE_DEBUG
  printf("resourceNu = %d\n", resourceNu);
  #endif

  // we have no choice but to allocate a conflicting resource number.
  if (resourceNu == -1) {
    free(conflictVector);
    //printf("free..\n");
    return Dequeue(RQ[type]);
  }

  /*---------------------------------------------------------------
   Allocate to the current slot the minimum shared resource number
  
  for (j = begin; j < end; (++j) % size) {
    if (resourceNu + 1 == (*RQ[type])->q[j]) {
      for (k = begin; k < j; (++k) % size)
        Enqueue(RQ[type], Dequeue(RQ[type]));

      free(conflictVector);
      printf("free..\n");
      return Dequeue(RQ[type]);
    }
  }
  ---------------------------------------------------------------*/
      j = begin;
      do {
        if (resourceNu + 1 == (*RQ[type])->q[j]) {
          k = begin;
          while (k != j) {
            Enqueue(RQ[type], Dequeue(RQ[type]));
            k = (++k) % size;
          } 
          free(conflictVector);
          //printf("free..\n");
          return Dequeue(RQ[type]);
        }
        j = (++j) % size;
      } while (j != end);

  //return Dequeue(RQ[type]);
}

/* Add one more FU of type */
int Reschedule(int cycle) {
 int i;
 int size;

 if (cycle < 0) {
   printf("Cannot reschedule the operations!\n");
   return 0;
 }

 if (cycle > minII) {
   for (i = 0; i < OP_NU; i++) {
     if (table[i].count) {
       size = (*RQ[i])->size + 1;
       UpdateQueue(RQ[i], size); 
       //printf("op_type %d: resource %d\n", i, (*RQ[i])->size);
       ResetRAT();
     }
   }

   myprintf("Reschedule the pipelined data path with more FUs\n");
   myprintf("------------------------------------------------\n");
   return 1;
 }
 return 0;
}

void FreeDFG() {
  int i;
  for (i = 0; i < NODE_NU; i++) {
    free(DFG[i]->opSrc);
    free(DFG[i]->opDest);

    if (DFG[i]->opMuxVal != NULL)
      free(DFG[i]->opMuxVal);

    if (DFG[i]->opMuxVal_d != NULL)
      free(DFG[i]->opMuxVal_d);

    if (DFG[i]->opName != NULL)
      free(DFG[i]->opName);

    free(DFG[i]);
  }
  free(DFG);
}

void FreeSlots() {
  free(ASAP_slots);
  free(ALAP_slots);
}

void FreeQueue() {
  enum operation op_type;

  for (op_type = add; op_type < add + OP_NU; op_type++) {
    DestroyQueue(RQ[op_type]);
    free(RQ[op_type]);
    free(RAT[op_type]);
  }

  free(RQ);
  free(RAT);
}

void FreeAll() {
  FreeDFG();
  FreeSlots();
  FreeQueue();
}

int ResourceAvailable(int type) {
  int result;
  if (type == nop) return 1;
  /*
  if (PQ[type] == NULL)
    result = (*RQ[type])->elemCount;
   */
  else
    result = (*RQ[type])->elemCount; // + (*PQ[type])->elemCount;
  return result;
}

void CreateResource() {

  enum operation op_type;
  int fuNu;

  RQ = (queue **) malloc(sizeof(queue *) * OP_NU);
  for (op_type = add; op_type < add + OP_NU; op_type++)
    RQ[op_type] = (queue *) malloc (sizeof(queue *)); 

  for (op_type = add; op_type < add + OP_NU; op_type++) {
#ifdef RC_SCHEDULE
    // 8/1/2011
    fuNu = (GetOpSym(op_type) == 'm' || GetOpSym(op_type) == 'r') ?
            FU[op_type] : myceil(FU[op_type], minII);
    CreateQueue(RQ[op_type], fuNu);
#endif

#ifdef MC_SCHEDULE
    CreateQueue(RQ[op_type], 1);
#endif
  }
}

//---------------------------------------------------------
// RAT
//---------------------------------------------------------

void CreateRAT() {
  enum operation op_type;
  RAT = (int **) malloc(sizeof(int *) * OP_NU);
  for (op_type = add; op_type < add + OP_NU; op_type++) {
    RAT[op_type] = (int *) calloc (myceil(FU[op_type], minII), sizeof(int));
  }
}


void ResetRAT() {
  enum operation op_type;
  int size;
  int i;
  for (op_type = add; op_type < add + OP_NU; op_type++) {
    size = (*RQ[op_type])->size;
    RAT[op_type] = (int *) realloc(RAT[op_type], sizeof(int) * size);
    for (i = 0; i < size; i++)
      RAT[op_type][i] = 0;
  }
}

void UpdateRAT(int nu, int op_type) {
  RAT[op_type][nu]++;
}

void DumpRAT() {
  enum operation op_type;
  int size;
  int i;
  for (op_type = add; op_type < add + OP_NU; op_type++) {
    printf("DumpRAT: op type %d  ", op_type);
    size = (*RQ[op_type])->size;
    for (i = 0; i < size; i++)
      printf("%d ", RAT[op_type][i]);
    printf("\n");
  }
}

void PrintVarLifeTime() {
  int i, j;
  int src;
  printf("--------- Variables' life time --------\n");
  for (j = 0; j < NODE_NU; j++) {
    if (!(DFG[j]->opSrc[0] < 0)) {
      for (i = 0; i < 2 - DFG[j]->opConst; i++) {
        src = DFG[j]->opSrc[i];
        printf("Var%3d  ", src);
        printf("%3d", DFG[src]->opResultDoneSlot);
        printf(" --- ");
        printf("%3d\n", DFG[j]->opScheduledSlot);
      }
    }
  }
  printf("---------------------------------------\n");
}
      

void PrintScheduledDFG(int slot) {
  int i, j;
  printf("---------------------------------------\n");
  for (i = 1; i < slot; i++) {
    printf("Slot %3d:\n", i);
    for (j = 0; j < NODE_NU; j++) {
      if (DFG[j]->opScheduledSlot == i) 
        printf("b Node %3d type %3d resource %3d\n", 
            j, DFG[j]->op, DFG[j]->opResourceNu);

      if (DFG[j]->opResultDoneSlot == i)
        printf("e Node %3d type %3d resource %3d\n", 
            j, DFG[j]->op, DFG[j]->opResourceNu);
    }
    printf("---------------------------------------\n");
  }
}

/* 
 * If the non-port operation is in the set of port operations,
 * then bind non-port resource to port resources. Otherwise,
 * initialize the non-port resource with the initial non-port
 * FUs 
 */
void ResetResource() {
  enum operation op_type;
  for (op_type = add; op_type < add + OP_NU; op_type++) {
#ifdef MC_SCHEDULE
    UpdateQueue(RQ[op_type], 1);
#endif
#ifdef RC_SCHEDULE
    UpdateQueue(RQ[op_type], myceil(FU[op_type], minII));
#endif
  }
}

int CountResource() {
  enum operation op_type;
  int sum  = 0;

  for (op_type = add; op_type < add + OP_NU; op_type++) {
    //if (PQ[op_type] != NULL) 
    //  sum += (*PQ[op_type])->size;
    sum += (*RQ[op_type])->size; 
  }

  return sum;
}

int CountOpNode(int type) {
  int i;
  int opNodeNu = 0;
  for (i = 0; i < NODE_NU; i++)
    if (DFG[i]->op == type)
      opNodeNu++;
  return opNodeNu;
}

void RecycleScheduledResource(int slot) {
  int i;
  int src1;
  int offset;
  enum operation type;

  for (i = 0; i < NODE_NU; i++) {
    if (DFG[i]->opScheduled && !DFG[i]->opResultDone &&
        (slot == DFG[i]->opScheduledSlot + DFG[i]->opRate - 1)) {

      src1 = DFG[i] -> opSrc[0];

      if (src1 < 0)
        ;//Enqueue(PQ[DFG[i]->op], DFG[i]->opResourceNu);
      else {
        type = DFG[i] -> op;
        offset = 0;
        Enqueue(RQ[type], DFG[i]->opResourceNu - offset);
      }

      myprintf("resource type %d is reclaimed at slot %d\n",
              DFG[i]->op, slot);
    }

    // Check when operation is done
    if (DFG[i]->opScheduled && !DFG[i]->opResultDone &&
        // opLatency cycles after opScheduledSlot (i.e. an active OPERATION_ND)
        (slot == DFG[i]->opScheduledSlot + DFG[i]->opLatency /*- 1*/)) {
      DFG[i]->opResultDone = 1;
      DFG[i]->opResultDoneSlot = slot;

      myprintf("Node %d operation is done at slot %d\n", i, slot);
      myprintf("%d %d\n", DFG[i]->opScheduledSlot, DFG[i]->opLatency); 
    }
  }
}

void CreateDFG() {
  int i, j;
  int srcNu, destNu;

  myprintf("Total node num = %d\n", NODE_NU);
  DFG = (opNode **) malloc(sizeof(opNode *) * NODE_NU);
  for (i = 0; i < NODE_NU; i++) {
    DFG[i] = (opNode *) malloc(sizeof(opNode));
    DFG[i]->opPrecision = sfp;
    DFG[i]->opVal = NULL;
    DFG[i]->opVal_d = NULL;
    DFG[i]->opMuxVal = NULL;
    DFG[i]->opMuxVal_d = NULL;
    DFG[i]->RegFileTable = NULL;
    DFG[i]->opConstVal = 0;
    DFG[i]->opConstVal_d = 0;
    DFG[i]->opConst = 0;
    DFG[i]->opSrcNu = 0;
    DFG[i]->opName = NULL;
  }

  ASAP_slots = (int *) calloc(NODE_NU, sizeof(int));
  ALAP_slots = (int *) calloc(NODE_NU, sizeof(int));

#include "Benchmarks.dfg"

  CheckDFG(); 

  for (i = 0; i < NODE_NU; i++) {
    if (DFG[i]->opSrc[0] < 0)
      myprintf("i: %d type: %s resource Nu: %d src0: %d dest: %d\n", 
            i, opSymTable(DFG[i]->op), DFG[i]->opResourceNu, 
            DFG[i]->opSrc[0], DFG[i]->opDest[0]);
    else {
      myprintf("i: %d type: %s resource Nu: %d ", 
          i, opSymTable(DFG[i]->op), DFG[i]->opResourceNu);

      srcNu = GetNodeSrcNu(DFG[i]->op);

      for (j = 0; j < srcNu; j++) {
        myprintf("src%3d: %3d ", j, DFG[i]->opSrc[j]);
      }

      destNu = DFG[i]->opDestNu;
      for (j = 0; j < destNu; j++) {
        myprintf("dest%3d: %3d ", j, DFG[i]->opDest[j]);
      }

      myprintf("\n");
    }
  }

  //----------------------------------------------------
  // Initialize opSrcNu if they are not initialized in
  // the benchmark configuration files.
  //----------------------------------------------------
  for (i = 0; i < NODE_NU; i++) {
    switch (DFG[i]->op) {
     // port node
      case nop  : 
      case nopc : 
      case tod  : 
      case tof  : if (!DFG[i]->opSrcNu) 
                    DFG[i]->opSrcNu = 1; 
                  else
                    assert(DFG[i]->opSrcNu == 1); 
                  break;

      case add  : 
      case mul  : 
      case div_ : 
      case sub  : 
      case max  : 
      case lt   : 
      case gt   : 
      case addd : 
      case muld :
      case divd : 
      case subd : 
      case maxd :
      case ltd  :
      case gtd  : if (!DFG[i]->opSrcNu)
                    DFG[i]->opSrcNu = 2;
                  else
                    assert(DFG[i]->opSrcNu == 2);
                  break;
      case mx   : 
      case mxd  : if (!DFG[i]->opSrcNu)
                    DFG[i]->opSrcNu = 3; 
                  else
                    assert(DFG[i]->opSrcNu == 3);
                  break;
      case rom  :
      case romd : if (!DFG[i]->opSrcNu)
                    DFG[i]->opSrcNu = 4; 
                  else
                    assert(DFG[i]->opSrcNu == 4);
                  break;

    }
  }

  //----------------------------------------------------
  // Initialize pipeline rate and altency of all the FUs
  //----------------------------------------------------
  for (i = 0; i < NODE_NU; i++) {
    if (DFG[i]->opSrc[0] < 0) {
      DFG[i]->opRate = 0;       // set port node latency to 0
      DFG[i]->opLatency = 0;
      DFG[i]->opPriority = i;   // Priority based on the topological order by default 
      DFG[i]->opResourceNu = i; // initialize virtual port resource(1/5/11)

      if (DFG[i]->op == nopc) {
        // app-specific 
        // app: mrbays
        //                  Norm  IIA IIC IIG IIT
        // Constant Value    i    i+1 i+2 i+3 i+4 (i = 0)
        // These input ports are not scheduled as 
        // port nodes and thus treated as if they
        // are FU input constant. I think the constant
        // values could differentiate the port
        // names in a raw manner.
        DFG[i]->opResourceNu = DFG[i]->opConstVal; 
      }
    }
    else {
      DFG[i]->opRate = RATE[(int) DFG[i]->op];
      DFG[i]->opLatency = LATENCY[(int) DFG[i]->op];
    }
  }
  /*
  for (i = 0; i < NODE_NU; i++) {
    if (DFG[i]->opSrc[0] < 0) {
      DFG[i]->opRate = 1;
      DFG[i]->opLatency = 1;
    }
    else {
      DFG[i]->opRate = RATE[(int) DFG[i]->op];
      DFG[i]->opLatency = LATENCY[(int) DFG[i]->op];
    }
  }
  */
}

void CheckDFG() {
  int i, j, k;
  int destNu, srcNu;
  int ic, error;

  assert(DFG[NODE_NU-1]->opDest[0] == SINK);

  error = 0;
  for (i = 0; i < NODE_NU; i++) {
    for (j = 0; j < DFG[i]->opDestNu; j++) {
      ic = 0;
      destNu = DFG[i]->opDest[j]; 
      myprintf("At node %d - op = %s whose dest num is %d \n", i,
          opSymTable(DFG[i]->op), destNu);
      if (destNu != SINK) {
        assert(destNu > i);
        srcNu = GetNodeSrcNu(DFG[destNu]->op);
        for (k = 0; k < srcNu; k++) {
          if (DFG[destNu]->opSrc[k] == i) ic = 1;
        }
        if (!ic) {
          printf("Error: node %d -- node %d are not connected right\n", i, destNu);
          error++;
        }
      }
    }
  }
  if (error) {
    printf("Error: some nodes are not connected right\n");
    exit(1);
  }
}

/*
 * S(n) maps an operation node to a time slot
 * delay(n) is the latency of the operation node n
 *
 * 1. All the time slots are larger than 0 (discard)
 * 2. If two operations (x, y) have dependence, then S(x) + delay(x) <= S(y)
 * 3. For any operation type t, no more than 'mt' nodes of type t are mapped a given slot
 * 3.1 The same operations scheduled at the same slot have different FUs if any.
 */
void CheckScheduling(int latency, int *ASAP_slots, int *ALAP_slots)
{
  int i, j;
  int mt;
  int destNu;
  enum operation op;

  for (i = 0; i < NODE_NU; i++) {

    // rule 1
    //assert(DFG[i]->opScheduledSlot > 0); 

    // check scheduled slot is in between mobility range

    if (ASAP_slots != NULL && ALAP_slots != NULL) {
      //assert(DFG[i]->opScheduledSlot <= ALAP_slots[i]);
      //assert(DFG[i]->opScheduledSlot >= ASAP_slots[i]);
      if (DFG[i]->opScheduledSlot > ALAP_slots[i]) {
        printf("Error at CheckScheduling: node %d at step %d > ALAP step %d\n",
        i, DFG[i]->opScheduledSlot, ALAP_slots[i]);
        PrintDFG(ASAP_slots, ALAP_slots);
      }
      if (DFG[i]->opScheduledSlot < ASAP_slots[i]) {
        printf("Error at CheckScheduling: node %d at step %d < ASAP step %d\n",
        i, DFG[i]->opScheduledSlot, ASAP_slots[i]);
        PrintDFG(ASAP_slots, ALAP_slots);
      }
    }

    // rule 2
    for (j = 0; j < DFG[i]->opDestNu; j++) {
      destNu = DFG[i]->opDest[j];
      if (destNu != SINK)
        if (DFG[i]->opScheduledSlot + DFG[i]->opLatency + 1 >=
            DFG[destNu]->opScheduledSlot) {
          printf("rule2 Error at CheckScheduling: node %d and node %d", i, destNu);
          if (ASAP_slots != NULL && ALAP_slots != NULL)
            PrintDFG(ASAP_slots, ALAP_slots);
          exit(1);
        }
    }
  }

  // rule 3    
  //for (op = add; op < add + OP_NU; op++) {
  //  for (i = 1; i < latency; i++) {
  //    mt = 0;
  //    for (j = 0; j < NODE_NU; j++) {
  //      if (DFG[j]->opScheduledSlot == i && DFG[j]->op == op)
  //        mt++;
  //    }
  //    assert(mt <= (RQ[op] == NULL) ? 0 : (*RQ[op])->size));
  //  }
  //}

  /* rule 3.1
  for (i = 0; i < NODE_NU; i++) {
    for (j = i + 1; j < NODE_NU; j++)
      if (DFG[i]->opScheduledSlot == DFG[j]->opScheduledSlot &&
          DFG[i]->op == DFG[j]->op)
        assert(DFG[i]->opResourceNu != DFG[j]->opResourceNu);
  }
  */
}

void SetNodePriority(int a0[], int p[], int nu) {
  myprintf("path length: a0 = "); PrintArray(a0, NODE_NU);
  myprintf("path length: p = "); PrintArray(p, NODE_NU);
  // start from non-port node
  bubblesort(a0, p, nu, NODE_NU);
  myprintf("after bubble sort path length: a0 = "); PrintArray(a0, NODE_NU);
  myprintf("after bubble sort path length: p = "); PrintArray(p, NODE_NU);
}


// Update ASAP slots of all the successors of node i
void Update_ASAP_Slots(int i, int *ASAP_slots, int *ALAP_slots, int flag) {

  int actual, orig;
  int destNu;
  int j, k;

  while(1) {

    if (DFG[i]->opDest[0] == SINK) return;

    //actual = DFG[i]->opScheduledSlot + DFG[i]->opLatency + 1;
    actual = ASAP_slots[i] + DFG[i]->opLatency + 1 + 1; // register insertion?

    destNu = DFG[i] -> opDestNu;

    for (j = 0; j < destNu; j++) {
      k = DFG[i]->opDest[j];
      orig = ASAP_slots[k];
      if (actual > orig && actual <= ALAP_slots[k]) {
        //myprintf("Adjust ASAP: node %d orig = %d actual = %d\n", \
            k, orig, actual);
        ASAP_slots[k] = actual;
        Update_ASAP_Slots(k, ASAP_slots, ALAP_slots, flag);
      }

      // what if actual is not in the range ( orig, ALAP_slot[k] ] ? 3/5/12
      //assert(actual > orig);
      //assert(actual <= ALAP_slots[k]);
    }
    return;
  }
}

// Update ALAP slots of all the predecessors of node i
void Update_ALAP_Slots(int i, int *ASAP_slots, int *ALAP_slots, int flag) {

  int actual, orig;
  int srcNu;
  int j, k;

  while(1) {

    if (DFG[i]->opSrc[0] < 0) return;

    srcNu = DFG[i]->opConst ? 1 : 2;

    for (j = 0; j < srcNu; j++) {
      // for each pred of i
      k = DFG[i]->opSrc[j];

      // the original last step of the pred
      orig = ALAP_slots[k];

      //actual = DFG[i]->opScheduledSlot - DFG[k]->opLatency - 1; 
      //actual = ALAP_slots[i] - DFG[k]->opLatency - 1; 
      actual = ASAP_slots[i] - DFG[k]->opLatency - 1 - 1; // register insertion ?
      if (actual < orig && actual >= ASAP_slots[k]) {
        //myprintf("Adjust ALAP: node %d orig = %d actual = %d\n",\
            k, orig, actual);
        ALAP_slots[k] = actual;
        Update_ALAP_Slots(k, ASAP_slots, ALAP_slots, flag);
      }

      // what if actual is not in the range [ ASAP_slot[k], orig ) ? 3/5/12
      //assert(actual < orig);
      //assert(actual >= ASAP_slots[k]);
    }
    return;
  }
}

//=======================================================
//  Resource-constraint Schedule RAT
//=======================================================

// Setup ASAP resource allocation table
void CreateScheduleRAT() {

  enum operation op_type;
  int fuNu;
  int i;

  SRAT = (SRAT_FuType_Ptr *) malloc (sizeof(SRAT_FuType_Ptr) * OP_NU);

  for (op_type = add; op_type < add + OP_NU; op_type++) {

    //fuNu = myceil(FU[op_type], minII);
    fuNu = (GetOpSym(op_type) == 'm' || GetOpSym(op_type) == 'r') ?
           FU[op_type] : myceil(FU[op_type], minII);

    SRAT[op_type] = (SRAT_FuType_Ptr) malloc (sizeof(SRAT_FuType) * fuNu);

    for (i = 0; i < fuNu; i++) {
      // Two input ports for each FU
      SRAT[op_type][i].slotNu   =  0;
      SRAT[op_type][i].conflict =  0;
      SRAT[op_type][i].head     =  NULL;
    }
  }
}

void FreeScheduleRAT() {
  enum operation op_type;
  int fuNu;
  int i;
  ScheduledSlotPtr head, p, p1, p2;

  for (op_type = add; op_type < add + OP_NU; op_type++) {

    // 8/1/11
    fuNu = (GetOpSym(op_type) == 'm' || GetOpSym(op_type) == 'r') ?
           FU[op_type] : myceil(FU[op_type], minII);

    for (i = 0; i < fuNu; i++) {
      p = head = SRAT[op_type][i].head;
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
    free(SRAT[op_type]);
  }
  free(SRAT);
}

void PrintScheduleRAT() {

  enum operation op_type;
  int fuNu;
  int i;

  ScheduledSlotPtr head, p, p1, p2;
  myprintf("-------- Schedule RAT --------\n");
  for (op_type = add; op_type < add + OP_NU; op_type++) {
    fuNu = myceil(FU[op_type], minII);
    for (i = 0; i < fuNu; i++) {
      myprintf("OP: %8s FU%d ", opSymTable(op_type), i);
      p = head = SRAT[op_type][i].head;
      while (p != NULL) {
        myprintf("%2d ", p->slot);
        p = p->next;
      }
      myprintf("\n");
    }
  }
  myprintf("\n--------------------------\n");
}

int InsertScheduleRAT(int op_type, int slot) {

  int i;
  int fuNu;
  int entry = 0;
  int conflict = 0;

  // num of slots assigned to the same FU (possibly +inf)
  int minSlotNu = 1000; 
  int maxSlotNu = -1; 

  ScheduledSlotPtr head, p0, p1;
  ScheduledSlotPtr ss;

  // Number of sources of each type is specified in FU table
  // fuNu = myceil(FU[op_type], minII);
    fuNu = (GetOpSym(op_type) == 'm' || GetOpSym(op_type) == 'r') ?
           FU[op_type] : myceil(FU[op_type], minII);

  // conflict check ta = tb (mod minII)
  for (i = 0; i < fuNu; i++) {
    p0 = SRAT[op_type][i].head;
    while (p0 != NULL) {
      if (abs(p0->slot - slot) % minII == 0) {
        myprintf("%s%d minII=%d %d <<<c>>> %d\n", 
            opSymTable(op_type), i, minII, p0->slot, slot);
        SRAT[op_type][i].conflict = 1;  
        conflict++;
        break;
      }
      p0 = p0 -> next;
    }
  }

  // We don't schedule the node if each FU's slot has conflict with 
  // the current slot, but must clear conflict records before function
  // return.
  if (conflict == fuNu) {
    for (i = 0; i < fuNu; i++) 
      (SRAT[op_type][i]).conflict = 0;  
    return -1;
  }

  // create a slot node to be added to the list described below
  ss = malloc(sizeof(struct ScheduledSlot));
  ss->slot = slot;

  // option A: Assign the operation to a leastly used non-conflicting FU  (ASAP)
  for (i = 0; i < fuNu; i++) {
    if (SRAT[op_type][i].conflict == 0)
      if (minSlotNu > (SRAT[op_type][i]).slotNu) {
        minSlotNu = (SRAT[op_type][i]).slotNu;
        entry = i;
      }
  }


  /* option B: Assign the operation to the mostly used non-conflicting FU (branch/bound)
  for (i = 0; i < fuNu; i++) {
    if (SRAT[op_type][i].conflict == 0)
      if (maxSlotNu < (SRAT[op_type][i]).slotNu) {
        maxSlotNu = (SRAT[op_type][i]).slotNu;
        entry = i;
      }
  }
  */

  // add scheduled slot to the end of the list
  p0 = p1 = head = (SRAT[op_type][entry]).head;

  if (head == NULL) {
    head = ss;
    ss->next = NULL;
  } else {
    while (p0 != NULL) {
      p1 = p0;
      p0 = p0->next;
    }

    p1->next = ss;
    ss->next = NULL;
  }

  // after add, update the list
  (SRAT[op_type][entry]).head = head;

  // increment allocated slot number
  (SRAT[op_type][entry]).slotNu++;

  // clear conflict records
  for (i = 0; i < fuNu; i++)
    (SRAT[op_type][i]).conflict = 0;

#ifdef DEBUG
  PrintScheduleRAT();
#endif

  return entry;
}

//=======================================================
//  Resource-constraint Schedule RAT end
//=======================================================
