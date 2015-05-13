//****************************************************************************
//
// WBM steps:
//
// Step 1 -- 5 are also described in CreateWBMCluster.c
//
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
//     
// Step 6 and after are described in this file
//
// 6. Allocate a register set of |Vmax| registers (SetV[0]) for each variable in Vmax
//    Assign each variable to a different register
//    e.g.  v1 - r1, v2 - r2, ... , v7 - r5
//
// 7. Check the life time of each variable in cluster i (i > 0) with
//    that of every variable that has been assigned to the register.
//    The conflict checks are the same as those implemented in LEA.
// 
// 8  If there is no conflict between the variable and all the variables that have
//    been assigned to a register, then the variable can be potentially assigned 
//    to this register. 
//
//    NOTE: Since a variable cannot possibly be assigned to ANY register during the 
//    variable-register assignment iterations, I used a full permutation sequence to
//    change the order of cluster selection and redo the assignment. 
//    If it still couldn't work after trying all the permutations, 
//    use LEA register allocation.
//  
// 9  For each variable that could be potententially assigned to a register
//    (e.g. represented as edge vj - ri), construct a weight for the edge
//    based on the weight function described below. All edge weights are put in
//    a cost matrix (size is minRegNu x minRegNu)
//  
// 10 Use Hungarian method to find the minimum weight sum of all the edges
//    The matching array is used to save the matching results.
//
// 11 The matching array contains all the assignments of variables & registers
//    pair in the current cluster.
//
// Note: Similar to the creation of VarTable in LEA, all the variables (variable list) 
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
int RegAlloc (int *Map, int delay) {

  int i, j, k, m, n, x, y;
  int x1, x2, x3;
  int conflict;
  int no;
  int timeV, timeR;
  int setV_startTime, setV_endTime;
  int setR_startTime, setR_endTime;
  int varID;
  int minRegNu;

  int conflict_nu;
  int clus_nu, clus;
  int var_nu;
  int reg_nu;
  int first;
  int update;
  int src, dest;
  int tmp;

  int *clusSeq;
  int *cost;
  int *match;

  int *matchArray;
  int regNu;

  int clusterNu;
  int Error1;
  int Error2;
  varList *setV;

  varList *p;
  varList currVar;
  varList head;
  var_struct_ptr var_MUXR_table;
  var_struct_ptr var_MUXP_table;

  varList List_head = NULL;
  int fail = 0;


#ifdef WBM

  //***************************************************************
  // weighted-bipartite matching
  //***************************************************************

TRY_AGAIN:

  clusterNu = 0;
  minRegNu  = 0;
  Error1    = 0;
  Error2    = 0;
  setV      = NULL;
  List_head = NULL;

  //===============================================================
  // Divide variables into clusters
  //===============================================================
  if (fail) { 
    setV = CreateWBMClusters(setV, delay, &minRegNu, &clusterNu);
  } else {
    setV = CreateWBMClustersMax(setV, delay, &minRegNu, &clusterNu);
  }

  //===============================================================
  // Dump cluster division results 
  //===============================================================
  for (i = 0; i < clusterNu; i++) {
    myprintf("cluster %d :", i);
    for (j = 0; j < minRegNu; j++) {
      if (setV[i][j].varID != -1)
        // ?: register not allocated yet
        myprintf(" (r?  v%d)", setV[i][j].varID); 
    }
    myprintf("\n");
  }

  //exit(1); // debug

  //===============================================================
  myprintf("END Cluster(Set) division\n");
  //===============================================================


  //===============================================================
  // Odd issues in WBM
  //
  // A variable cannot be assigned to any register in the register
  // set in the varible regsiter assignment iterations(see below).
  //
  // Let's use different select sequences besides the default
  // sequence(i.e. 1,2,...clusterNu-1).
  //
  //===============================================================
  clusSeq = (int *) malloc (sizeof(int) * (clusterNu - 1));

  for (clus = 1; clus < clusterNu; clus++)
    clusSeq[clus - 1] = clus; // Set default cluster select sequence

LOOP:

  //while (1) {

    for (i = 0; i < NODE_NU; i++) Map[i] = -1; //init Map

    //===============================================================
    // Create VarTable based on WBM
    //
    // For VarTable insertion, we get the actual parameter from VarList 
    // element. So create VarList also.
    //===============================================================
    for (i = 0; i < minRegNu; i++) CreateVarTable(i + 1);
    List_head = CreateVarList(List_head);

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
      assert(currVar != NULL);
      VarTableInsert(i + 1, currVar);
    }

    //===============================================================
    // Setup initial weight for each register in set R(i.e. cluster 0)
    //===============================================================
    var_MUXR_table = malloc (sizeof(var_struct) * minRegNu); 
    var_MUXP_table = malloc (sizeof(var_struct) * minRegNu); 

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
    match = (int *) malloc (sizeof(int) * minRegNu);

    //===============================================================
    // Iteratively assign all variables in a cluster to the registers
    //
    // A variable can be assigned to a register only if its lifetime 
    // does not overlap with the lifetimes of all the variables alreay
    // assigned to that register. 
    //===============================================================

     for (clus = 1; clus < clusterNu; clus++) {

        clus_nu = clusSeq[clus - 1];

       //------------------------------------------------------------
       // for each cluster, initialize the cost matrix
       //------------------------------------------------------------
       for (i = 0; i < minRegNu; i++)
         for (j = 0; j < minRegNu; j++)
           cost[i * minRegNu + j] = 0;

       //-----------------------------------------------------------
       // For each possible variable 'vj' in set V
       //-----------------------------------------------------------

       //-----------------------------------------------------------
       // When the number of variables in a cluster is more than
       // the number of unique registers that can be assigned to,
       // redo the WBM. 
       //
       // MatchArray[0] records the number of variables
       // MatchArray[1] records the number of unique registers 
       // MatchArray[2..n] records the register number of each 
       // unique register(i.e. "i")
       //-----------------------------------------------------------
       matchArray = (int *) calloc (2, sizeof(int));

       for (j = 0; j < minRegNu; j++) {

         if (setV[clus_nu][j].varID != -1) {

           matchArray[0]++;

           // Reset vj conflict counter
           conflict_nu = 0;

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

             if (!no) {

               regNu = matchArray[1];

               for (n = 1; n <= regNu; n++)
                 if (i == matchArray[n+1]) 
                   break;

               if (n > regNu) {
                 matchArray[1] = ++regNu;
                 matchArray = (int *) realloc (matchArray, sizeof(int) * (regNu + 2));
                 matchArray[regNu + 1] = i;
               }
             }
           }

           if (conflict_nu == minRegNu || matchArray[0] > matchArray[1]) {

             if (conflict_nu == minRegNu) {
               Error1++;
               myprintf("Error v%d cannot be assigned to any register!!\n",
                   setV[clus_nu][j].varID);
             }
             if (matchArray[0] > matchArray[1]) {
               myprintf("Error %d variables cannot be assigned to %d register!!\n",
                   matchArray[0], matchArray[1]);
               Error2++;
             }

             FreeVarTable(minRegNu);
             for (i = 0; i < clusterNu; i++) {
               for (j = 0; j < minRegNu; j++) {
                 if (setV[i][j].varID != -1) {
                   List_head = VarListDelete(List_head, setV[i][j].varID);
                 }
               }
             }

             for (i = 0; i < minRegNu; i++) {
               if (var_MUXR_table[i].var_list != NULL) {
                 free(var_MUXR_table[i].var_list);
                 var_MUXR_table[i].var_list = NULL;
               }

               if (var_MUXP_table[i].var_list != NULL) {
                 free(var_MUXP_table[i].var_list);
                 var_MUXP_table[i].var_list = NULL;
               }
             }

             free(var_MUXR_table);
             free(var_MUXP_table);
             
             free(match);

             free(cost);

             free(matchArray);

             //if (Error1 == fac(clusterNu - 1) || Error2 == fac(clusterNu - 1)) {
             if (Error1 == 3000 || Error2 == 3000) {
               for (i = 0; i < clusterNu; i++) free(setV[i]);
               free(setV);

               free(clusSeq);

               fail++;

               printf(" All permutations of cluster selection fail %d time(s) !!\n", fail);

               if (fail == 1) 
                 //goto TRY_AGAIN;
                return -2; 
             }

             // Let's redo the WBM with a different permutation of selection
             perm(clusSeq, clusterNu - 1);
             goto LOOP;

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
            myprintf("MuxR: calculating edge weight (r%d v%d)\n", i + 1, setV[clus_nu][j].varID);

            var_nu = var_MUXR_table[i].var_nu;
            tmp = var_nu;
            myprintf("x1 initial weight = %d\n", tmp);

            k = setV[clus_nu][j].varID; // k is vj

            //-------------------------------------------------------------------------
            // compare the node with var_list node
            // Two variables which are output operands of different operations that can
            // not be performed by the same function unit should be bound to different
            // registers even if there lifetime intervals don't overlap
            //
            // In other words, a small weight is assigned if the two output operands of
            // different operations (including nop) can be performed by the same 
            // function unit
            //-------------------------------------------------------------------------
            for (n = 0; n < var_nu; n++) {
              // same operation
              if ((var_MUXR_table[i].var_list[n]).fuOp == DFG[k]->op) {
                varID = (var_MUXR_table[i].var_list[n]).varID;
                // different input port assignment
                if (DFG[k]->op == nop && GetInputPortNu(k) != GetInputPortNu(varID) || 
                // same operations different FUs
                    DFG[k]->opResourceNu != (var_MUXR_table[i].var_list[n]).fuNu) {
                  tmp++;
                } 
              } else { // different operations
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
                      // that can be performed by the same functional unit FU, should be bound
                      // to the same register if possible because those operations have a chance
                      // to be assigned to this FU;
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

      //=============================================================
      // Solve minimum weight matching using almost off-the-shelf codes
      //=============================================================
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

      free(matchArray);

     } // for (clus = 1; clus < clusterNu; clus++)

     //break; 

   //} // while(1)

  //===============================================================
  // END Iteratively assign variables to registers
  //===============================================================

  // Free VarList
  for (i = 0; i < clusterNu; i++) {
    for (j = 0; j < minRegNu; j++) {
      if (setV[i][j].varID != -1) {
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

  free(clusSeq);


#elif defined(LEA)
  //***************************************************************
  // left-edge allocation
  //***************************************************************

  minRegNu = LeftEdgeRegisterBinding(Map);

  myprintf("LEA Map[%d]: ", minRegNu);
  for (i = 0; i < NODE_NU; i++) 
    myprintf("(v%d r%d) ", i, Map[i]);
  myprintf("\n");

#else
  //***************************************************************
  // Minimum FU with maximum number of registers which are equal
  // to the number of nodes in the DFG
  //
  // if there is no nopc node, reg_index equals i for each for loop
  //***************************************************************

  minRegNu = NODE_NU;
  List_head = CreateVarList(List_head);
  int reg_index;

  for (reg_index = 0, i = 0; i < minRegNu; i++) {

    if (DFG[i]->op != nopc) {

      //CreateVarTable(i + 1);
      varID = i; // i because it will be compared with the item in VarList 
      //Map[varID] = i + 1;

      reg_index++;
      CreateVarTable(reg_index);
      Map[varID] = reg_index;

      // Get the corresponding varID from VarList
      currVar = List_head;
      while (currVar != NULL) {
        if (currVar->varID  == varID)
          break;
        else 
          currVar = currVar->next;
      }

      // Insert the VarID into VarTable
      assert (currVar != NULL);
      //VarTableInsert(i + 1, currVar);
      VarTableInsert(reg_index, currVar);

      // Delete it
      List_head = VarListDelete(List_head, varID);

    }
  }
  minRegNu = reg_index;

#endif

  return minRegNu;  
}
