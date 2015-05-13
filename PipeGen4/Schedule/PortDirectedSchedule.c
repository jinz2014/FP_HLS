#include <stdio.h> 
#include <stdlib.h> 
#include "Schedule.h"

void PortDirectedSchedule(FILE *fp, int portConfigCnt, int MAX_PORT_NU, 
                          int *ASAP_slots, int *ALAP_slots, 
                          int *Xconstraint, int *Kconstraint, 
                          ScheduleStats SS[]) {
  char scheduleName[10];
  char make_cmd[50];

  // Scheduled FUs
  int ASAP_resource, ALAP_resource, List_resource, Iter_resource;

  // Data path DII
  int ASAP_DII, ALAP_DII;

  // Data path delay 
  int ASAP_delay, ALAP_delay, List_delay, delay;

  //---------------------------------------------
  // ASAP
  //---------------------------------------------

  myprintf("********* ASAP Start ********* \n");

  ASAP_delay = ASAP(ASAP_slots, Xconstraint, Kconstraint);
  ASAP_DII = CheckConflict();
  ASAP_resource = CountResource();

  //PrintDFG(ASAP_slots, ALAP_slots);

  CSP->dly = ASAP_delay;

  PrintStats(fp, "ASAP", ASAP_DII, ASAP_resource, ASAP_delay);

  myprintf("********* ASAP Scheduling Done ********* \n");

  //goto ALAP_SCH;

  GenerateBackEnd(fp, scheduleName, "ASAP", portConfigCnt, ASAP_delay, SS);

  myprintf("********* ASAP Generation Done ********* \n");

  //exit(1);

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

  printf("---------------------------\n");
  printf("ASAP regs fanout (%d)\n", CSP->rfo);
  printf("ASAP func fanout (%d)\n", CSP->ffo);
  printf("ASAP port fanout (%d)\n", CSP->pfo);
  printf("ASAP mux chain reg (%d)\n", CSP->mux_reg);
  printf("ASAP mux fanin (%d)\n", CSP->fan);
  printf("ASAP mux input (%d)\n", CSP->mux);
  printf("ASAP muxp input (%d)\n", CSP->muxp);
  printf("ASAP muxr input (%d)\n", CSP->muxr);
  printf("ASAP reg number(%d)\n", CSP->reg);
  printf("ASAP latency   (%d)\n", CSP->dly);


ALAP_SCH:
  ResetResource();
  ResetRAT();

#ifdef CSV
  fprintf(csv[0][REG], ",%d", CSP->reg);
  fprintf(csv[0][DLY], ",%d", CSP->dly);
  fprintf(csv[0][MUX], ",%d", CSP->mux);
  fprintf(csv[0][FAN], ",%d", CSP->fan);
#endif

  return;  // just ASAP results

  //---------------------------------------------
  // ALAP
  //---------------------------------------------
  myprintf("********* ALAP Start ********* \n");

  ALAP_delay = ALAP(ALAP_slots, MAX_PORT_NU);
  ALAP_DII = CheckConflict();
  ALAP_resource = CountResource();

  CSP->dly = ALAP_delay;

  //PrintDFG(ASAP_slots, ALAP_slots);

  PrintStats(fp, "ALAP", ALAP_DII, ALAP_resource, ALAP_delay);

  myprintf("********* ALAP Scheduling Done ********* \n");

  goto ENUM_SCH;
  //return;

  GenerateBackEnd(fp, scheduleName, "ALAP", portConfigCnt, ALAP_delay, SS);

  myprintf("********* ALAP Generation Done ********* \n");

#ifdef MODELSIM
  printf("**************************************************\n");
  printf("*   Testing circuit ALAP%d_%s\n", portConfigCnt, CircuitName);
  printf("**************************************************\n");

  make_cmd[0] = '\0';
  sprintf(make_cmd, "make ALAP%d_%s", portConfigCnt, CircuitName);
  system(make_cmd);

  printf("**************************************************\n");
  printf("*  Done Testing circuit ALAP%d_%s\n", portConfigCnt, CircuitName);
  printf("**************************************************\n");
#endif

  fprintf(stderr, "---------------------------\n");

  fprintf(stderr, "ALAP regs fanout (%d)\n", CSP->rfo);
  fprintf(stderr, "ALAP func fanout (%d)\n", CSP->ffo);
  fprintf(stderr, "ALAP port fanout (%d)\n", CSP->pfo);
  fprintf(stderr, "ALAP mux fanin (%d)\n", CSP->fan);
  fprintf(stderr, "ALAP mux input (%d)\n", CSP->mux);
  fprintf(stderr, "ALAP muxp input (%d)\n", CSP->muxp);
  fprintf(stderr, "ALAP muxr input (%d)\n", CSP->muxr);
  fprintf(stderr, "ALAP reg number(%d)\n", CSP->reg);
  fprintf(stderr, "ALAP latency   (%d)\n", CSP->dly);

ENUM_SCH:
  ResetResource();
  ResetRAT();

#ifdef CSV
  fprintf(csv[1][REG], ",%d", CSP->reg);
  fprintf(csv[1][DLY], ",%d", CSP->dly);
  fprintf(csv[1][MUX], ",%d", CSP->mux);
  fprintf(csv[1][FAN], ",%d", CSP->fan);
#endif

  myprintf("********* Enumeration Scheduling Start ********* \n");

  CreateScheduleRAT();

  int i;
  int first, last;


  // The product of all the mobilities. The size of some benchmarks are too big 
  for (i = MAX_PORT_NU; i < NODE_NU; i++) {
    if (ASAP_slots[i] > ALAP_slots[i]) {
      // update ASAP/ALAP slots if ALAP has shorter latency 
      first = ALAP_slots[i];
      last = ASAP_slots[i];

      ASAP_slots[i] = first;
      ALAP_slots[i] = last;
    }

    if (ALAP_slots[i] - ASAP_slots[i] > 0)
      ENUM_SZ *= ALAP_slots[i] - ASAP_slots[i] + 1;
  }

  fprintf(stderr, "Est. Schedule enum space size %lf\n", ENUM_SZ);

  // A naive branch and bound complete enumeration method 
  Enumerate (ASAP_slots, ALAP_slots, MAX_PORT_NU);

  fprintf(stderr, "Act. Schedule enum number = %lf\n", ENUM_NU);

  fprintf(stderr, "---------------------------\n");
  fprintf(stderr, "Enum mux fanout (%d %d)\n", min_mux_fanout, max_mux_fanout);
  fprintf(stderr, "Enum mux fanin (%d %d)\n", min_mux_fanin, max_mux_fanin);
  fprintf(stderr, "Enum mux input (%d %d)\n", min_mux_input, max_mux_input);
  fprintf(stderr, "Enum muxp input (%d %d)\n", min_muxp_input, max_muxp_input);
  fprintf(stderr, "Enum muxr input (%d %d)\n", min_muxr_input, max_muxr_input);
  fprintf(stderr, "Enum reg number(%d %d)\n", min_reg_nu, max_reg_nu);
  fprintf(stderr, "Enum latency   (%d %d)\n", min_latency, max_latency);

  FreeScheduleRAT();
  //PrintDFG(ASAP_slots, ALAP_slots);

  //
}

