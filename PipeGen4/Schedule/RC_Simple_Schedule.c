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

// branch bound
double ENUM_NU;
double ENUM_SZ = 1;
int min_mux_fanin = 100000;
int max_mux_fanin = 0;
int min_mux_fanout = 100000;
int max_mux_fanout = 0;
int min_mux_input = 100000;
int max_mux_input = 0;
int min_muxp_input = 100000;
int max_muxp_input = 0;
int min_muxr_input = 100000;
int max_muxr_input = 0;
int min_reg_nu = 100000;
int max_reg_nu = 0;
int min_latency = 100000;
int max_latency = 0;

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

int WBM_FAILED;

#ifdef DATA_PATH_ONLY
  portStringPtr PortNameList = NULL;
  portStringPtr PortDeclList = NULL;
#endif

  char stat[100]; 
  FILE *csv[3][5] ;
  //FILE *csv[5] ;

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
  int *X, *K;
  int sum;
  long permNu; 
  long partition_cnt;
  long ordering_cnt;
  char *parm;

  int N = Nx + Nk;
  int sample_dist;

  //int P_lo = atoi(argv[3]);
  //int P_hi = atoi(argv[4]);
  //assert(P_hi <= N && P_lo >= 0 && P_lo <= P_hi);

  // open benchmark statistic text file 
  FILE *fp = OpenStat("rc", filename);

  //-----------------------------------------------
  // Init Schedule Statistics
  //-----------------------------------------------

  ScheduleStats SS[3]; // ASAP, ALAP, Iter

  //-----------------------------------------------
  // Check constraints
  //-----------------------------------------------
  assert(RATE <= LATENCY); 

  CreateDFG();
  
  //exit(0);  // generate a right DFG after it changes ?

  //-----------------------------------------------
  // Simple Schedule
  //
  // No port orderings for all the port numbers
  // Assign the logic port to physical port in the
  // order of port priority
  //-----------------------------------------------
  X = K = NULL;
  px = pk = 0;

  //for (P = MAX_PORT_NU; P <= MAX_PORT_NU; P++) {
  //for (P = 4; P <= 4; P++) {
  for (P = 1; P <= 1; P++) {

    PORT_NU = P;

    myprintf("Port number: %d\n", PORT_NU);

    //fprintf(fp, "Port number: %d\n", PORT_NU);
    myfprintf(fp, "Port number: %d\n", PORT_NU);

    //-----------------------------------------------------------
    // Create port and node priority
    //-----------------------------------------------------------
    CreatePortPriority(N, P);

    //-----------------------------------------------------------
    // Set global minimum II
    //-----------------------------------------------------------
    SetMinII(N);

    //----------------------------------------------------
    // By default only one permutation of port priority or
    // P = MaxPortNu
    //----------------------------------------------------
    permNu = 1; 
    sample_dist = 1;

    #ifdef PORT_ALL_PRIORITY

    //----------------------------------------------------
    // All permutations of port priority
    //----------------------------------------------------
    if (P < N) permNu = fac(N); 

    if (permNu <= fac(9)) 
      sample_dist = 1;
    else if (permNu <= fac(11)) 
      sample_dist = 10;
    else
      sample_dist = 10000;

    #endif

    //-------------------------------------------------------------------
    // Create queue data structure
    //-------------------------------------------------------------------
    CreateResource();
    CreateRAT();

    ResetStats(SS);


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
      PortDirectedSchedule(fp, P, MAX_PORT_NU, ASAP_slots, ALAP_slots, \
                           X, K, SS);

      //----------------------------------------------------
      // generate next permutation of port priority
      //----------------------------------------------------
      if (PortPriority != NULL && permNu > 1) {
        for (j = 0; j < sample_dist; j++)
          perm(PortPriority, N);
      }
      break;
    }

    PrintScheduleStat(fp, P, N, px, pk, SS);
      
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

