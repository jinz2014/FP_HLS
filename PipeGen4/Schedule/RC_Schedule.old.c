#include <stdlib.h> 
#include <stdio.h> 
#include <assert.h> 
#include "queue.h"
#include "Schedule.h"
#include "PortSchedule.h"

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


//#include "fir16.config"
//#include "sample1.config"
//#include "ratelaw.config"
//#include "ratelaw1.config"
//#include "pos.config"
//#include "sop.config"
//#include "sample.config"
//#include "random2.config"
//#include "random1.config"

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
int *PortAssignment;

opNode **DFG;
queue **RQ, **PQ;
int **RAT, **PRAT;
Fu **MuxRAT;
ResourceList *FuRAT;

// Schedule RAT  4/5/11
SRAT_FuType_Ptr *SRAT;

varList *VarTable;
MuxSelList *MuxSelTable;
int MuxNuCnt;

#ifdef DATA_PATH_ONLY
  portStringPtr PortNameList = NULL;
  portStringPtr PortDeclList = NULL;
#endif

//----------------------------------------------------
// 
//----------------------------------------------------
int main (int argc, char *argv[]) {

  int i, j, k;

  int portConfigCnt = 3;

  FILE *fp = fopen(filename, "a+");
  //FILE *fp = stdout;


  //=================================================
  // constraints
  //=================================================
  // by default
  //int *Xconstraint = X_PORTS_CONSTRAINT; 
  //int *Kconstraint = K_PORTS_CONSTRAINT;
  int *Xconstraint, *Kconstraint;
  int m, n;

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

  //----------------------------------------------------
  // 
  //----------------------------------------------------
  CreateRAT();

  //----------------------------------------------------
  // 
  //----------------------------------------------------
  CreateDFG();

  //----------------------------------------------------
  // 
  //----------------------------------------------------
  CreatePortPriority(MAX_PORT_NU);

  while (1) {

    PORT_NU = portConfigCnt;

    fprintf(fp, "Port number: %d\n", PORT_NU);

    SetMinII(MAX_PORT_NU);

    //===============================================================
    // For a fixed port priority, we have XComb * KComb combinations
    //===============================================================

    // Single port ordering test
    //m = 1; 
    //n = 2;

    // Full port ordering test
    m = atoi(argv[1]);
    n = atoi(argv[2]);

    //for (m = 0; m < XComb; m++) {

      Xconstraint = X_PORTS_CONSTRAINT + m;

      //for (n = 0; n < KComb; n++) {

        Kconstraint = K_PORTS_CONSTRAINT + n;

        PortDirectedSchedule(fp, portConfigCnt, MAX_PORT_NU, ASAP_slots, 
            ALAP_slots, Xconstraint, Kconstraint, SSP);
      //} 
    //}

    if (MAX_PORT_NU == portConfigCnt) break;

    if (PORT_NU == 3) break; 
    portConfigCnt++;

  } // while

  fprintf(fp, "=============================================================\n");
  fprintf(fp, "Port RC Schedule Statics\n");
  fprintf(fp, "=============================================================\n");
  fprintf(fp, "ASAP Delay <%3d %3d> Mux <%3d %3d> Reg <%3d %3d>\n", 
      SSP->ASAP_min_delay, SSP->ASAP_max_delay,
      SSP->ASAP_min_mux, SSP->ASAP_max_mux,
      SSP->ASAP_min_reg, SSP->ASAP_max_reg);
  fprintf(fp, "ALAP Delay <%3d %3d> Mux <%3d %3d> Reg <%3d %3d>\n", 
      SSP->ALAP_min_delay, SSP->ALAP_max_delay,
      SSP->ALAP_min_mux, SSP->ALAP_max_mux,
      SSP->ALAP_min_reg, SSP->ALAP_max_reg);
  
  //----------------------------------------------------
  // Free DFG and resource and etc.
  //----------------------------------------------------
  FreeAll();

  free(PortPriority);
  free(NodePriority);
  free(PortAssignment);

  printf("We are done");

  fclose(fp);

  return 0;
}
