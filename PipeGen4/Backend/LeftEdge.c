#include <stdio.h> 
#include <stdlib.h> 
#include <assert.h> 
#include "Schedule.h"

//-----------------------------------------------------------
// LEA for register sharing in a pipelined data path 
//-----------------------------------------------------------
int LeftEdgeRegisterBinding(int *Map) {

  int i;
  int reg_index = 0;
  int last;
  int conflict;
  int time, start, end;
  int time1;

  varList List_head = NULL;
  varList currVar, tempVar;
  varList var;

  List_head = CreateVarList(List_head);
  myprintf("-- Print sorted variables' life time --\n");
  VarListPrint(List_head);
  myprintf("---------------------------------------\n");
  
  while (List_head != NULL) {
    reg_index++;
    currVar = List_head; //VarListFirst(List_head);
    last = 0;
    
    CreateVarTable(reg_index);

    while (currVar != NULL) {

      if (currVar->startTime >= last) {
           
        // Check start time conflict of shared register
        // VarTable starts from index 0
        start = currVar->startTime;
        end = currVar->endTime;
        conflict = 0;
        var = VarTable[reg_index - 1];

        //======================================================================
        // Each time step in the lifetime interval of a 
        // variable needs to be compared with the start 
        // time of all previously allocated variables. 
        // If the time difference is a multiple of minII, 
        // then a new register is allocated for the variable. 
        //======================================================================
        // 8/1/11
        // 
        // Assumption
        //
        // Registers are not shared when their data path width is not the same
        // In addition, there is no register allocation for "cport" node.
        //======================================================================
        while (var != NULL) {

          if (DFG[var->varID]->opPrecision != DFG[currVar->varID]->opPrecision || 
              strcmp(opTypeTable(DFG[var->varID]->op), "compare") != 
              strcmp(opTypeTable(DFG[currVar->varID]->op), "compare") ||
              strcmp(opTypeTable(DFG[var->varID]->op), "cport") != 
              strcmp(opTypeTable(DFG[currVar->varID]->op), "cport")) {

            conflict = 1;
            break;
          }

          for (time = start; time < end; time++) {
            assert(time >= var->startTime);
            for (time1 = var->startTime; time1 < var->endTime; time1++)
              if ((time - time1) % minII) {
              continue;
            } else {
              conflict = 1;
              break;
            }
          }
          var = var->next;
        } 
        //======================================================================
        // end of while loop 
        //======================================================================

        if (conflict) {
          //currVar = VarListNext(List_head, currVar->varID);
          currVar = VarListNext(List_head, currVar);
          continue; 
        }

        //======================================================================
        // The end step of a variable needs to be compared 
        // with the end time of all previously allocated variables. 
        // If the time difference is a multiple of minII, 
        // then a new register is allocated for the variable. 
        //======================================================================
        
        var = VarTable[reg_index - 1];
        while (var != NULL) {
          assert(end >= var->endTime);
          if ((end - var->endTime) % minII) {
            var = var->next;
          } else {
            conflict = 1;
            break;
          }
        }

        if (conflict) {
          //currVar = VarListNext(List_head, currVar->varID);
          currVar = VarListNext(List_head, currVar);
          continue; 
        }

        /*
        if (DFG[currVar->varID]->op == nopc) {
          reg_index = 0 - (int)(DFG[currVar->varID]->opConstVal);
          myprintf("reg_index = %d\n", reg_index);
        }
        */

        Map[currVar->varID] = reg_index;
        last = currVar->endTime;

        assert(currVar->startTime < currVar->endTime);

        VarTableInsert(reg_index, currVar);

        tempVar = currVar;
        myprintf("currVar->varID %d\n", currVar->varID);
        //currVar = VarListNext(List_head, currVar->varID);
        currVar = VarListNext(List_head, currVar);
        List_head = VarListDelete(List_head, tempVar->varID);
      }
      else
        //currVar = VarListNext(List_head, currVar->varID);
        currVar = VarListNext(List_head, currVar);
    }
    myprintf("reg_index %d\n", reg_index);
  }

  // Output register for SINK nodes 
  //Map[NODE_NU-1] = reg_index;

  for (i = 0; i < reg_index; i++) {
    myprintf("VarTable shared register %d:\n", i + 1);
    VarListPrint(VarTable[i]);
  }

  myprintf("\n");

  /*
  myprintf("Map: ");
  for (i = 0; i < NODE_NU; i++) 
    myprintf("(%d %d) ", i, Map[i]);
  myprintf("\n");
  */

  return reg_index;
}

//------------------------------------------------------
// Each operation node's output variable is considered 
// including the SINK nodes as their endTime is N/A.
//
// nopc node is not added to the VarList. Currently,
// 
// In mrbay application, nopcs nodes are only connected to
// MUX nodes and MULD nodes. 
//------------------------------------------------------
varList CreateVarList(varList head) {

  int i, j;
  int varID, startTime, endTime;
  varList p = head;

  for (j = 0; j < NODE_NU; j++) {

    if (DFG[j]->opSrc[0] >= 0) {  // excluding port nodes

      if (DFG[j]->op == tod || DFG[j]->op == tof) { // single-double conversion
        for (i = 0; i < 1; i++) {
          if (DFG[j]->opSrc[i] >= 0) {
            varID = DFG[j]->opSrc[i]; 
            startTime = DFG[varID]->opResultDoneSlot;

            //====================================================
            // Register insertion 7/26/11
            //====================================================
            //endTime = DFG[j]->opScheduledSlot;
            endTime = GetEndTime(j);

            myprintf("%s Var%3d : ", (DFG[j]->op == tod) ? "TOD" : "TOF", varID); 
            myprintf("%3d(node %d)", startTime, varID); myprintf(" --- "); myprintf("%3d(node %d)\n", endTime, j);
            p = VarListInsert(p, varID, startTime, endTime);
          }
        }
      }

      // mux node (one of the node's sources may be a nopc node)
      else if (DFG[j]->op == mx || DFG[j]->op == mxd) { // 2-1 mux
        for (i = 0; i < 3; i++) {
          if (DFG[j]->opSrc[i] >= 0) {
            varID = DFG[j]->opSrc[i]; 

            if (DFG[varID]->op == nopc)
              startTime = GetEndTime(j);
            else
              startTime = DFG[varID]->opResultDoneSlot;

            //====================================================
            // Register insertion 7/26/11 ??
            //====================================================
            //endTime = DFG[j]->opScheduledSlot;
            endTime = GetEndTime(j);

            myprintf("MUX Var%3d : ", varID); 
            myprintf("%3d(node %d)", startTime, varID); myprintf(" --- "); myprintf("%3d(node %d)\n", endTime, j);
            if (DFG[varID]->op != nopc)
              p = VarListInsert(p, varID, startTime, endTime);
          }
        }
      }

      // rom node
      else if (DFG[j]->op == rom || DFG[j]->op == romd) {
        for (i = 0; i < 4; i++) {
          if (DFG[j]->opSrc[i] >= 0) {
            varID = DFG[j]->opSrc[i]; 
            startTime = DFG[varID]->opResultDoneSlot;

            //====================================================
            // Register insertion 7/26/11 ??
            //====================================================
            //endTime = DFG[j]->opScheduledSlot;
            endTime = GetEndTime(j);

            myprintf("ROM Var%3d : ", varID); 
            myprintf("%3d(node %d)", startTime, varID); myprintf(" --- "); myprintf("%3d(node %d)\n", endTime, j);
            p = VarListInsert(p, varID, startTime, endTime);
          }
        }
      }
      
      // other node (one of the node's sources may be a nopc node)
      else {
        for (i = 0; i < 2 - DFG[j]->opConst; i++) {
          // Get Node j's source register number
          varID = DFG[j]->opSrc[i]; 

          if (DFG[varID]->op == nopc)
            startTime = GetEndTime(j);
          else
            startTime = DFG[varID]->opResultDoneSlot;

          //====================================================
          // Register insertion 7/9/11
          //====================================================
          //endTime = DFG[j]->opScheduledSlot;
          endTime = GetEndTime(j);

          myprintf("Var%3d : ", varID); 
          myprintf("%3d(node %d)", startTime, varID); myprintf(" --- "); myprintf("%3d(node %d)\n", endTime, j);

          //==================================================================
          // We don't insert the nopc's varID and lifetime into the list !!
          //==================================================================
          if (DFG[varID]->op != nopc)
            p = VarListInsert(p, varID, startTime, endTime);
        }
      }
    }
  }

  // Add output register for SINK node(s)
  for (j = 0; j < NODE_NU; j++) {
    if (DFG[j]->opDest[0] == SINK && DFG[j]->opDestNu == 1) { 
      varID = j; // named as node ID
      startTime = DFG[varID]->opResultDoneSlot;
      endTime = 10000;
      p = VarListInsert(p, varID, startTime, endTime);
    }
  }
  return p;
}

varList VarListFirst(varList head) {
  return head;
}

//--------------------------------------------------------------------------
// Returns the variable pointer following varID in List
//--------------------------------------------------------------------------
varList VarListNext(varList head, varList currVar) { //int varID) {
  assert (head != NULL);

  varList p1, p2;
  p1 = head;

  while (currVar->varID != p1->varID && p1->next != NULL) {
    p2 = p1;
    p1 = p1->next;
  }

  while (currVar->varID == p1->varID && p1 != NULL) {
    if ( currVar->startTime == p1->startTime && currVar->endTime == p1->endTime)
      return p1->next;
    else 
      p1 = p1->next;
  }

  myprintf("Variable ID %d not found\n", currVar->varID);
  exit(1);
  return NULL;
}

//--------------------------------------------------------------------------
// Insert the variables in L in ascending order with their start times,
// Start(v) as the primary key and in descending order with their end times,
// End(v) as the secondary key.
//--------------------------------------------------------------------------
varList VarListInsert(varList head, int varID, int startTime, int endTime) {
  varList var;
  varList p0, p1, p2;
  int key;

  p2 = head;

  while (p2 != NULL) {

    if (p2->varID == varID) {

      if (p2->endTime < endTime) {

        // Multiple destinations of a node have the same start time
        assert(p2->startTime == startTime); 

        // Delete the node and reinsert it as the endTime has been Increased
        // for the same varID (5/2/2011) e.g. UUCI  Var2 (1,2) --> (1,7)
        head = VarListDelete(head, varID);
        break;
      }
      else 
        return head; // Don't insert the same node with smaller or equal endTime
    }
    else {
      p2 = p2->next;
    }
  }

  p0 = head; // p0 may be different from p2 !!

  var = malloc(sizeof(struct varLifeTime));
  var->varID = varID;
  var->startTime = startTime;
  var->endTime = endTime;

  myprintf("Inserting variable ... %d \n", varID);

  if (head == NULL) {
    head = var;
    var->next = NULL;
  } else {
    while (p0 != NULL) {
      key = 0;
      // sorts the variables in ascending order with their start times 
      // as the primary key and in descending order with their end time
      // as the secondary key.
      if (startTime < p0->startTime)
        key = 1;
      //else if (startTime == endTime)
      else if (startTime == p0->startTime)
        if (endTime > p0->endTime)
          key = 1;

      if (key) {
        if (p0 == head) {
          var->next = head;
          head = var;
        } else {
          var->next = p0;
          p1->next = var;
        }
        return head; // go back
      }

      p1 = p0;
      p0 = p0->next; 
    }

    p1->next = var;
    var->next = NULL;
 }   
 return (head);
}

//------------------------------------------------------
// Delete the node with the given variable number varID
//------------------------------------------------------
varList VarListDelete(varList head, int varID) {
  assert (head != NULL);

  varList p1, p2;
  p1 = head;


  while (varID != p1->varID && p1->next != NULL) {
    p2 = p1;
    p1 = p1->next;
  }

  if (varID == p1->varID) {

    if (p1 == head)
      head = p1->next;
    else
      p2->next = p1->next;

    free(p1);

  } else {

    myprintf("Variable ID %d not found\n", varID);

  }
  return (head);
}

  
void VarListPrint(varList head) {
  varList p = head;
  myprintf("Variable List:\n");
  
  myprintf("Var ID:     ");
  while (p != NULL) {
    myprintf("%5d ", p->varID);
    p = p->next;
  }

  myprintf("\n");
  p = head;

  myprintf("FP prec:    ");
  while (p != NULL) {
    myprintf("%5d ", DFG[p->varID]->opPrecision);
    p = p->next;
  }

  myprintf("\n");
  p = head;


  myprintf("Var ID op:  ");
  while (p != NULL) {
    myprintf("%5s ", opSymTable(DFG[p->varID]->op));
    p = p->next;
  }

  myprintf("\n");
  p = head;

  myprintf("Start time: ");
  while (p != NULL) {
    myprintf("%5d ", p->startTime);
    p = p->next;
  }

  myprintf("\n");
  p = head;

  myprintf("End time:   ");
  while (p != NULL) {
    myprintf("%5d ", p->endTime);
    p = p->next;
  }
  myprintf("\n");
}

void FreeVarTable(const int SharedRegNu) {
  int i;
  varList head, p, p1, p2;

  for (i = 0; i < SharedRegNu; i++) {
    p = head = VarTable[i];
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
  free(VarTable);
  VarTable = NULL;
}

void VarTableInsert(int regNu, varList currVar) {

  varList head, p0, p1;
  varList var;

  p0 = p1 = head = VarTable[regNu - 1];

  var = malloc(sizeof(struct varLifeTime));
  var->varID = currVar->varID;
  var->startTime = currVar->startTime;
  var->endTime = currVar->endTime;

  if (head == NULL) {
    //myprintf("Create head var ID %d\n", var->varID );
    head = var;
    var->next = NULL;
  } else {

    while (p0 != NULL) {
      p1 = p0;
      p0 = p0->next;
    }

    // Append the resource at the end of the list
    //myprintf("Append var ID %d\n", var->varID);
    p1->next = var;
    var->next = NULL;
  }

  VarTable[regNu - 1] = head;
}

//------------------------------------------------------------------
// Create Variable Table based on the number of shared registers 
// and the number of registers sharing the same register
//------------------------------------------------------------------
void CreateVarTable(int regNu) {

  varList *ptr;
  ptr = (VarTable == NULL) ? NULL : VarTable;
  VarTable = (varList *) realloc (ptr, sizeof(varList) * regNu);
  VarTable[regNu - 1] = NULL;
}
