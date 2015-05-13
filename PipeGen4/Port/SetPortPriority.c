#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "Schedule.h"
#include "PortSchedule.h"


#ifdef PORT_CONSTRAINT
// p[]: PortPriority
// n[]: PortAssignment

//==========================================================================
// This version of constraint_sort is different from SetPortPriority.c.orig
// in that the function returns just one valid port assignment instead of
// enumeration of all the valid port assignment for a given port
// orderings.
//
// The purpose is to decouple constrained-sort function with the port-directed 
// schedule. Otherwise, the port-directed schedule has to be called
// inside the constraint-sort function. 
//
// So this function, as a independent module, can be used with other 
// shedulings without being coupled too tightly with the schedulings.
//
// Notes
// Use static int variables to keep the data for every call of this
// function after the first call of it.
//
// Return
// true when a valid port assignment is found;
// false otherwise
//
// Function
// ------------------------------------------------------------------
//
// 1. When the number of physical ports are not full(i.e. not all of them
//    are accepting input data (Kport1, Xport1), we use indices Kport1 and Xport1
//    In this case, there is a one-to-many assignment between the 
//    logical port and the physical port
//
// 2. When the number of physical ports are full(i.e. all of them are  
//    accepting input data, we use indices Kport2 and Xport2
//    In this case, there is a one-to-one assignment between the 
//    logical port and the physical port. NOTICE there is just one 
//    way for assignment. 
//
//    
// e.g. 
//    Suppose the port partition is 2X + 2K (two X ports and two K ports)
//    and the port ordering for 5 input data is as follows:
//
//      cycle1   cycle2 
//    X   1        1
//    K   1        2
//
//    Since there are 4 ports: P1, P2, P3, P4
//
//    In cycle1: the X logical port can be assigned to P1 or P2 
//               the K logical port can be assigned to P3 or P4 
//    In cycle2: the X logical port can be assigned to P1 or P2 
//               the K logical ports are assigned to P3 and P4 
//               Note the order of the two logical K ports(K1, K2) are determined
//               by the port priority. We assume there is only one way for
//               port assignement(i.e. P4 and P3 is not considered as another
//               assignment for K1 and K2). 
//
//======================================================================

int constraint_sort(FILE *fp, int p[], int n[], int nu, 
                    int *Xconstraint, int *Kconstraint, 
                    int px, int pk, int first) {
  int i, k, m, x, y, z;
  int ksize, xsize; 
  int cnt;
  int Xport1, Kport1;
  int Xport2, Kport2;
  int flag;
  int find, nextfind;
  static int min, max, r;
  static int portXAssignCnt;
  static int portKAssignCnt;
  static int *Kp = NULL;
  static int *Xp = NULL;
  static int *port_perm = NULL;
  static int *portX_assign = NULL;
  static int *portK_assign = NULL;

  if (Xconstraint == NULL && Kconstraint == NULL) {

    //-----------------------------------------------------------------------
    // 1. Circuit is fully parallel : no priority (default port assignment)
    // 2. Circuit has one port: each element in PortAssignment is one
    // 3. Simple Schedule in which there is no port X and port K restriction
    //-----------------------------------------------------------------------
    for (i = 0; i < nu; i++) {
      //---------------------------------
      // assignment for 1 or max ports
      //n[i] = (p == NULL) ? i + 1 : 1; 
      //---------------------------------

      //---------------------------------
      // modular assignment for all ports
      //---------------------------------
      n[i] = i % PORT_NU + 1;
    }

    // Copy data from PortPriority to SortedPortPriority(p)
    if (p != NULL  && PortPriority != NULL)
      memcpy(p, PortPriority, sizeof(int) * nu);

    return 1;

  } else if (first) {

    //----------------------------------------------------------
    // Put X ports in Xp and K ports in Kp
    //----------------------------------------------------------
    for (i = 0, k = 0; i < nu; i++) {
      if (PORTS_CONSTRAINT[PortPriority[i]])
        k++;
    }

    ksize = k;
    xsize = nu - k;

    Kp = (int *) malloc(sizeof(int) * ksize);
    Xp = (int *) malloc(sizeof(int) * xsize);

    for (i = 0, x = 0, y = 0; i < nu; i++) {
      if (PORTS_CONSTRAINT[PortPriority[i]]) 
        Kp[x++] = PortPriority[i];
      else
        Xp[y++] = PortPriority[i];
    }

    //----------------------------------------------------------
    // Print logical port numbers
    //----------------------------------------------------------
    myprintf("constrainted sort Kp = "); PrintArray(Kp, ksize);
    myprintf("constrainted sort Xp = "); PrintArray(Xp, xsize);

    myprintf("Xconstrainted = "); PrintArray(Xconstraint, minII);
    myprintf("Kconstrainted = "); PrintArray(Kconstraint, minII);

    portX_assign = (int *) malloc(sizeof(int) * minII);
    portK_assign = (int *) malloc(sizeof(int) * minII);

    //==============================================================
    // If the number of X or K input ports at a given cycle is less 
    // than the total physical ports px or pk, then the port(s) can
    // be assigned to any combinations of the physical port numbers.
    //
    // We use portX(K)_assign[i] to indicate the above case.
    //==============================================================
    for (i = x = y = z = 0; i < minII; i++) {
      cnt = Xconstraint[i];
      if (cnt && cnt < px) { 
        portXAssignCnt += cnt;
        portX_assign[i] = 1;
      } else {
        portX_assign[i] = 0;
      }

      cnt = Kconstraint[i];
      if (cnt && cnt < pk) {
        portKAssignCnt += cnt;
        portK_assign[i] = 1;
      } else {
        portK_assign[i] = 0;
      }
    }

    //==============================================================
    // Now we know the length of port assignment array.
    // 
    // The value of each element in the assignment array is 
    // in the range [min, max]  
    //
    // Assume the ports assignment range is 
    // 1, 2, ..., px for port X
    //
    // px + 1, px + 2, ..., px + pk for port K
    //==============================================================
    r = portXAssignCnt + portKAssignCnt;

    if (r > 0) {
      port_perm = (int *) malloc (sizeof(int) * r);

      if (portXAssignCnt && portKAssignCnt) {
        min = 1;
        max = px + pk;
      }
      else if (portXAssignCnt) {
        min = 1;
        max = px;
      }
      else if (portKAssignCnt) {
        min = px + 1;
        max = px + pk;
      }

      for (i = 0; i < r; i++) {
        port_perm[i] = min;
      }
    }
  } // if (first)

  // if first != 0 and port_perm = NULL, then there was no port_perm
  // for the last port assignment.
  else if (port_perm == NULL) 
    return 0;

// Do-while loop to find a valid port assignment
  do {

    find = 0;
    nextfind = 0;

    //==============================================================
    // Use two sets of indices(Xport1, Kport1 and Xport2, Kport2) 
    // to access the array port_perm
    //
    // the port_perm length for port X is 
    // port_perm[0 ... portXAssignCnt] 
    //
    // the port_perm length for port K is 
    // port_perm[portXAssignCnt + 1 ... portXAssignCnt + portXAssignCnt] 
    //
    // Reset Xport1 and Kport1 for every new permutation
    // Reset Xport2 and Kport2 for every cycle in the new permutation
    //==============================================================
    Xport1 = 0; 
    Kport1 = (portXAssignCnt && portKAssignCnt) ? portXAssignCnt : 0; 

    for (i = x = y = z = 0; i < minII; i++) {

      Xport2 = 1;
      Kport2 = px + 1;

      cnt = Xconstraint[i];
      flag = 0;

      while (cnt > 0) {

        if (portX_assign[i]) {

          //------------------------------------------------------------------
          // Before physical port number assignment, 
          // select valid port assignment first
          //
          // c1. physical port numbers assigned in a given cycle are distinct
          // c2. physical port number is in the range [1, px] or [px+1, px+pk]
          // 
          // Note on conditional statement:
          // if (portXAssignCnt && portKAssignCnt && port_perm[Xport1] > px) 
          //
          // Because of min and max boundary set above, 
          //
          // if PortXAssignCnt == 0 then all elems in port_perm > px
          // if PortKAssignCnt == 0 then all elems in port_perm <= px
          // For the two condition cases, we don't need to compare against px.
          //
          // The same applies to check of physical port numbers of K
          //------------------------------------------------------------------
          if (Xconstraint[i] == 1) {
            if (portXAssignCnt && portKAssignCnt && port_perm[Xport1] > px) {
              flag = 1; 
              break;
            }
          } else {
            if (cnt == Xconstraint[i]) { // check once
              for (k = 0; k < Xconstraint[i]; k++) {
                // c2 
                if ( portXAssignCnt && portKAssignCnt && port_perm[Xport1+k] > px) {
                  flag = 1;
                  break;
                }
                // c1: Compare port_perm[k] with all the previous elements in port_perm
                for (m = 0; m < k; m++) {
                  if (port_perm[Xport1+m] == port_perm[Xport1+k]) {
                    flag = 1;
                    break;
                  }
                } // for
                if (flag) break;
              } // for
            }
          }
          if (flag) break;

          assert(port_perm[Xport1] <= px) ;
          n[x] = port_perm[Xport1];
          Xport1++;
        }
        else {
          n[x] = Xport2++; 
        }
        
        p[x++] = Xp[y++];
        cnt--;

      } // while

      if (flag) break; // break for (i = x = y = z = 0; i < minII; i++)

      cnt = Kconstraint[i];

      while (cnt > 0) {
        if (portK_assign[i]) {
          
          if (Kconstraint[i] == 1) {
            if (portXAssignCnt && portKAssignCnt && port_perm[Kport1] <= px) {
              flag = 1; 
              break;
            }
          } else {
            if (cnt == Kconstraint[i]) {
              for (k = 0; k < Kconstraint[i]; k++) {
                // c2:
                if (portXAssignCnt && portKAssignCnt && port_perm[Kport1+k] <= px) {
                  flag = 1;
                }
                // c1: Compare port_perm[k] with all the previous elements in port_perm
                for (m = 0; m < k; m++) {
                  if (port_perm[Kport1+m] == port_perm[Kport1+k]) {
                    flag = 1;
                    break;
                  }
                }
                if (flag) break;
              }
            }
          }

          if (flag) break;
          assert(port_perm[Kport1] > px) ;
          n[x] = port_perm[Kport1];
          Kport1++;
        }
        else {
          n[x] = Kport2++; 
        }

        p[x++] = Kp[z++];
        cnt--;
      }

      if (flag) break; 
    } // for (i = x = y = z = 0; i < minII; i++)

    //=================================================================
    // Manual orderings of p[], SortedPortPriority
    // Manual orderings of Xconstraint[]
    // Manual orderings of Kconstraint[]
    //=================================================================
    for (i = 0; i < 4; i++) // clL
      p[i] = 32 + i;

    for (i = 0; i < 4; i++) // clR
      p[i+4] = 36 + i;

    for (i = 0; i < 16; i++) // tiPL
      p[i+8] = 0 + i;

    for (i = 0; i < 16; i++) // tiPR
      p[i+24] = 16 + i;

    p[40] = 40; // lnScaler
    p[41] = 41; // numSites
    p[42] = 42; // scP

    for (i = 0; i < 12; i++)
      Xconstraint[i] = 0;

    for (i = 0; i < 10; i++)
      Kconstraint[i] = 4; 

    Xconstraint[10] = 2;
    Xconstraint[11] = 1;


    if (!flag) {
      myprintf("constrainted sort p = "); PrintArray(p, nu);
      myprintf("port assignment n = "); PrintArray(n, nu);

      //----------------------------------------------
      // Stats
      //----------------------------------------------
      myfprintf(fp, "Logical input port # =  "); 
      for (i = 0; i < nu; i++)
        myfprintf(fp, "%3d ", (p==NULL) ? n[i] : p[i]);
      myfprintf(fp, "\n");

      myfprintf(fp, "Physical input port # = "); 
      for (i = 0; i < nu; i++)
        myfprintf(fp, "%3d ", n[i]);
      myfprintf(fp, "\n");
      //----------------------------------------------
      //
      //----------------------------------------------

      myprintf("Logical input port # =  "); 
      for (i = 0; i < nu; i++)
        myprintf("%3d ", (p==NULL) ? n[i] : p[i]);
      myprintf("\n");

      myprintf("Physical input port # = "); 
      for (i = 0; i < nu; i++)
        myprintf("%3d ", n[i]);
      myprintf("\n");

      find = 1; // find a port assignment 
      nextfind = Next(min, max, r, port_perm);
      break;
    }
  } while (nextfind = Next(min, max, r, port_perm));

  if (nextfind  == 0) {
    free(Kp); Kp = NULL;
    free(Xp); Xp = NULL;
    free(portX_assign); portX_assign = NULL;
    free(portK_assign); portK_assign = NULL;
    if (r > 0) { 
      free(port_perm);
      port_perm = NULL;
    }
    min = 0; max = 0; r = 0;
    portXAssignCnt = 0;
    portKAssignCnt = 0;
  }
  return find;
}
#endif

// sort arrays a[st, nu) and p[st, nu) in decreasing order
void bubblesort(int a[], int p[], int st, int nu) {
  int i, j, temp;
  for (i = st; i < nu; i++)
    for (j = nu - 1; j > i; j--)
      if (a[j] > a[j-1]) {
        temp = a[j]; a[j] = a[j-1]; a[j-1] = temp;
        temp = p[j]; p[j] = p[j-1]; p[j-1] = temp;
      }
}

//=================================================================
// 
// Each element of the Priority array p[] stores the logical port
// number. The index of the array indicates the priority for the 
// logical port.
//
// Note it is the index that indicates the port priority, not the
// element value.
//=================================================================
void priority_sort(int a0[], int a1[], int p[], int t1[], int nu) {

  int i;
  int st = 0;
  int ed = 1;
  int flag = 0;
  int *t0 = (int *) malloc(sizeof(int) * nu);

  for (i = 0; i < nu; i++) t0[i] = a0[i]; // already sorted
  for (i = 0; i < nu; i++) t1[i] = a1[p[i]];

  myprintf("-t0 = "); PrintArray(t0, nu);
  myprintf("-t1 = "); PrintArray(t1, nu);

  // For a set of elements with the same value in the primary array,
  // sort decreasingly the corresponding elements in the secondary array
  for (i = 0; i < nu; i++) { // 0, 1, 2
    if (i + 1 < nu && t0[i] == t0[i + 1]) {
      ed++;
      flag = 1;
    }
    else {
      assert(st < nu);
      assert(ed <= nu);
      assert(ed >= st);
      if (flag) { bubblesort(t1, p, st, ed); flag = 0;}
      st = i + 1;
      ed = st + 1;
    }
  }
  myprintf("psort: a = "); PrintArray(t1, nu);
  myprintf("psort: p = "); PrintArray(p, nu);
  free(t0); 
}

#ifdef PORT_CONSTRAINT
void perm_sort(int p[], int nu) {
  int i, j, k, x;
  int ksize, xsize; 
  int cnt;
  int Kn, Xn;

  k = 0;

  for (i = 0; i < nu; i++) {
    if (PORTS_CONSTRAINT[i]) k++;
  }

  ksize = k;
  xsize = nu - k;

  int *Kp = (int *) malloc(sizeof(int) * ksize);
  int *Xp = (int *) malloc(sizeof(int) * xsize);
  int *Ks = (int *) malloc(sizeof(int) * ksize);
  int *Xs = (int *) malloc(sizeof(int) * xsize);

  // collect data for X and K ports respectively
  for (i = 0, k = 0, j = 0; i < nu; i++) {
    if (PORTS_CONSTRAINT[i]) 
      Kp[k++] = i;
    else
      Xp[j++] = i;
  }
  
  // initial permutation
  for (i = 0; i < xsize; i++) Xs[i] = i;
  //for (i = 0; i < ksize; i++) Ks[i] = i;
   
  // all permutations of sequence Xs and Ks
  Xn = fac(xsize);
  Kn = fac(ksize);

  // Test all port orderings
  for (x = 0; x < Xn; x++) {

    // reset Ks
    for (i = 0; i < ksize; i++) Ks[i] = i;

    for (k = 0; k < Kn; k++) {

      SetFullPortPriority(p, Xs, Ks, Xp, Kp, xsize, ksize);
      myprintf("perm_sort p = ");  PrintArray(p, nu);

      if (k == Kn) break;
      perm(Ks, ksize);
    }
    if (x == Xn) break;
    perm(Xs, xsize);
  }
}
#endif


#ifdef SAMPLE_TEST
int main () {
  int a0[4], a1[4], a2[4]; 
  int p[4];

  int *t1 = (int *) malloc(sizeof(int) * 4);
  int i;

  for (i = 0; i < 4; i++) p[i] = i;

  // A sample in which a0 has higher priority than a1, which than a2
  a0[0] = 4; a0[1] = 5; a0[2] = 5; a0[3] = 6; // Succ
  a1[0] = 2; a1[1] = 0; a1[2] = 4; a1[3] = 2; // Coverage 
  a2[0] = 2; a2[1] = 4; a2[2] = 1; a2[3] = 1; // Frequencey 

  printf("Show inputs arrays: \n");
  printf("a0 = "); PrintArray(a0, 4);
  printf("a1 = "); PrintArray(a1, 4);
  printf("a2 = "); PrintArray(a2, 4);

  //----------------------------------------------------------------
  //  Assign priority to each input port 
  //----------------------------------------------------------------

  //printf("Show the first sorted array and its indirect index: \n");
  bubblesort(a0, p, 0, 4);

  printf("Show the second sorted array and its indirect index: \n");
  priority_sort(a0, a1, p, t1, 4);

  printf("Show the third sorted array and its indirect index: \n");
  priority_sort(t1, a2, p, t1, 4);

  free(t1); 
  return 0;
}
#endif

