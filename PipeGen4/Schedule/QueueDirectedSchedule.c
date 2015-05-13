#include <stdio.h> 
#include <stdlib.h> 
#include "Schedule.h"

void QueueDirectedSchedule(FILE *fp, int portConfigCnt, int MAX_PORT_NU, 
                          int *ASAP_slots, int *ALAP_slots,
                          int *Xconstraint, int *Kconstraint,
                          ScheduleStats SS[]) {
  char scheduleName[10];
  char make_cmd[50];

  int ASAP_latency, ALAP_latency, List_latency, latency;

  // Data path delay 
  int ASAP_delay, ALAP_delay, List_delay, delay;

  // Scheduled FUs
  int ASAP_resource, ALAP_resource, List_resource, Iter_resource;

  myprintf("********* ASAP Start ********* \n");
  do {
    ASAP_delay = ASAP(ASAP_slots, Xconstraint, Kconstraint);

    ASAP_latency = CheckConflict();

  } while(Reschedule(ASAP_latency));

  ASAP_resource = CountResource();

  CSP->dly = ASAP_delay;

  PrintStats(fp, "ASAP", ASAP_latency, ASAP_resource, ASAP_delay);
  myprintf("********* ASAP Done ********* \n");

  GenerateBackEnd(fp, scheduleName, "ASAP", portConfigCnt, ASAP_delay, SS);

  /*
#ifdef MODELSIM
  printf("**************************************************\n");
  printf("*   Testing circuit ASAP%d_%s\n", portConfigCnt, CircuitName);
  printf("**************************************************\n");
  fflush(stdout);

  make_cmd[0] = '\0';
  sprintf(make_cmd, "make ASAP%d_%s", portConfigCnt, CircuitName);
  if (system(make_cmd) != 0) {
    printf("********* ASAP system(make_cmd) Error ********* \n");
    exit(-1);
  }

  printf("**************************************************\n");
  printf("*   Done Testing circuit ASAP%d_%s\n", portConfigCnt, CircuitName);
  printf("**************************************************\n");
#endif
*/

#ifdef CSV
  fprintf(csv[0][REG], ",%d", CSP->reg);
  fprintf(csv[0][DLY], ",%d", CSP->dly);
  fprintf(csv[0][MUX], ",%d", CSP->mux);
  fprintf(csv[0][FAN], ",%d", CSP->fan);
#endif

  ResetResource();
  ResetRAT();

  //---------------------------------------------
  // ALAP
  //---------------------------------------------
  myprintf("********* ALAP Start ********* \n");
  do {

    ALAP_delay = ALAP(ALAP_slots, MAX_PORT_NU);

    ALAP_latency = CheckConflict();

  } while(Reschedule(ALAP_latency));

  ALAP_resource = CountResource();

  CSP->dly = ALAP_delay;

  //PrintDFG(ASAP_slots, ALAP_slots);

  PrintStats(fp, "ALAP", ALAP_latency, ALAP_resource, ALAP_delay);
  myprintf("********* ALAP Done ********* \n");

  GenerateBackEnd(fp, scheduleName, "ALAP", portConfigCnt, ALAP_delay, SS);

  /*
#ifdef MODELSIM
  printf("**************************************************\n");
  printf("*   Testing circuit ALAP%d_%s\n", portConfigCnt, CircuitName);
  printf("**************************************************\n");
  fflush(stdout);

  make_cmd[0] = '\0';
  sprintf(make_cmd, "make ALAP%d_%s", portConfigCnt, CircuitName);
  if (system(make_cmd) != 0) {
    printf("********* ALAP system(make_cmd) Error ********* \n");
    exit(-1);
  }

  printf("**************************************************\n");
  printf("*   Done Testing circuit ALAP%d_%s\n", portConfigCnt, CircuitName);
  printf("**************************************************\n");
#endif
*/

#ifdef CSV
  fprintf(csv[1][REG], ",%d", CSP->reg);
  fprintf(csv[1][DLY], ",%d", CSP->dly);
  fprintf(csv[1][MUX], ",%d", CSP->mux);
  fprintf(csv[1][FAN], ",%d", CSP->fan);
#endif

  ResetResource();
  ResetRAT();

  myprintf("********* Iterative Start ********* \n");

  Iterative_Scheduling(ASAP_slots, ALAP_slots);

  PipeFuAlloc();

  //PrintDFG(ASAP_slots, ALAP_slots);
  myprintf("********* Iterative Done ********* \n");

  Iter_resource = PipeFuCost(ASAP_slots, ALAP_slots);

  CSP->dly = ASAP_delay;

  // Same delay as ASAP
  PrintStats(fp, "Iter", minII, Iter_resource, ASAP_delay);

  GenerateBackEnd(fp, scheduleName, "Iter", portConfigCnt, ASAP_delay, SS);

#ifdef MODELSIM
  printf("**************************************************\n");
  printf("*   Testing circuit Iter%d_%s\n", portConfigCnt, CircuitName);
  printf("**************************************************\n");
  fflush(stdout);

  make_cmd[0] = '\0';
  sprintf(make_cmd, "make Iter%d_%s", portConfigCnt, CircuitName);
  if (system(make_cmd) != 0) {
    printf("********* Iterative system(make_cmd) Error ********* \n");
    exit(-1);
  }

  printf("**************************************************\n");
  printf("*   Done Testing circuit Iter%d_%s\n", portConfigCnt, CircuitName);
  printf("**************************************************\n");
#endif

#ifdef CSV
  fprintf(csv[2][REG], ",%d", CSP->reg);
  fprintf(csv[2][DLY], ",%d", CSP->dly);
  fprintf(csv[2][MUX], ",%d", CSP->mux);
  fprintf(csv[2][FAN], ",%d", CSP->fan);
#endif

  ResetResource();
  ResetRAT();
}
