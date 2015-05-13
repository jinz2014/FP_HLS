//****************************************************************************
//
// WBM steps:
//
// 1. Use LEA to calcuate the min register number (minRegNu). During LEA, 
//    all the variables (variable list) assigned to a register (ri) are 
//    put in VarTable[i]
//
// 2. Divide the variables in VarTable into clusters. The number of 
//    clusters is the max length of a variable list in the VarTable
//    e.g. r0 :  v1  v3
//         r1 :  v2  v4
//         r2 :  v5   
//         r3 :  v6  
//         r4 :  v7  v8  
//    cluster 0 includes variables v1, v2, v5, v6, v7
//    cluster 1 includes variables v3, v4, v8
//     
// 3. Assign each variable in cluster 0 to a different register
//    e.g.  v1 - r1, v2 - r2, ... , v7 - r5
//
// 4. Check the life time of each variable in cluster i (i > 0) with
//    that of every variable that has been assigned to the register.
//    The conflict checks are the same as those implemented in LEA.
// 
// 5  If there is no conflict then the variable can be potentially 
//    assigned to the register 
//  
// 6  For each variable that could be potententially assigned to a register
//    (e.g. represented as edge vj - ri), construct a weight for the edge
//    based on the weight function defined. All edge weights are put in
//    a cost matrix (size is minRegNu x minRegNu)
//  
// 7  Use Hungarian method to find the minimum weight sum of all the edges
//    The matching array is used to save the matching results.
//
// 8  The matching array contains all the assignments of variables & registers
//    pair in the current cluster
//
// Note: Similar to the VarTable in LEA, all the variables (variable list) 
//       assigned to a register (ri) are put in VarTable[i] in WBM allocation
//****************************************************************************
#include <stdio.h> 
#include <stdlib.h> 
#include <assert.h> 
#include "Schedule.h"
#include "PortSchedule.h"
#include "WBM.h"

//--------------------------------------------------------------------
// Bipartite Matching for register sharing in a pipelined data path 
//--------------------------------------------------------------------
int WBM3 (int *Map) {

  int i, j, k, m, n, x, y;
  int x1, x2, x3;
  int conflict, conflict_nu, no;
  int timeV, timeR;
  int setV_startTime, setV_endTime;
  int setR_startTime, setR_endTime;
  int varID;

  int clus_nu = 0;
  int var_nu;
  int reg_nu;
  int first;
  int update;
  int src, dest;
  int tmp;

  int *cost;
  int *match;
  varList *setV = NULL;
  varList *p;
  varList List_head = NULL;
  varList currVar;
  varList head;

  //===============================================================
  // Get min number of registers with LEA
  // and reset Map after calling LEA 
  //===============================================================
  const int minRegNu = LeftEdgeRegisterBinding(Map);

  myprintf("LEA Map[%d]: ", minRegNu);
  for (i = 0; i < NODE_NU; i++) 
    myprintf("(v%d r%d) ", i, Map[i]);
  myprintf("\n");

#ifdef WBM

  for (i = 0; i < NODE_NU; i++) Map[i] = -1;

  //===============================================================
  // Divide variables into clusters
  //===============================================================
  p = (varList *) malloc(sizeof(varList) * minRegNu);

  // Access VarTable with p[i]
  for (i = 0; i < minRegNu; i++) p[i] = VarTable[i];

  setV = NULL;

  do {
    clus_nu++;
    setV = (varList *) realloc (setV, clus_nu * sizeof(varList));
    myprintf("cluster number is %d\n", clus_nu);
    setV[clus_nu - 1] = NULL;

    first = 1;

    for (i = 0; i < minRegNu; i++) {

      if (p[i] != NULL) {
        
        if (first) {
          setV[clus_nu - 1] = malloc(sizeof(struct varLifeTime) * minRegNu);
          first = 0;
          if (i > 0) { // Careful
            for (j = 0; j < i; j++) setV[clus_nu - 1][j].varID = -1;
          }
        }

        setV[clus_nu - 1][i].varID     = p[i]->varID;
        setV[clus_nu - 1][i].startTime = p[i]->startTime;
        setV[clus_nu - 1][i].endTime   = p[i]->endTime;

        p[i] = p[i]->next;
      }
      else if (setV[clus_nu-1] != NULL) {
        setV[clus_nu - 1][i].varID = -1;
      }
    }
  } while (setV[clus_nu - 1] != NULL);

  free(p);

  //===============================================================
  // Free the VarTable from LEA and recreate VarTable based on WBM
  //
  // For VarTable insertion, we get the actual parameter from VarList 
  // element. So create VarList also.
  //===============================================================

  FreeVarTable(minRegNu);
  for (i = 0; i < minRegNu; i++) CreateVarTable(i + 1);
  List_head = CreateVarList(List_head);

  //===============================================================
  // END Divide variables into clusters
  //===============================================================

  int clusterNu = clus_nu - 1; 

  //--------------------------------------------------------------
  // Dump cluster division results 
  //--------------------------------------------------------------
  for (i = 0; i < clusterNu; i++) {
    myprintf("cluster %d :", i);
    for (j = 0; j < minRegNu; j++) {
      if (i == 0) 
        myprintf(" (r%d v%d)", j, setV[i][j].varID); // (register, varID) pair
      else if (setV[i][j].varID != -1)
        myprintf(" (r?  v%d)", setV[i][j].varID);    // register not allocated yet
    }
    myprintf("\n");
  }

  //=============================================================
  // Allocate registers for cluster 0 variables
  //
  // Also Find VarID in VarList and put (VarID, register number)
  // into the VarTable
  //=============================================================
  for (i = 0; i < minRegNu; i++) {
    varID = setV[0][i].varID;
    Map[varID] = i + 1;

    // Get the corresponding varID from VarList
    currVar = List_head;
    while (currVar != NULL) {
      if (currVar->varID  == varID)
        break;
      else 
        currVar = currVar->next;
    }

    // Insert the VarID into VarTable
    VarTableInsert(i + 1, currVar);
  }


  //===============================================================
  // Setup initial weight for each register in set R(i.e. cluster 0)
  //===============================================================
  var_struct_ptr var_MUXR_table = malloc (sizeof(var_struct) * minRegNu); 
  var_struct_ptr var_MUXP_table = malloc (sizeof(var_struct) * minRegNu); 

  for (i = 0; i < minRegNu; i++) {

    //------------------------------------------------------------------
    // MUXR
    //
    //--------------------------------------------------------------------
    // case 1.  if var_nu = 0, create a new var regardless of node type
    // case 2.  No increase of MUXR for two input-port nodes which have 
    //          the same dest and the same physical port 
    // case 3.  An increase of MUXR when an input port and a operator nodes
    //          have the same dest
    // case 4.  An increase of MUXR when two operator nodes have 
    //          the same dest and
    //          4.1 different op
    //          4.2 same op with different resource number
    //          4.3 No increase of MUXR same op with the same resource number
    //--------------------------------------------------------------------

    var_MUXR_table[i].var_list = NULL;
    var_MUXR_table[i].var_nu   = 0;

    varID = setV[0][i].varID; 

    myprintf("MuxR: r%d <- node=%d Op=%c\n", i, varID, GetOpSym(DFG[varID]->op));

    TableUpdateMUXR(var_MUXR_table, i, varID);

    //------------------------------------------------------------------
    // MUXP
    //------------------------------------------------------------------
    var_MUXP_table[i].var_list = NULL;
    var_MUXP_table[i].var_nu   = 0;

    TableUpdateMUXP(var_MUXP_table, i, varID);
    
  } // for (i = 0; i < minRegNu; i++) 


  myprintf("===========================\n");
  myprintf("Dump var MUXR table\n", i);
  myprintf("===========================\n");
  myprintf("reg#   (var op op#)\n");
  for (i = 0; i < minRegNu; i++) {
    myprintf("r%d : ", i);
    if (var_MUXR_table[i].var_list != NULL) {
      for (n = 0; n < var_MUXR_table[i].var_nu; n++) {
        myprintf("(%d %c %d) ", (var_MUXR_table[i].var_list[n]).varID,
            GetOpSym((var_MUXR_table[i].var_list[n]).fuOp), 
                           (var_MUXR_table[i].var_list[n]).fuNu); 
      }
    } else myprintf("null");
    myprintf("\n");
  }

  myprintf("===========================\n");
  myprintf("Dump var MUXP table\n", i);
  myprintf("===========================\n");
    myprintf("reg#   (var op op#)\n");
  for (i = 0; i < minRegNu; i++) {
    myprintf("r%d : ", i);
    if (var_MUXP_table[i].var_list != NULL) {
      for (n = 0; n < var_MUXP_table[i].var_nu; n++) {
        myprintf("(%d %c %d) ", (var_MUXP_table[i].var_list[n]).varID,
            GetOpSym((var_MUXP_table[i].var_list[n]).fuOp), 
                           (var_MUXP_table[i].var_list[n]).fuNu); 
      }
    } else myprintf("null");
    myprintf("\n");
  }

  //===============================================================
  // END Setup initial weight for each register in set R (cluster0)
  //===============================================================


  //===============================================================
  // Setup 1-Dim cost matrix
  //===============================================================
  cost  = (int *) malloc (sizeof(int) * minRegNu * minRegNu); 

  //===============================================================
  // Setup WBM matrix
  //===============================================================
  //match = (int *) malloc (sizeof(int) * 2 * minRegNu);
  match = (int *) malloc (sizeof(int) * minRegNu);


  //===============================================================
  // Iteratively assign all variables in a cluster to the registers
  //===============================================================

   for (clus_nu = 1; clus_nu < clusterNu; clus_nu++) {

     //------------------------------------------------------------
     // for each cluster, initialize the cost matrix
     //------------------------------------------------------------
     for (i = 0; i < minRegNu; i++)
       for (j = 0; j < minRegNu; j++)
         cost[i * minRegNu + j] = 0;

     //-----------------------------------------------------------
     // For each possible variable 'vj' in set V
     //-----------------------------------------------------------
     for (j = 0; j < minRegNu; j++) {

       if (setV[clus_nu][j].varID != -1) {

         // vj's life time
         setV_startTime = setV[clus_nu][j].startTime; 
         setV_endTime   = setV[clus_nu][j].endTime;

         // Compared it with all variables assigned to ri 
         for (i = 0; i < minRegNu; i++) {

           no = 0;

           myprintf("**************************************************\n");
           myprintf("cluster %d : v%d <%d %d> can be bound to r%d ?\n", 
                     clus_nu, setV[clus_nu][j].varID, 
                     setV_startTime, setV_endTime, i + 1);
           myprintf("**************************************************\n");

           for (k = 0; k < NODE_NU; k++) {

             if (Map[k] == i + 1) {

              conflict = 0;

               head = VarTable[i];
               while (head != NULL) {
                 if (head->varID == k) break;
                 head = head->next;
               }

               assert( head != NULL);

               // ri's life time
               setR_startTime = head->startTime;
               setR_endTime   = head->endTime;

               myprintf("cluster %d : v%d <%d %d> can be bound to r%d: v%d <%d %d> ?", 
                   clus_nu,
                   setV[clus_nu][j].varID , setV_startTime, setV_endTime, 
                   i + 1, k, setR_startTime, setR_endTime);

               //------------------------------------------------
               // There is a weighted edge if and only if 
               // vi in SetV ( SetV[i] | 1 <= i < clusterNu) ) 
               // can be assigned to rj in SetR (SetV[0])
               // If there is conflict, the corresponding entry is
               // a BIG number
               //------------------------------------------------

               // 1. time range comparison same as LEA
               for (timeV = setV_startTime; timeV < setV_endTime; timeV++) {
                 for (timeR = setR_startTime; timeR < setR_endTime; timeR++) {
                   if (abs(timeV - timeR) % minII) 
                     continue; 
                   else {
                     conflict++;
                     no = 1;
                     break;
                   }
                 }
                 if (conflict) {
                   break; // break outer for-loop
                 }
               }

               if (conflict) {
                 myprintf(" no1\n");
                 cost[i * minRegNu + j] = BIG;
                 no = 1;
                 continue;
               }

               // 2. end time comparison same as LEA
               if (!(abs(setV_endTime - setR_endTime) % minII)) {
                 cost[i * minRegNu + j] = BIG;
                 myprintf(" no2\n"); // cannot be shared
                 conflict++;
                 no = 1;
                 continue;
               }

               // 3. overlapping check as LEA after detection of a bug
               //    e.g. in the course of WBM 
               // 0--1
               // 4--5
               // 2--3
               // 3--5 *conflict undetected by the conflict checks in LEA*
               if (setV_startTime < setR_startTime && 
                   setV_endTime < setR_endTime     && 
                   setV_endTime > setR_startTime   ||

                   setV_startTime < setR_endTime  &&
                   setV_startTime > setR_startTime  &&
                   setV_endTime > setR_endTime) {

                 conflict++;
                 cost[i * minRegNu + j] = BIG;
                 myprintf(" no3\n"); // cannot be shared
                 continue;
               }
               myprintf(" yes\n"); // cannot be shared
             }
           }
           myprintf(" %s\n", no ? "no" : "yes");
           conflict_nu += no;
         }
         if (conflict_nu == minRegNu) {
           myprintf("-------------------------------------------------\n");
           myprintf("Error v%d cannot be assigned to any register!!\n", setV[clus_nu][j].varID);
           myprintf("-------------------------------------------------\n");

           FreeVarTable(minRegNu);
           for (i = 0; i < clusterNu; i++) {
             for (j = 0; j < minRegNu; j++) {
               if (setV[i][j].varID != -1) {
                 List_head = VarListDelete(List_head, setV[i][j].varID);
               }
             }
           }

           for (i = 0; i < minRegNu; i++) {
             if (var_MUXR_table[i].var_list != NULL) 
               free(var_MUXR_table[i].var_list);
             if (var_MUXP_table[i].var_list != NULL) 
               free(var_MUXP_table[i].var_list);
           }

           free(var_MUXR_table);
           free(var_MUXP_table);

           for (i = 0; i < clusterNu; i++) free(setV[i]);

           free(setV);
           
           free(match);

           free(cost);

           return -2;  
         }
       } else {
         // If there are fewer than "minRegNu" registers in a cluster
         // then any remaining entry which is not assigned to a variable
         // is a BIG number
         for (k = 0; k < minRegNu; k++) // column i updates
           cost[k * minRegNu + j] = BIG;
       }
     }

     //myprintf("Before computing weight: check element value of cost matrix\n");
     //PrintMatrix(cost, minRegNu, minRegNu);

     
    //==============================================================================
    // Reference MUXR/P tables to update cost matrix with 
    // weight function (x1 + 100*x2 + x3)
    //==============================================================================

    for (j = 0; j < minRegNu; j++) {
      for (i = 0; i < minRegNu; i++) {

        if (cost[i * minRegNu + j] != BIG && setV[clus_nu][j].varID != -1) {

          //-----------------------------------------------------------------------
          // size of MUXR(r) IF vj is assigned to ri
          //-----------------------------------------------------------------------
          myprintf("MuxR: calculating edge weight (r%d v%d)\n", i, setV[clus_nu][j].varID);

          var_nu = var_MUXR_table[i].var_nu;
          tmp = var_nu;
          myprintf("x1 initial weight = %d\n", tmp);

          k = setV[clus_nu][j].varID; // k is vj

          //-------------------------------------------------------------------------
          // compare the node with var_list node
          // Two variables which are output operands of different operations that can
          // not be performed by the same function unit should be bound to different
          // registers even if there lifetime don't overlap
          //
          // In other words, a small weight is assigned if the two output operands of
          // different operations (including nop) can be performed by the same 
          // function unit
          //-------------------------------------------------------------------------
          for (n = 0; n < var_nu; n++) {
            if ((var_MUXR_table[i].var_list[n]).fuOp == DFG[k]->op) {
              varID = (var_MUXR_table[i].var_list[n]).varID;
              if (DFG[k]->op == nop && GetInputPortNu(k) != GetInputPortNu(varID) || 
                  DFG[k]->opResourceNu != (var_MUXR_table[i].var_list[n]).fuNu) {
                tmp++;
              } 
            } else {
              tmp++;
            }
          }
          
          x1 = tmp;
          myprintf("x1 weight = %d\n", x1);

          //-----------------------------------------------------------------------
          // 0 if the FU producing vj already drove register ri before this register
          // binding iteration; 1 otherwise.
          //-----------------------------------------------------------------------
            
          x2 = 1;
          for (n = 0; n < var_MUXR_table[i].var_nu; n++) {
            if ((var_MUXR_table[i].var_list[n]).fuOp == DFG[k]->op) {
              varID = (var_MUXR_table[i].var_list[n]).varID;
              if (DFG[k]->op == nop && GetInputPortNu(k) == GetInputPortNu(varID) ||
                 (var_MUXR_table[i].var_list[n]).fuNu == DFG[k]->opResourceNu)
              x2 = 0;
              break;
            }
          }

          myprintf("x2 = %d\n", x2);

          //-----------------------------------------------------------------------
          // Find the node whose source is vj and then compare the node's operation
          // and resource number with all those of the variables already assigned
          // in MUXP table. 
          //-----------------------------------------------------------------------
          x3 = 0;
          for (x = 0; x < NODE_NU; x++) {
            if (DFG[x]->opSrc[0] >= 0) { 
              for (m = 0; m < 2 - DFG[x]->opConst; m++) {
                if (DFG[x]->op != nop && DFG[x]->opSrc[m] == k) {
                  for (n = 0; n < var_MUXP_table[i].var_nu; n++)
                    //-------------------------------------------------------------------------
                    // Two variables which are input operands of different operations
                    // that can be performed by a same functional unit fu, should be bound
                    // to the same register if possible because those operations have a chance
                    // to be assigned to fu;
                    // 
                    // In other words, a small weight is assigned to an edge if the two input
                    // operands of different operations can be performed by the same function unit
                    // 
                    //-------------------------------------------------------------------------
                    if (!((var_MUXP_table[i].var_list[n]).fuNu == DFG[x]->opResourceNu && 
                          (var_MUXP_table[i].var_list[n]).fuOp == DFG[x]->op &&
                          (var_MUXP_table[i].var_list[n]).varID != x)) // different nodes
                      x3++;
                }
              }
            }
          }
          myprintf("x3 = %d\n", x3);

          cost[i * minRegNu + j] = x1 + 100 * x2 + x3; 
        }
      }
    }

    //myprintf("After computing weight: check element value of cost matrix\n");
    //PrintMatrix(cost, minRegNu, minRegNu);

    //brute_force_assignment(match, cost, minRegNu);
    hungarian_method(match, cost, minRegNu);

    myprintf("dump match array\n");
    for (i = 0; i < minRegNu; i++) {
      myprintf("%d ", match[i]);
    }
    myprintf("\n");

    //=============================================================
    // Allocate registers for variables from cluster 1 to clusterNu 
    // Also put (VarID, register number) into the VarTable
    //=============================================================
    for (j = 0; j < minRegNu; j++) {

       varID = setV[clus_nu][j].varID;

       if ( varID != - 1) {

         for (i = 0; i < minRegNu; i++) {
           //if (j == match[i] - minRegNu) { // find assignment of ri to vj
           if (j == match[i]) { // find assignment of ri to vj

             reg_nu = i;

             Map[varID] = reg_nu + 1;

             currVar = List_head;
             while (currVar != NULL) {
               if (currVar->varID  == varID)
                 break;
               else 
                 currVar = currVar->next;
             }

             VarTableInsert(reg_nu + 1, currVar);

   //========================================================================
   // Update the entry of matched register in the MUXR/MUXP tables
   //========================================================================
             TableUpdateMUXR(var_MUXR_table, reg_nu, varID);

             TableUpdateMUXP(var_MUXP_table, reg_nu, varID);

           }
         }

       } // if (varID != -1)
    }

    myprintf("WBM Map: ");
    for (i = 0; i < NODE_NU; i++) 
      myprintf("(v%d r%d) ", i, Map[i]);
    myprintf("\n");

   } // for (clus_nu = 1; clus_nu < clusterNu; clus_nu++)

  //===============================================================
  // END Iteratively assign variables to registers
  //===============================================================

  // Free VarList
  for (i = 0; i < clusterNu; i++) {
    for (j = 0; j < minRegNu; j++) {
      if (setV[i][j].varID != -1) {
        //printf("Deleting %d\n",  setV[i][j].varID);
        List_head = VarListDelete(List_head, setV[i][j].varID);
      }
    }
  }

  // Take a look at WBM-based VarTable
  for (i = 0; i < minRegNu; i++) {
    myprintf("VarTable shared register %d:\n", i + 1);
    VarListPrint(VarTable[i]);
  }

  //===============================================================
  // Free up resources
  //===============================================================
  for (i = 0; i < minRegNu; i++) {
    if (var_MUXR_table[i].var_list != NULL) 
      free(var_MUXR_table[i].var_list);
    if (var_MUXP_table[i].var_list != NULL) 
      free(var_MUXP_table[i].var_list);
  }

  free(var_MUXR_table);
  free(var_MUXP_table);

  for (i = 0; i < clusterNu; i++) free(setV[i]);

  free(setV);
  
  free(match);

  free(cost);
#endif // WBM

  return minRegNu;  
}


