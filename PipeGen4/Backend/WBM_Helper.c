#include <stdio.h> 
#include <stdlib.h> 
#include "Schedule.h"
#include "PortSchedule.h"
#include "WBM.h"

//====================================================================================
//  MUXR table constains:
//  each variable's ID, operation and resource number
//
//  The index of the table is the register number to which the 
//  variable has been assigned.
//
//  The table is not updated with a new variable when 
//   1. the new variable corresponding to an input port has the same physical port
//      as the the input-port variable already in the table
//   or
//   2. the new variable's operation and resource number are the same as those of a 
//      variable already in the table.
//====================================================================================
void TableUpdateMUXR(var_struct_ptr var_MUXR_table, int reg_nu, int varID) {

  int update;
  int var_nu;
  int n;

  update = 1;
  var_nu = var_MUXR_table[reg_nu].var_nu;

  for (n = 0; n < var_nu; n++) {
    if ( (var_MUXR_table[reg_nu].var_list[n]).fuOp == DFG[varID]->op && (
         DFG[varID]->op == nop && 
         GetInputPortNu(varID) == GetInputPortNu((var_MUXR_table[reg_nu].var_list[n]).varID) ||
         (var_MUXR_table[reg_nu].var_list[n]).fuNu == DFG[varID]->opResourceNu)) {
      update = 0;
      break;
    }
  }

  if (update) {
    var_MUXR_table[reg_nu].var_list = realloc 
      (var_MUXR_table[reg_nu].var_list, (++(var_MUXR_table[reg_nu].var_nu)) * sizeof(var));
    (var_MUXR_table[reg_nu].var_list[var_nu]).fuNu  = DFG[varID]->opResourceNu; 
    (var_MUXR_table[reg_nu].var_list[var_nu]).fuOp  = DFG[varID]->op; 
    (var_MUXR_table[reg_nu].var_list[var_nu]).varID = varID;
  }
}

//====================================================================================
//  MUXP table constains each variable's ID, operation and resource number
//  The index of the table is the register number to which the 
//  variable has been assigned.

//  In DFG, find the node whose source is equal to the new variables' VarID,
//  The table is not updated when the node's ID is the same as any of the variables 
//  alreay in the list
//====================================================================================
void TableUpdateMUXP(var_struct_ptr var_MUXP_table, int reg_nu, int varID) {

  int update;
  int var_nu;
  int k, m, n;
  int src;

  for (k = 0; k < NODE_NU; k++) {
    if (DFG[k]->opSrc[0] >= 0) { 
      for (m = 0; m < 2 - DFG[k]->opConst; m++) {
        src = DFG[k]->opSrc[m];
        if (src == varID) {

          update = 1;
          var_nu = var_MUXP_table[reg_nu].var_nu;

          for (n = 0; n < var_nu; n++) {
            if ((var_MUXP_table[reg_nu].var_list[n]).fuOp == DFG[k]->op &&
                 DFG[k]->opResourceNu == (var_MUXP_table[reg_nu].var_list[n]).fuNu && 
                 k == (var_MUXP_table[reg_nu].var_list[n]).varID) {
              update = 0;
              break; 
            }
          }

          if (update) {
            // Save the node's varID whose source ID is equal to new variable's varID
            var_MUXP_table[reg_nu].var_list = realloc (var_MUXP_table[reg_nu].var_list, (++(var_MUXP_table[reg_nu].var_nu)) * sizeof(var));
            (var_MUXP_table[reg_nu].var_list[var_nu]).varID = k;
            (var_MUXP_table[reg_nu].var_list[var_nu]).fuNu = DFG[k]->opResourceNu; 
            (var_MUXP_table[reg_nu].var_list[var_nu]).fuOp = DFG[k]->op; 
            myprintf("MUXP: updating reg %d var_list with %d number\n", reg_nu, var_MUXP_table[reg_nu].var_nu);
          }
        }
      }
    }
  }
}

//===========================================================================
// var_MUX_table: var_MUXP_table/var_MUXR_table
// name: MUXP/MUXR
//===========================================================================
void DisplayTableMUX(var_struct_ptr var_MUX_table, int minRegNu, char *name) {
  int i, n;
  myprintf("===========================\n");
  myprintf("Dump var %s table\n", name);
  myprintf("===========================\n");
    myprintf("reg#   (var op op#)\n");
  for (i = 0; i < minRegNu; i++) {
    myprintf("r%d : ", i);
    if (var_MUX_table[i].var_list != NULL) {
      for (n = 0; n < var_MUX_table[i].var_nu; n++) {
        myprintf("(%d %c %d) ", (var_MUX_table[i].var_list[n]).varID,
            GetOpSym((var_MUX_table[i].var_list[n]).fuOp), 
                           (var_MUX_table[i].var_list[n]).fuNu); 
      }
    } else myprintf("null");
    myprintf("\n");
  }
}

//===========================================================================
// Use off-the-shelf codes available online
//===========================================================================
void hungarian_method(int *match, int *cost, int size) {

  hungarian_problem_t p;

  int** m = array_to_matrix(cost, size, size);

  int matrix_size = hungarian_init(&p, m , size, size, \
                    HUNGARIAN_MODE_MINIMIZE_COST) ; 
  hungarian_solve(&p);

  int i,j;
  int **C = (&p)->assignment;
  myprintf("\n");
  for(i=0; i< size; i++) {
    for(j=0; j< size; j++) {
      if (C[i][j]) match[i] = j;
    }
  }

  hungarian_free(&p);
  for(i=0;i<size;i++) free(m[i]);
  free(m);
}

//===========================================================================
// convert array to matrix
//===========================================================================
int** array_to_matrix(int* m, int rows, int cols) {
  int i,j;
  int** r;
  r = (int**)calloc(rows,sizeof(int*));
  for(i=0;i<rows;i++)
  {
    r[i] = (int*)calloc(cols,sizeof(int));
    for(j=0;j<cols;j++)
      r[i][j] = m[i*cols+j];
  }
  return r;
}

void PrintMatrix(int *cost, int row, int col) {
  int i, j;
  for (i = 0; i < row; i++) {
   for (j = 0; j < col; j++)
     myprintf("%4d ", cost[i * col + j]); 
   myprintf("\n");
  }
}

void FreeVarTableEntry(const int entry) {
  int i;
  varList head, p, p1, p2;

  p = head = VarTable[entry];
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
  VarTable[entry] = NULL;
}

varList *CreateCluster(varList *setV, int clus_nu, int varTableIdx, int minRegNu) {

  int i, j; 
  varList p;

  myprintf("Creating cluster %d from VarTable[%d]\n", 
      clus_nu, varTableIdx-1);
  setV = (varList *) realloc (setV, clus_nu * sizeof(varList));
  //assert(setV[clus_nu-1] == NULL);
  setV[clus_nu - 1] = malloc(sizeof(struct varLifeTime) * minRegNu);

  p = VarTable[varTableIdx - 1];
  i = 0;

  while (p != NULL) {
    setV[clus_nu - 1][i].varID     = p->varID;
    setV[clus_nu - 1][i].startTime = p->startTime;
    setV[clus_nu - 1][i].endTime   = p->endTime;
    i++;
    p = p->next;
  }

  for (j = i; j < minRegNu; j++)
    setV[clus_nu - 1][j].varID = -1;

  return setV;
}




/*******************************************************************************
 Algorithm 
 
 Find the time step where the number of living variables
 is maximum. The number is the minimum number of registers
 needed in a pipelined datapath. 

  for (t = 0; t <= delay; t++) {
    for each var node in the DFG {
      if a var's lifetime >= minII
        record the var in Vmax
        regNu++;
      else 
        min_end = min_start = - ceil(start / minII);  
        max_end = max_start = (delay - start) / minII 

        for (i = min_start; i <= max_start ; i++)
            pstart = start + minII * i 
            pend   = end   + minII * i
            if (t >= pstart && t < pend) 
              record the var in Vmax
              regNu++;
              break;
    }
    if (RegNu > maxRegNu)
      maxRegNu = RegNu;
   }

   Functions args
   head :         head of the variable list
   entry:         entry in the Var Table
   delay:         total time steps
   maxRegNu:      max number of vars
   maxTime:       the time step where there are max number of vars
*******************************************************************************/
void FindMaxVarSet(varList head, int entry, int delay, int *maxRegNu, int *maxTime) {

  int regNu;
  int t, m, n, i, j;
  int varID, start, end;
  int pstart, pend;
  int min_start; 
  int min_end;
  int max_start;
  int max_end; 
  varList p;

  // setup two lists for ping-pong operation
  CreateVarTable(entry); 
  CreateVarTable(entry + 1); 

  for (t = 0; t <= delay; t++) {

    regNu = 0;

    //---------------------------------------------------------------------------
    // For each time step, calculate the number of variables from Varlist 
    // which covers it.
    //---------------------------------------------------------------------------
    p = head;
    while (p != NULL) {
      varID = p->varID;
      start = p->startTime;
      end   = p->endTime;
      //printf("varID %d <%d %d>: ", varID, start, end);

      if (end - start >= minII) {
        //printf("time=%d %d > minII\n", t, end-start);
        VarTableInsert(entry + 1, p);
        regNu++;
      }
      else {
        min_start = - (start / minII + (start % minII) ? 1 : 0); 
        min_end   = min_start;

        max_start = (delay - start) / minII;
        max_end   = max_start; 

        // find lifetime which overlaps with time step t 
        for (i = min_start; i <= max_start; i++) {
          pstart = start + minII * i;
          pend   = end   + minII * i;
          //==============================================================
          // Find all the overlapping variables for each time step. 
          //
          // Note the pipelined nature of each variables must be taken
          // into account.
          //
          // To match the maxRegNu result with that from LEA, regNu(i.e.
          // a new register) is not added when the time step is equal 
          // to pend. The findings is discovered by the conflict check in LEA. 
          //
          //==============================================================
          if (t >= pstart && t < pend) {
            //printf("time=%d <%d %d> factor=%d\n", t, pstart, pend, i);
            regNu++;
            VarTableInsert(entry + 1, p);
            break;
          }
        }
        // the var's lifetime has no overlap with the time step t.
        //if (i == max_start + 1) printf("\n");
      }
      p = p->next;
    }

    //printf("time=%d regNu = %d\n\n", t, regNu);

    if (regNu > *maxRegNu) {
      *maxRegNu = regNu;
      *maxTime  = t;

      FreeVarTableEntry(entry - 1); // free at entry - 1

      p = VarTable[entry];
      while (p != NULL) { 
        VarTableInsert(entry, p); // insert at entry - 1
        p = p->next;
      }
    }

    FreeVarTableEntry(entry); // free at VarTable[entry]
  }

  // Debug
  myprintf("At time step %d Max pipeline reg %d\n", *maxTime, *maxRegNu);
}

