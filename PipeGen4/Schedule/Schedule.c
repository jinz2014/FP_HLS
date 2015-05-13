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

opNode **DFG;
queue **RQ, **PQ;
int **RAT, **PRAT;
Fu **MuxRAT;
ResourceList *FuRAT;

SRAT_Fu *SRAT;

//ResourceList *StimTable;
int *StimSequenceTable;
varList *VarTable;
MuxSelList *MuxSelTable;
int MuxNuCnt;
//char vfilename[100];


//----------------------------------------------------
// 
//----------------------------------------------------
int main () {

  int i, j, k;

  enum operation op_type;

  // Total ASAP slot number
  int ASAP_slot_nu;

  // Data path latency 
  int ASAP_latency, ALAP_latency, List_latency, latency;

  // Data path delay 
  int ASAP_delay, ALAP_delay, List_delay, delay;

  // Scheduled FUs
  int ASAP_resource, ALAP_resource, List_resource, Iter_resource;

  // Number of input port configurations
  int MAX_PORT_NU = 0; 

  int timeSlotNu;
  int SharedRegNu;
  int MuxInputNu;
  int portConfigCnt = 3;

  // Register number mapped to the variable array.
  int *Map;

  char ScheduleName[10];
  FILE *fp = fopen(filename, "w");

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

  CreatePortPriority(MAX_PORT_NU);

  while (1) {
  //----------------------------------------------------
  // Incrementally add FUs to meet minII constraint
  // MinIISchedule()
  //----------------------------------------------------


  PORT_NU = portConfigCnt;

  fprintf(fp, "Port number: %d\n", PORT_NU);

  SetMinII(MAX_PORT_NU);

  //---------------------------------------------
  // ASAP
  //---------------------------------------------

  printf("********* ASAP Start ********* \n");
  do {

    //CreateStimSequenceTable(MAX_PORT_NU);

    ASAP_delay = ASAP(ASAP_slots);

    ASAP_latency = CheckConflict();

  } while(Reschedule(ASAP_latency));

  ASAP_resource = CountResource();

  PrintStats(fp, "ASAP", ASAP_latency, ASAP_resource, ASAP_delay);

  printf("********* ASAP Done ********* \n");

  ResetResource();
  ResetRAT();
  //GenerateBackEnd(fp, ScheduleName, "ASAP", portConfigCnt, ASAP_delay);

  /*
  sprintf(ScheduleName, "ASAP%d_", portConfigCnt);

  Map = (int *) calloc (NODE_NU, sizeof(int));
  SharedRegNu = LeftEdgeRegisterBinding(Map);

  InterconnectionAlloc(Map, SharedRegNu);
  MuxInputNu = GenerateHDL(Map, SharedRegNu, ScheduleName, ASAP_delay); 
  FreeTables(Map, SharedRegNu);

  fprintf(fp, "Number of Registers : %d\n", SharedRegNu);
  fprintf(fp, "Number of Mux inputs: %d\n", MuxInputNu);
  fprintf(fp, "\n\n");

  ResetResource();
  ResetRAT();
  //ResetPRAT();
  */

  //---------------------------------------------
  // ALAP
  //---------------------------------------------
  printf("********* ALAP Start ********* \n");
  do {
    //CreateStimSequenceTable(MAX_PORT_NU);

    //ASAP_slot_nu = ASAP(ASAP_slots);
    //ALAP(ALAP_slots, ASAP_slot_nu);
    //ALAP_delay = ALAP1(ALAP_slots, MAX_PORT_NU, 1);
    ALAP_delay = ALAP(ALAP_slots, MAX_PORT_NU);

    ALAP_latency = CheckConflict();

  } while(Reschedule(ALAP_latency));

  ALAP_resource = CountResource();

  PrintStats(fp, "ALAP", ALAP_latency, ALAP_resource, ALAP_delay);
  printf("********* ALAP Done ********* \n");

  ResetResource();
  ResetRAT();
  /*

  GenerateBackEnd(fp, ScheduleName, "ALAP", portConfigCnt, ALAP_delay);

  sprintf(ScheduleName, "ALAP%d_", portConfigCnt);
  Map = (int *) calloc (NODE_NU, sizeof(int));
  SharedRegNu = LeftEdgeRegisterBinding(Map);

  InterconnectionAlloc(Map, SharedRegNu);
  MuxInputNu = GenerateHDL(Map, SharedRegNu, ScheduleName, ALAP_delay); 
  FreeTables(Map, SharedRegNu);

  fprintf(fp, "Number of Registers : %d\n", SharedRegNu);
  fprintf(fp, "Number of Mux inputs: %d\n", MuxInputNu);
  fprintf(fp, "\n\n");


  //---------------------------------------------
  // List
  //---------------------------------------------
 
  printf("********* List Start ********* \n");
  do {
    CreateStimSequenceTable(MAX_PORT_NU);
    List_delay = ListSchedule(ASAP_slots, ALAP_slots, MAX_PORT_NU, 1); 
    List_latency = CheckConflict();
  } while (Reschedule(List_latency));

  List_resource = CountResource();

  PrintStats(fp, "List", List_latency, List_resource, List_delay);
  printf("********* List Done ********* \n");
  sprintf(ScheduleName, "List%d_", portConfigCnt);
  Map = (int *) calloc (NODE_NU, sizeof(int));
  SharedRegNu = LeftEdgeRegisterBinding(Map);
  InterconnectionAlloc(Map, SharedRegNu);
  MuxInputNu = GenerateHDL(Map, SharedRegNu, ScheduleName, List_delay); 
  FreeTables(Map, SharedRegNu);

  fprintf(fp, "Number of Registers : %d\n", SharedRegNu);
  fprintf(fp, "Number of Mux inputs: %d\n", MuxInputNu);
  fprintf(fp, "\n\n");

  ResetResource();
  ResetRAT();
  */


  /*
  //---------------------------------------------
  // FDS
  //---------------------------------------------

  ResetResource();
  ResetRAT();

  printf("********* FDS Start ********* \n");
  FDS(ASAP_slots, ALAP_slots);

  ResetResource();
  ResetRAT();

  printf("********* FDS Done ********* \n");
  */

  printf("********* Iterative Start ********* \n");
  //CreateStimSequenceTable(MAX_PORT_NU);
  for (i = 0; i < NODE_NU; i++) {
    if (DFG[i]->opSrc[0] < 0)
      //StimSequenceTable[i] = DFG[i]->opPriority;
      //StimSequenceTable[i] = PortPriority[i];
  }
  Iterative_Scheduling(ASAP_slots, ALAP_slots);

  PipeFuAlloc();

  //PrintDFG(ASAP_slots, ALAP_slots);
  printf("********* Iterative Done ********* \n");

  Iter_resource = PipeFuCost(ASAP_slots, ALAP_slots);

  PrintStats(fp, "Iter", minII, Iter_resource, ASAP_delay);

  // Same delay as ASAP
  GenerateBackEnd(fp, ScheduleName, "Iter", portConfigCnt, ASAP_delay);
  
  //----------------------------------------------------
  // 
  //----------------------------------------------------
  //if (!GeneratePort(MAX_PORT_NU)) break;
  if (MAX_PORT_NU == portConfigCnt) break;

  ResetResource();
  ResetRAT();

  // For test purpose
  if (PORT_NU == 3) break; 
  portConfigCnt++;
 } 

  fclose(fp);
  
  //----------------------------------------------------
  // Free DFG and resource and etc.
  //----------------------------------------------------
  FreeAll();

  free(PortPriority);
  free(NodePriority);

  printf("We are done");

  return 0;
}
