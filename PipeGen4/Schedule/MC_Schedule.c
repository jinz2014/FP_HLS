#include <stdlib.h> 
#include <stdio.h> 
#include <assert.h> 
#include <string.h> 
#include <math.h> 
#include "queue.h"
#include "Schedule.h"
#include "PortSchedule.h"
#include "Benchmarks.config"

// conflict
struct conflict_table table[OP_NU]; 

int minII;
int *ASAP_slots, *ALAP_slots;
int *PortPriority;
int *SortedPortPriority;
int *NodePriority;
int *PortAssignment;

opNode **DFG;
queue **RQ, **PQ;
int **RAT, **PRAT;
Fu **MuxRAT;
ResourceList *FuRAT;

FanoutR_ptr RegFanout;
int **FuFanout;
int *PortFanout;

// Schedule RAT  4/5/11
SRAT_FuType_Ptr *SRAT;

varList *VarTable;
MuxSelList *MuxSelTable;
int MuxNuCnt;

#ifdef DATA_PATH_ONLY
  portStringPtr PortNameList = NULL;
  portStringPtr PortDeclList = NULL;
#endif

  char stat[100]; 
  FILE *csv[3][5] ;

  CircuitStats CS;
  CircuitStatsPtr CSP = &CS;

//----------------------------------------------------
// 
//----------------------------------------------------
int main (int argc, char *argv[]) {

  int i, j, k;

  // Full port ordering test
  int Nx = atoi(argv[1]);
  int Nk = atoi(argv[2]);

  int P, px, pk;
  int II;
  int tmp ;
  int *X, *K;
  int sum;
  long permNu; 
  long partition_cnt;
  long ordering_cnt;
  char *parm;

  int N = Nx + Nk;
  int sample_dist;

  // open benchmark statistic text file 
  FILE *fp = OpenStat("mc", filename);


  //-----------------------------------------------
  // Init Schedule Statistics
  //-----------------------------------------------

  ScheduleStats SS[3]; // ASAP, ALAP, Iter

  //----------------------------------------------------
  // Check constraints
  //----------------------------------------------------
  assert(RATE <= LATENCY); 

  CreateDFG();

  //===============================================================
  // For a fixed port priority, we have XComb * KComb combinations
  //===============================================================

  // no port orderings for single port(P = 1) and fully parallel port (P = N)
  for (P = 3; P <= 3; P++) {

    PORT_NU = P;

    myprintf("Port number: %d\n", PORT_NU);
    myfprintf(fp, "Port number: %d\n", PORT_NU);

    //-----------------------------------------------------------
    // Create port and node priority
    //-----------------------------------------------------------
    CreatePortPriority(N, P);

    //-----------------------------------------------------------
    // Set global minimum II
    //-----------------------------------------------------------
    SetMinII(N);
    tmp = minII;

    //----------------------------------------------------
    // by default only one permutation or P = MaxPortNu
    //----------------------------------------------------
    permNu = 1; 
    sample_dist = 1;

    #ifdef PORT_ALL_PRIORITY

    // All permutations of port priority
    if (P < N) permNu = fac(N); 

    if (permNu <= fac(10)) 
      sample_dist = 1;
    else
      sample_dist = 100;

    #endif

    //-------------------------------------------------------------------
    // Create queue data structure
    //-------------------------------------------------------------------
    CreateResource();
    CreateRAT();

    if (P == 1 || P == N) {

      X = NULL; K = NULL;

      ResetStats(SS);

      //---------------------------------------------
      // open benchmark_stats_px_xxx.csv file
      //---------------------------------------------
      #ifdef CSV
      OpenCSV(csv, CircuitName, P, N, px, pk);
      #endif

      for (k = 0; k < permNu / sample_dist; k++) {

        if (PortPriority != NULL) {
          myfprintf(fp, "Port Priority: ");
          for (i = 0; i < N; i++) myfprintf(fp, "%3d ", PortPriority[i]);
          myfprintf(fp, "\n");
        }

        constraint_sort(fp, SortedPortPriority, PortAssignment, MAX_PORT_NU, 
                  X, K, px, pk);

        if (SortedPortPriority != NULL) {
          myfprintf(fp, "Sorted Port Priority: ");
          for (i = 0; i < N; i++) myfprintf(fp, "%3d ", SortedPortPriority[i]);
          myfprintf(fp, "\n");
        }

        // Port Priority changes every loop iteration
        QueueDirectedSchedule(fp, P, MAX_PORT_NU, ASAP_slots, ALAP_slots, \
                             X, K, SS);

        // generate next permutation of port priority
        if (PortPriority != NULL && permNu > 1) {
          for (j = 0; j < sample_dist; j++)
            perm(PortPriority, N);
        }
      }

      #ifdef CSV
      CloseCSV(csv);
      #endif

      PrintScheduleStat(fp, P, N, px, pk, SS);
    }
    //==================================================================================
    // Done (P == 1 || P == N) 
    //==================================================================================
      
    //==================================================================================
    // (1 < P < N)
    //==================================================================================
    else {

      partition_cnt = 0;

      ResetStats(SS);

      for (px = 1; px <= Nx; px++) {

        pk = P - px;  // px + pk = P

        //-------------------------------------------------------------------
        // Here is a valid port partition
        //
        // If only minII port parition is considered, then comment out
        // if (pk >= 1 && pk <= Nk )
        //
        // and use
        // /* port partitions with minimum II */
        // if (pk >= 1 && pk <= Nk && 
        //  (int) ceil((double) Nx / px) <= minII &&
        //  (int) ceil((double) Nk / pk) <= minII)
        // 
        //-------------------------------------------------------------------
        /*
        if (pk >= 1 && pk <= Nk )  

          //-------------------------------------------------------------------
          // Adjust DII
          //-------------------------------------------------------------------
          minII = tmp; // minimum DII by default

          if (myceil(Nx, px) < myceil(Nk, pk)) 
             II = myceil(Nk, pk);
          else
             II = myceil(Nx, px);

          assert(II >= minII);

          if (II > minII) minII = II;
          */
       if (pk >= 1 && pk <= Nk && 
        myceil(Nx, px) <= minII && myceil(Nk, pk) <= minII) {

          myfprintf(fp, "port partition (%dX + %dK) \n", px, pk);

          //-------------------------------------------------------------------
          // Open CSV files
          //-------------------------------------------------------------------
          #ifdef CSV
          OpenCSV(csv, CircuitName, P, N, px, pk);
          #endif

          //-------------------------------------------------------------------
          // Reset CSP
          //-------------------------------------------------------------------
          CSP->reg = 0;
          CSP->dly = 0;
          CSP->mux = 0;
          CSP->fan = 0;

          // setup array for X and K port orderings
          X = (int *) malloc (sizeof(int) * minII);
          K = (int *) malloc (sizeof(int) * minII);

          //-------------------------------------------------------------------
          // for each port partition, use full permutations of port priority
          //-------------------------------------------------------------------
          partition_cnt++; 

          for (k = 0; k < permNu / sample_dist; k++) {

            myfprintf(fp, "Port Priority: ");
            for (i = 0; i < N; i++) myfprintf(fp, "%3d ", PortPriority[i]);
            myfprintf(fp, "\n");

            for (i = 0; i < minII; i++) X[i] = 0;

          //-------------------------------------------------------------------
          // for each port partition and each port priority permutation
          //-------------------------------------------------------------------
            ordering_cnt = 0;

            while (Next(0, px, minII, X)) {

              sum = 0;

              for (i = 0; i < minII; i++) sum += X[i];

              if (sum != Nx) continue;

              for (i = 0; i < minII; i++) K[i] = 0;

              while (Next(0, pk, minII, K)) {
                sum = 0;
                for (i = 0; i < minII ; i++) sum += K[i];
                if (sum == Nk) {
          //-------------------------------------------------------------------
          // Here is a valid port ordering for current port partition
          //-------------------------------------------------------------------

                  if (PortPriority != NULL) {
                    // generate next permutation for the next sample point
                    for (j = 0; j < sample_dist; j++) {
                      if (perm(PortPriority, N) == -1) {
                        // reset port priority for each port partition 
                        for (i = 0; i < N; i++) PortPriority[i] = i;
                      }
                    }
                  }

                  myfprintf(fp, "X: ");
                  for (i = 0; i < minII; i++) myfprintf(fp, "%3d ", X[i]);
                  myfprintf(fp, "\n");
                  myfprintf(fp, "K: ");
                  for (i = 0; i < minII; i++) myfprintf(fp, "%3d ", K[i]);
                  myfprintf(fp, "\n");

                  int first = 1;
                  while (constraint_sort(fp, SortedPortPriority,
                        PortAssignment, MAX_PORT_NU, X, K, px, pk, first)) {
        //-------------------------------------------------------------------
        // Here is a valid port assignment
        //-------------------------------------------------------------------
                    myfprintf(fp, "Sorted Port Priority: ");
                    for (i = 0; i < N; i++) myfprintf(fp, "%3d ", SortedPortPriority[i]);
                    myfprintf(fp, "\n");

                    QueueDirectedSchedule(fp, P, MAX_PORT_NU, ASAP_slots, \
                                         ALAP_slots, X, K, SS);
                    first = 0;
                  }

                  // number of orderings for each port partition 
                  ordering_cnt++;
                }
              }
            } // outer while

            myfprintf(fp, "partition (%dX + %dK) priority No.%d has %d orderings\n", 
                px, pk, k, ordering_cnt);

          } // for (k = 0; k < permNu / sample_dist; k++) 

        //-------------------------------------------------------------------
        // Free X,K after iteration of all the permutations of port priority
        //-------------------------------------------------------------------
          free(X); 
          free(K); 

        //-------------------------------------------------------------------
        // Close CVS files for each port partition
        //-------------------------------------------------------------------
          #ifdef CSV
          CloseCSV(csv);
          #endif

          PrintScheduleStat(fp, P, N, px, pk, SS);

        } // if (pk >= 1 && pk <= Nk)
      } // for (px = 1; px <= Nx; px++)

      myfprintf(fp, "************************************************\n");
      myfprintf(fp, "Port = %d has %d partitions\n", P, partition_cnt);
      myfprintf(fp, "************************************************\n\n");
    }

    FreeQueue();

    if (PortPriority != NULL) free(PortPriority);
    if (SortedPortPriority != NULL) free(SortedPortPriority);
    free(NodePriority);
    free(PortAssignment);

  } // for (P = 1; P <= N; P++)


  FreeSlots();
  FreeDFG();
  fclose(fp);

  printf("We are done");
  return 0;
}

