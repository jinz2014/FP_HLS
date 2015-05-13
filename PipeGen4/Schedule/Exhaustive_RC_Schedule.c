#include <stdlib.h> 
#include <stdio.h> 
#include <assert.h> 
#include "queue.h"
#include "Schedule.h"

// Source node
//const int SRC = -1;
const int P1  = -1;
const int P2  = -2;
const int P3  = -3;
const int P4  = -4;
const int P5  = -5;
const int P6  = -6;
const int P7  = -7;
const int P8  = -8;
const int P9  = -9 ;
const int P10 = -10;
const int P11 = -11;
const int P12 = -12;
const int P13 = -13;
const int P14 = -14;
const int P15 = -15;
const int P16 = -16;

// Sink node
const int SINK = -1;


//#include "pos.config"
//#include "sop.config"
//#include "sample.config"
//#include "random2.config"
//#include "random1.config"
//#include "fir16.config"
//#include "sample1.config"
//#include "ratelaw.config"
//#include "ratelaw1.config"

#ifdef PPBR
#include "ppbr.config"
#endif
#ifdef ORDBBR
#include "ordbbr.config"
#endif
#ifdef ORDUBR
#include "ordubr.config"
#endif
#ifdef ORDBUR 
#include "ordbur.config" 
#endif
#ifdef UCTR   
#include "uctr.config"   
#endif
#ifdef UMAR   
#include "umar.config"   
#endif
#ifdef UMR    
#include "umr.config"    
#endif
#ifdef UCTI   
#include "ucti.config"   
#endif
#ifdef UMAI   
#include "umai.config"   
#endif
#ifdef UMI    
#include "umi.config"    
#endif
#ifdef UNII   
#include "unii.config"   
#endif
#ifdef UUCI   
#include "uuci.config"   
#endif
#ifdef UUCR   
#include "uucr.config"   
#endif
#ifdef UAII   
#include "uaii.config"   
#endif
#ifdef UAR    
#include "uar.config"    
#endif
#ifdef UCII   
#include "ucii.config"   
#endif
#ifdef UCIR   
#include "ucir.config"   
#endif
#ifdef UAI    
#include "uai.config"    
#endif
#ifdef UHMR   
#include "uhmr.config"   
#endif
#ifdef UUR    
#include "uur.config"    
#endif


// conflict
struct conflict_table table[OP_NU]; 

int minII;
int *ASAP_slots, *ALAP_slots;
int *PortPriority;
int *NodePriority;

opNode **DFG;
queue **RQ, **PQ;
int **RAT, **PRAT;
Fu **MuxRAT;
ResourceList *FuRAT;
//ResourceList *StimTable;
int *StimSequenceTable;
varList *VarTable;
MuxSelList *MuxSelTable;
int MuxNuCnt;
//char vfilename[100];

// Schedule RAT  4/5/11
SRAT_FuType_Ptr *SRAT;


//----------------------------------------------------
// 
//----------------------------------------------------
int main () {

  int i, j, k, x;

  int ksize, xsize; 
  int Kn, Xn;

  enum operation op_type;

  int portConfigCnt = 3;

  FILE *fp = stderr;


  //-----------------------------------------------
  // Init Schedule Statistics
  //-----------------------------------------------
  ScheduleStats SS;
  ScheduleStatsPtr SSP = &SS;
  SSP->ASAP_min_delay = 10000;
  SSP->ASAP_max_delay = 0;

  SSP->ASAP_min_mux = 10000;
  SSP->ASAP_max_mux = 0;

  SSP->ASAP_min_reg = 10000;
  SSP->ASAP_max_reg = 0;

  SSP->ALAP_min_delay = 10000;
  SSP->ALAP_max_delay = 0;

  SSP->ALAP_min_mux = 10000;
  SSP->ALAP_max_mux = 0;

  SSP->ALAP_min_reg = 10000;
  SSP->ALAP_max_reg = 0;

  //----------------------------------------------------
  // Check constraints
  //----------------------------------------------------
  assert(RATE <= LATENCY); 

  //----------------------------------------------------
  // 
  //----------------------------------------------------
  CreateResource();

  CreateRAT();

  //----------------------------------------------------
  // 
  //----------------------------------------------------
  CreateDFG();

  PortPriority = (int *) malloc (sizeof(int) * MAX_PORT_NU);
  NodePriority = (int *) malloc (sizeof(int) * NODE_NU);

  for (i = 0; i < MAX_PORT_NU; i++) {
    PortPriority[i] = i;
  }

  for (i = 0; i < NODE_NU; i++) {
    NodePriority[i] = i;
  }

  int *path  = (int *) malloc (sizeof(int) * NODE_NU);
  SetPathLength(path);
  SetNodePriority(path, NodePriority, MAX_PORT_NU);
  free(path);

  k = 0;
  for (i = 0; i < MAX_PORT_NU; i++) {
    if (PORTS_CONSTRAINT[i]) k++;
  }

  ksize = k;
  xsize = MAX_PORT_NU - k;

  int *Ks = (int *) malloc(sizeof(int) * ksize);
  int *Xs = (int *) malloc(sizeof(int) * xsize);

  //=================================================
  // Start
  // collect data for X and K ports respectively
  //
  // When PORTS_CONSTRAINT[port_nu] = 1
  //   the input data goes to K port 
  // else
  //   the input data goes to X port 
  //=================================================

  int *Kp = (int *) malloc(sizeof(int) * ksize);
  int *Xp = (int *) malloc(sizeof(int) * xsize);

  for (i = 0, k = 0, j = 0; i < MAX_PORT_NU; i++) {
    if (PORTS_CONSTRAINT[i]) 
      Kp[k++] = i;
    else
      Xp[j++] = i;
  }

  PrintArray(Xp, xsize);
  PrintArray(Kp, ksize);
  //=================================================
  // 
  //=================================================

  
  //=================================================
  // Set initial permutation of X port
  // Later set initial permutation of K port
  //=================================================
  for (i = 0; i < xsize; i++) Xs[i] = i;
  //for (i = 0; i < ksize; i++) Ks[i] = i;
   
  // Xn, Kn are the total number of permutations of 
  // sequence Xs and Ks
  // e.g.
  // 1, 2, 3
  // 1, 3, 2
  // 2, 1, 3
  // 2, 3, 1
  // 3, 1, 2
  // 3, 2, 1
  // fac(3) =  6
  Xn = fac(xsize);
  Kn = fac(ksize);
  
  // XComb & KComb are predefined for each benchmark
  printf("Port permutation space size %ld\n", Xn * Kn * XComb * KComb);
  //=================================================
  // 
  //=================================================

  //---------------------------------------------------------------------
  // Enumerate all port orderings
  //---------------------------------------------------------------------

  //---------------------------------------------------------------------
  // Some predefined number of combinations 
  //---------------------------------------------------------------------
  int m, n;
  int *Xconstraint, *Kconstraint;
  int CombSize = XComb > KComb ? XComb : KComb;

    // Xn permutation
    for (x = 0; x < Xn; x++) {
      // for each X perm, we have XComb combinations
      for (m = 0; m < XComb; m++) {

        // for each X comb, we reset Ks array 
        for (i = 0; i < ksize; i++) Ks[i] = i;

        Xconstraint = X_PORTS_CONSTRAINT + m;

        // for each X comb, we have Kn permutations
        for (k = 0; k < Kn; k++) {
          // for each K perm, we have KComb combinations
          for (n = 0; n < KComb; n++) {

            Kconstraint = K_PORTS_CONSTRAINT + n;

            SetFullPortPriority(PortPriority, Xconstraint, Kconstraint,
                                Xs, Ks, Xp, Kp, xsize, ksize, CombSize);

            PortDirectedSchedule(fp, portConfigCnt, MAX_PORT_NU, ASAP_slots, ALAP_slots, Xconstraint, Kconstraint, SSP);
          }

          perm(Ks, ksize);
        }
      }
      perm(Xs, xsize);
    }


  printf("=============================================================\n");
  printf("Exhaustive Port RC Schedule Statics\n");
  printf("=============================================================\n");
  printf("ASAP Delay range <%d %d>\n", SSP->ASAP_min_delay, SSP->ASAP_max_delay);
  printf("ASAP Mux range <%d %d>\n", SSP->ASAP_min_mux, SSP->ASAP_max_mux);
  printf("ASAP Reg range <%d %d>\n", SSP->ASAP_min_reg, SSP->ASAP_max_reg);
  printf("ALAP Delay range <%d %d>\n", SSP->ALAP_min_delay, SSP->ALAP_max_delay);
  printf("ALAP Mux range <%d %d>\n", SSP->ALAP_min_mux, SSP->ALAP_max_mux);
  printf("ALAP Reg range <%d %d>\n", SSP->ALAP_min_reg, SSP->ALAP_max_reg);
  
  //----------------------------------------------------
  // Free DFG and resource and etc.
  //----------------------------------------------------
  free(Ks); free(Xs); 
  free(Kp); free(Xp); 
  free(PortPriority);
  free(NodePriority);

  FreeAll();

  printf("We are done");

  fclose(fp);

  return 0;
}
