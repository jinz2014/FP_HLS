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

varList *CreateWBMClustersMax(varList *setV, int delay, int *minRegNu, int *clusterNu) {

  int i = 0;
  int maxRegNu, maxCutTime;
  int maxTime;

  varList V_head = NULL;
  varList p;

  myprintf("pipeline delay %d\n", delay);

  //---------------------------------------------------------------
  // Use varlist to collect all variables without repetition
  //---------------------------------------------------------------
  V_head = CreateVarList(V_head);
  myprintf("*********************** Print initial V set *********************\n");
  VarListPrint(V_head);

  while (V_head != NULL) {

    i++;

    //---------------------------------------------------------------
    // Find the time step with max number of living variables
    // and put them in set V_MAX
    //---------------------------------------------------------------
    maxRegNu = maxTime = 0;

    FindMaxVarSet(V_head, i, delay, &maxRegNu, &maxTime);

    // Save the maxRegNu only once
    if (i == 1) *minRegNu  = maxRegNu;

    maxCutTime = maxTime;

    myprintf("*********************** Print Vmax set **************************\n");
    VarListPrint(VarTable[i - 1]);

    //---------------------------------------------------------------
    // Create a cluster for Vmax 
    //---------------------------------------------------------------
    setV = CreateCluster(setV, i, i, *minRegNu); 
    (*clusterNu)++;

    //---------------------------------------------------------------
    // Remove Vmax from V 
    //---------------------------------------------------------------
    p = VarTable[i - 1];
    while (p != NULL) {
      V_head = VarListDelete(V_head, p->varID);
      p = p->next;
    }

    myprintf("***************** Print V set after removing Vmax **************\n");
    VarListPrint(V_head);

  }

  // Free the VarTable 
  FreeVarTable(i);

  //exit(1);
  return setV;
}

