#include "Schedule.h"
#include "PortSchedule.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include "WBM.h"

//*************************************************************************
// 1. Sort variables in V by the time they were born 
//
// 2. Find the control step CSmax where the number of variables alive is a maximum
//    Vmax   = { v | v is alive during CSmax}
//    Vright = { v | V - Vmax and v lives after CSmax}
//    Vleft  = { v | V - Vmax and v dies before CSmax}
//
// 3. put the variables of Vmax   in VarTable[0]
//    put the variables of Vright in VarTable[1]
//    put the variables of Vleft  in VarTable[2]
//
// 4. While (Vright is not empty) {
//      find the next mutually overlapped set in Vright
//      put the set in VarTable[i], i >= 3
//      i++;
//    }
//    
// 5. Sort Vleft according to the control steps in which they die
//
//    While (Vleft is not empty) {
//      find the next mutually overlapped set in Vleft
//      put the set in the VarTable[i] 
//      i++;
//    }
//*************************************************************************

//===============================================================
// Divide variables into clusters
//===============================================================

varList *CreateWBMClusters(varList *setV, int delay, int *minRegNu, int *clusterNu) {

  const int V_MAX = 1, V_RIGHT = 2, V_LEFT = 3; 
  int i;
  int varID;
  int maxRegNu, maxCutTime;
  int maxTime, time;

  varList V_head = NULL;
  varList VL_head = NULL;
  varList VR_head = NULL;
  varList p, q;

  myprintf("pipeline delay %d\n", delay);

  //---------------------------------------------------------------
  // Use varlist to collect all variables without repetition
  //---------------------------------------------------------------
  V_head = CreateVarList(V_head);
  myprintf("*********************** Print initial V set *********************\n");
  VarListPrint(V_head);

  //---------------------------------------------------------------
  // Find the time step with max number of living variables
  // and put them in set V_MAX
  //---------------------------------------------------------------
  maxRegNu = maxTime = 0;
  FindMaxVarSet(V_head, V_MAX, delay, &maxRegNu, &maxTime);
  *minRegNu  = maxRegNu;
  maxCutTime = maxTime;

  myprintf("*********************** Print Vmax set **************************\n");
  VarListPrint(VarTable[V_MAX - 1]);

  //---------------------------------------------------------------
  // Create a cluster for Vmax 
  //---------------------------------------------------------------
  setV = CreateCluster(setV, V_MAX, V_MAX, *minRegNu);
  (*clusterNu)++;

  //---------------------------------------------------------------
  // Remove Vmax from V 
  //---------------------------------------------------------------
  p = VarTable[V_MAX - 1];
  while (p != NULL) {
    V_head = VarListDelete(V_head, p->varID);
    p = p->next;
  }

  myprintf("***************** Print V set after removing Vmax **************\n");
  VarListPrint(V_head);

  //---------------------------------------------------------------
  // Add Vright = { v | V - Vmax and v lives after maxTime }
  //---------------------------------------------------------------
  CreateVarTable(V_RIGHT); // Recreate one entry for Vmax

  p = V_head; // Get from V
  while (p != NULL) {
    if (p->startTime >= maxCutTime) {

      VarTableInsert(V_RIGHT, p);

    }
    p = p->next;
  }

  //---------------------------------------------------------------
  // Remove Vright from V 
  //---------------------------------------------------------------
  p = VarTable[V_RIGHT - 1];
  while (p != NULL) {
    V_head = VarListDelete(V_head, p->varID);
    p = p->next;
  }

  myprintf("************************* Print Vright set ************************\n");
  VarListPrint(VarTable[V_RIGHT - 1]);

  //---------------------------------------------------------------
  // find the next mutually overlapped set Vunshare in Vright
  //---------------------------------------------------------------
  i = 0; // V_MAX, V_RIGHT, V_LEFT, V_LEFT + i
  while (VarTable[V_RIGHT - 1] != NULL) {

    maxRegNu = 0;
    maxTime  = 0;
    VR_head = VarTable[V_RIGHT - 1];

    FindMaxVarSet(VR_head, V_LEFT + i + 1, delay, &maxRegNu, &maxTime);

    // debug
    myprintf("********** Print mutually overlapped set in Vright **********\n");
    VarListPrint(VarTable[V_LEFT + i]);

    //---------------------------------------------------------------
    // Create a cluster for Vunshare in Vright
    //---------------------------------------------------------------
    setV = CreateCluster(setV, V_RIGHT + i, V_LEFT + i + 1, *minRegNu);
    (*clusterNu)++;

    // Use vars in VarTable[V_LEFT + i] to remove Vunsharable from VarTable[V_RIGHT]
    p = VarTable[V_LEFT + i];
    while (p != NULL) {
      VR_head = VarListDelete(VR_head, p->varID);
      p = p->next;
    }

    VarTable[V_RIGHT - 1] = VR_head;

    // go to next varTable entry
    i++;
  }

  myprintf("***************** Print V set after removing Vright **************\n");
  VarListPrint(V_head);

  //---------------------------------------------------------------
  // Add Vleft  = { v | V - Vmax and v dies before maxTime }
  //
  // If Vright is empty, then create Vleft entry;
  // Otherwise reset it to NULL as realloc() didn't do it after 
  // allocation of Vright set and the Vunshared set in Vright.
  // Recall that allocation of Vleft was already done after allocation
  // of Vunshared set in Vright.
  //---------------------------------------------------------------
  if (i == 0) 
    CreateVarTable(V_LEFT);  
  else
    VarTable[V_LEFT - 1] = NULL;

  int maxEndTime;

  // Sort Vleft according the steps which they die
  while (V_head != NULL) { // Get from Varlist
    maxEndTime = 0;
    q = NULL;

    p = V_head;  // V
    while (p != NULL) {
      if (p->endTime <= maxCutTime && p->endTime > maxEndTime) {
        maxEndTime = p->endTime;
        q = p;
      }
      p = p->next;
    }

    if (q != NULL) {
      // Insert Vleft from V
      VarTableInsert(V_LEFT, q);

      // Remove Vleft from V
      V_head = VarListDelete(V_head, q->varID);
    }
  }

  myprintf("************************* Print Vleft set ************************\n");
  VarListPrint(VarTable[V_LEFT - 1]);

  //---------------------------------------------------------------
  // find the next mutually overlapped set Vunshare in Vleft
  //---------------------------------------------------------------
  while (VarTable[V_LEFT - 1] != NULL) {

    VL_head = VarTable[V_LEFT - 1];

    maxRegNu = 0; maxTime = 0;
    FindMaxVarSet(VL_head, V_LEFT + i + 1, delay, &maxRegNu, &maxTime);

    p = VarTable[V_LEFT + i];

    // debug
    myprintf("********** Print mutually overlapped set in Vleft **********\n");
    VarListPrint(p);

    //---------------------------------------------------------------
    // Create a cluster for Vunshare in Vleft
    //---------------------------------------------------------------
    setV = CreateCluster(setV, V_RIGHT + i , V_LEFT + i + 1, *minRegNu);
    (*clusterNu)++;

    // Use vars in VarTable[V_LEFT + i] to remove Vunsharable from VarTable[V_LEFT]
    while (p != NULL) {
      VL_head = VarListDelete(VL_head, p->varID);
      p = p->next;
    }

    myprintf("********** Print Vleft after removing Vunshare **********\n");
    VarListPrint(VL_head);

    VarTable[V_LEFT - 1] = VL_head;

    // go to next varTable entry
    i++;
  }

  // Free the VarTable 
  FreeVarTable(V_LEFT + i);

  return setV;
}

