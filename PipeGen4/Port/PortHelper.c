#include <stdlib.h> 
#include <stdio.h> 
#include <assert.h> 
#include "Schedule.h"
#include "PortSchedule.h"

void CreatePortPriority(int maxInputPortNu, int P) {
  int i;

  if (P != maxInputPortNu) {
    PortPriority = (int *) malloc (sizeof(int) * maxInputPortNu);
    SortedPortPriority = (int *) malloc (sizeof(int) * maxInputPortNu);
  }
  else {
    PortPriority = NULL;
    SortedPortPriority = NULL;
  }

  PortAssignment   = (int *) malloc (sizeof(int) * maxInputPortNu);
  NodePriority = (int *) malloc (sizeof(int) * NODE_NU);

  //--------------------------------------
  // Port priority (topological by default)
  //--------------------------------------
  if ( P != maxInputPortNu) {
    for (i = 0; i < maxInputPortNu; i++) {
      PortPriority[i] = i;
    }
  }

  //-----------------------------------------------------
  // Port mapping from logical port number (PortPriority[i])
  // to physical port number
  //-----------------------------------------------------
  for (i = 0; i < maxInputPortNu; i++) {
    PortAssignment[i] = 0;
  }

  //-----------------------------------------------------
  // Internal nodes with path_length-based priority
  //-----------------------------------------------------
  for (i = 0; i < NODE_NU; i++) {
    NodePriority[i] = i;
  }

  int *path  = (int *) malloc (sizeof(int) * NODE_NU);
  SetPathLength(path);
  SetNodePriority(path, NodePriority, maxInputPortNu);

  free(path);

  if ( P != maxInputPortNu) {

  #ifdef PORT_PRIORITY

  /* Port-constrained scheduling 2009
     For a set of input ports assigned with three kind of priorities 
     (1. usage freq, 2. number of successors and 3)subtree coverage) 
     in which the priority order is 2 > 3 > 1.
     How to order input ports sequence according to the priority relationship?
  */
  
  //  A global array records the priority from number 0 to (maxInputPortNu - 1)
  // For example: 
  // pri port
  // 0 -- 1
  // 1 -- 2
  // 2 -- 3
  // 3 -- 4
  // 4 -- 0
  // Then the node(port) priority is n1 > n2 > n3 > n4 > n0
  //-------------------------------------------------------------------
    int *successor = (int *) calloc (maxInputPortNu, sizeof(int));
    int *frequency = (int *) calloc (maxInputPortNu, sizeof(int));
    int *coverage  = (int *) calloc (maxInputPortNu, sizeof(int));

    SetPortSucc (successor);
    SetPortFU   (frequency);
    SetPortCover(coverage);

    SetPortPriority(successor, coverage, frequency, PortPriority, maxInputPortNu);

    free(successor); free(frequency); free(coverage); 

    // Errors
    // UUCI
    // priority[5] = {2 3 0 1 4}
    /*
    PortPriority[0] = 2;
    PortPriority[1] = 3;
    PortPriority[2] = 0;
    PortPriority[3] = 1;
    PortPriority[4] = 4;
    */
    #endif
  }

  //-----------------------------------------------------
  // Update DFG opPriority
  // The node priority happens to be similar to node mobility:
  // a small value of opPriority has higher priority.
  //-----------------------------------------------------
  for (i = 0; i < NODE_NU; i++)
    //if (DFG[i]->opSrc[0] < 0) { 
    if (DFG[i]->op == nop) {
      //DFG[i]->opPriority = GetPortPriority(i, PortPriority, maxInputPortNu);
      // 5/31/11
      DFG[i]->opPriority = (PortPriority == NULL) ? 0 : 
        GetPortPriority(i, PortPriority, maxInputPortNu);
      myprintf("node %d: priority = %d\n", i, DFG[i]->opPriority);
    }
}

void SetPortPriority(int a0[], int a1[], int a2[], int p[], int maxInputPortNu) {

  myprintf("Show all priority arrays: \n");
  myprintf("successor: a0 = "); PrintArray(a0, maxInputPortNu);
  myprintf("coverage : a1 = "); PrintArray(a1, maxInputPortNu);
  myprintf("frequency: a2 = "); PrintArray(a2, maxInputPortNu);
  int *t1 = (int *) malloc(sizeof(int) * maxInputPortNu);

  bubblesort(a0, p, 0, maxInputPortNu);
  myprintf("a0 = "); PrintArray(a0, maxInputPortNu);
  myprintf("p  = ");  PrintArray(p, maxInputPortNu);

  myprintf("Show the second sorted array and its indirect index: \n");
  priority_sort(a0, a1, p, t1, maxInputPortNu);

  myprintf("Show the third sorted array and its indirect index: \n");
  priority_sort(t1, a2, p, t1, maxInputPortNu);

  free(t1); 
}

//-----------------------------------------------------
// return the index of the node's priority
//
// Node number in PortPriority array with index i=0 highest priority
//-----------------------------------------------------
int GetPortPriority(int nodeID, int PortPriority[], int maxInputPortNu) {
  int i;
  for (i = 0; i < maxInputPortNu; i++)
    if (nodeID == PortPriority[i]) return i;

  // never comes here
  assert(i != maxInputPortNu);
}

#ifdef PORT_CONSTRAINT
void CheckPortPriority(int *p, int *Xconstraint, int *Kconstraint, int size) {
  int i, j, k = 0;

  for (i = 0; i < size; i++) {
    if (Xconstraint[i]) {
      assert(PORTS_CONSTRAINT[p[k]] == 0);
      k++; // go to next element in p
    } 
     
    for (j = 0; j < Kconstraint[i]; j++) {
      assert(PORTS_CONSTRAINT[p[j + k]] == 1);
    }

    k += j; // go to next element in p
  }
}

void SetFullPortPriority(int *p, 
  int *Xconstraint, int *Kconstraint, 
  int *Xs, int *Ks, int *Xp, int *Kp, 
  int xsize, int ksize, int CombSize) {

  int i, j, k, x;
  int maxCnt;
  int psize = xsize + ksize;

   myprintf("Xconstraint priority = ");
   PrintArray(Xconstraint, CombSize);

   myprintf("Kconstraint priority = ");
   PrintArray(Kconstraint, CombSize);

   myprintf("Xs perm = ");
   PrintArray(Xs, xsize);

   myprintf("Ks perm = ");
   PrintArray(Ks, ksize);

   myprintf("Xp node = ");
   PrintArray(Xp, xsize);

   myprintf("Kp node = ");
   PrintArray(Kp, ksize);

  i = 0; j = 0; x = 0; k = 0;
  while (CombSize-- > 0) {
    if (Xconstraint[x]) {
      p[i++] = Xp[Xs[j++]];
    }

    maxCnt = Kconstraint[x];

    while (maxCnt > 0) {
      p[i++] = Kp[Ks[k++]];
      maxCnt--; 
    }
    x++;
  }

  myprintf("port priority = "); PrintArray(p, psize);

  CheckPortPriority(p, Xconstraint, Kconstraint, CombSize);
}
#endif // PORT_CONSTRAINT


//========================================================================
// Port priority helpers
//========================================================================

/* Port coverage of division, for example */
void SetPortCover(int coverage[]) {
  int i, j;
  enum operation op;
  int latency = 0;

  // Get operation with longest latency
  // Assume DIV takes 27cc
  for (i = 0; i < OP_NU; i++)
    if (LATENCY[i] > latency) {
      latency = LATENCY[i];
      j = i;
    }

  op = add + j;

  // tree coverage for each long-latency operation op (e.g. div)
  for (i = 0; i < NODE_NU; i++) {
    if (DFG[i]->op == op && DFG[i]->opDest[0] == SINK) continue;
    if (DFG[i]->op == op) {
      // reset port nodes
      for (j = 0; j < NODE_NU; j++)
        //if (DFG[j]->opSrc[0] < 0) 
        if (DFG[j]->op == nop)
          DFG[j]->opScheduled = 0;

      myprintf("From div node %d\n", i);
      PortCover(i, coverage);
    }
  }

  // Debug operator tree coverage of input port
  for (i = 0; i < NODE_NU; i++) {
    //if (DFG[i]->opSrc[0] < 0) { 
    if (DFG[i]->op == nop) {
      myprintf("coverage: port node %d - %d\n", i, coverage[i]);
      //assert(CheckPortCover(op, i) ==  coverage[i]);
    }
  }
}

int CheckPortCover(int op, int i) {

  int j;
  int coverage;
  // reset all nodes
  for (j = MAX_PORT_NU; j < NODE_NU; j++) DFG[j]->opScheduled = 0;
  coverage = OpCover(op, i);
  //myprintf("CheckPortCover: port node %d - %d\n", i, coverage);
  return coverage; 
}

/* 
 * 
 *
 */
int OpCover(int op, int i) {

  int j, k;
  int destNu;
  int coverage = 0;

  while(1) {

    //myprintf("Check OPCover: traveral node %d\n", i);

    if (DFG[i]->opDest[0] == SINK) return coverage;

    destNu = DFG[i] -> opDestNu;
    for (j = 0; j < destNu; j++) {
      k = DFG[i]->opDest[j];
      if (op == DFG[k]->op && !DFG[k]->opScheduled) {
        myprintf("Check OPCover: node %d = op\n", k);
        coverage++;
        DFG[k]->opScheduled = 1;
      }
      coverage += OpCover(op, k);
    }
    return coverage;
  }
}

void SetPortFU(int frequency[]) {
  int i;
  for (i = 0; i < NODE_NU; i++) {
    //if (DFG[i]->opSrc[0] < 0) {
    if (DFG[i]->op == nop) {
      myprintf("frequency: port node %d - %d\n", i, DFG[i]->opPriority);
      //DFG[i]->opPriority = DFG[i]->opDestNu;
      //frequency[i] = DFG[i]->opPriority;
      frequency[i] = DFG[i]->opDestNu;
    }
  }
}

void SetPortSucc(int successor[]) {
  int i;
  for (i = 0; i < NODE_NU; i++) {
    //if (DFG[i]->opSrc[0] < 0) {
    if (DFG[i]->op == nop) {
      myprintf("successor: port node %d - %d\n", i, DFG[i]->opPriority);
      //DFG[i]->opPriority = PathSucc(i);
      //successor[i] = DFG[i]->opPriority;
      successor[i] = PathSucc(i);
    }
  }
}

void SetPathLength (int path[]) {
  int i;
  for (i = 0; i < NODE_NU; i++) {
    DFG[i]->opPriority = PathLength(i);
    myprintf("Path length: node %d to SINK is %d\n", i, DFG[i]->opPriority);
    path[i] = DFG[i]->opPriority;
  }
}

//===================================================================
// 
//===================================================================

// If a port node has been traversed (suppose opScheduled is set),
// then it is not counted again
void PortCover(int nodeID, int *coverage) {
  int i;
  myprintf("Traverse node %d\n", nodeID);
  if (DFG[nodeID]->opSrc[0] < 0 && DFG[nodeID]->op != nopc && !DFG[nodeID]->opScheduled) { // 4/12/12
    myprintf("Traverse port node %d\n", nodeID);
    coverage[nodeID]++;
    DFG[nodeID]->opScheduled = 1; 
  } else if (DFG[nodeID]->opSrc[0] >= 0) {
    for (i = 0; i < 2; i++) {
      if (DFG[nodeID]->opConst) { 
        PortCover(DFG[nodeID]->opSrc[i], coverage);
        break;
      } else 
        PortCover(DFG[nodeID]->opSrc[i], coverage);
    }
  }
}

/* The number of successors from primary inputs to primary outputs.
 * This roughly equals the number of operations performed until 
 * the primary output 
 */
int PathSucc(int nodeID) {

  int i = nodeID;
  int j;
  int destNu;
  int pathSucc;
  int maxPathSucc = 0;
  int succ = 0;

  while(1) {
    succ++;
    if (DFG[i]->opDest[0] == SINK) return succ;

    destNu = DFG[i] -> opDestNu;
    for (j = 0; j < destNu; j++) {
      pathSucc = PathSucc(DFG[i]->opDest[j]);
      if (maxPathSucc < pathSucc) 
        maxPathSucc = pathSucc; 
    }
    succ += maxPathSucc;
    return succ;
  }
}

/* The FU latency from primary inputs to primary outputs.
 * This equals the sum of each operation's delay between
 * the primary input and output.
 */
int PathLength(int nodeID) {

  int i = nodeID;
  int j;
  int destNu;
  int pathLength;
  int maxPathLength = 0;
  int length = 0;

  while(1) {
    //length += (DFG[i]->opLatency + 1);
    length += DFG[i]->opLatency;
    if (DFG[i]->opDest[0] == SINK) return length;

    destNu = DFG[i] -> opDestNu;
    for (j = 0; j < destNu; j++) {
      pathLength = PathLength(DFG[i]->opDest[j]);
      if (maxPathLength < pathLength) 
        maxPathLength = pathLength; 
    }
    length += maxPathLength;
    return length;
  }
}


