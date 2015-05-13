#include <stdlib.h> 
#include <stdio.h> 
#include <assert.h> 
#include <string.h> 
#include "queue.h"
#include "Schedule.h"
#include "PortSchedule.h"

//================================================================
// open benchmark statistics text file under the control
// of register allocation and test mode
//================================================================
FILE *OpenStat(const char *priority, const char *filename) {

  char file[100];
  char filetmp[100];

  sprintf(filetmp, "%s", filename);
  char *dot = strchr(filetmp, '.');
  *dot = '\0';

  #ifdef PORT_PRIORITY
    #ifdef WBM
    sprintf(file, "Stat/%s_wbm_%s_simple.txt", priority, filetmp);
    #elif defined(LEA)
    sprintf(file, "Stat/%s_lea_%s_simple.txt", priority, filetmp);
    #else
    sprintf(file, "Stat/%s_max_%s_simple.txt", priority, filetmp);
    #endif
  #endif

  #ifdef PORT_ALL_PRIORITY
    #ifdef WBM
    sprintf(file, "Stat/%s_wbm_%s_all.txt", priority, filetmp);
    #elif defined(LEA)
    sprintf(file, "Stat/%s_lea_%s_all.txt", priority, filetmp);
    #else
    sprintf(file, "Stat/%s_max_%s_all.txt", priority, filetmp);
    #endif
  #endif

  return fopen(file, "w+");
}

//================================================================
// Open csv files
//
// P: Current logical port number
// N: Total logical port number
// px: Physical X port number
// pk: Physical K port number
//================================================================
void OpenCSV(FILE *csv[][5], const char *CircuitName, int P, int N, int px, int pk) {
  int i, j, fs;
  char *test, *priority, *parm, *schedule;
  char stat[100];

  schedule = priority = test = parm = NULL;

  //#ifdef PORT_PRIORITY 
  test = strdup("simple"); // by default
  //#endif

  #ifdef PORT_ALL_PRIORITY
  test = strdup("all");
  #endif

  #ifdef MC_SCHEDULE
    #ifdef WBM
      priority = strdup("mc_wbm");
    #elif defined(LEA)
      priority = strdup("mc_lea");
    #else
      priority = strdup("mc_max");
    #endif
  fs = 3;
  #endif

  #ifdef RC_SCHEDULE
    #ifdef WBM
      priority = strdup("rc_wbm");
    #elif defined(LEA)
      priority = strdup("rc_lea");
    #else
      priority = strdup("rc_max");
    #endif
  fs = 2;
  #endif

  for (j = 0; j < fs; j++) {
    switch(j) {
      case 0: schedule = strdup("asap"); break;
      case 1: schedule = strdup("alap"); break;
      case 2: schedule = strdup("iter"); break;
    }

    for (i = 0; i < 4; i++) {
      switch (i) {
        case 0: parm = strdup("reg"); break;
        case 1: parm = strdup("mux"); break;
        case 2: parm = strdup("dly"); break;
        case 3: parm = strdup("fan"); break;
      }

      if (P == 1 || P == N) 
        sprintf(stat, "CSV/%s/%s/%s_%s/Port%d/%s_%s.csv", 
            priority, schedule, CircuitName, test, P, CircuitName, parm);
      else
        sprintf(stat, "CSV/%s/%s/%s_%s/Port%d/%s_%dX%dK_%s.csv", 
            priority, schedule, CircuitName, test, P, CircuitName, px, pk, parm);

      csv[j][i] = fopen(stat, "w+");             
      if (csv[j][i] == NULL) perror("Error in opening file");

      fprintf(csv[j][i], "port=%d", P);

      if (parm != NULL) free(parm);
    } // for i
    if (schedule != NULL) free(schedule);
  } // for j
  if (test != NULL) free(test);
  if (priority != NULL) free(priority);
}

//================================================================
// Close csv files
//================================================================
void CloseCSV(FILE *csv[][5]) {
  int i, j, N;
  #ifdef MC_SCHEDULE
  N = 3;
  #endif

  #ifdef RC_SCHEDULE
  N = 2;
  #endif

  for (j = 0; j < N; j++) {
    for (i = 0; i < 4; i++) {
      fprintf(csv[j][i], "\n");
      fclose(csv[j][i]);
    }
  }
}

//================================================================
// Print schedule stats ranges 
//
// P: Current logical port number
// N: Total logical port number
// px: Physical X port number
// pk: Physical K port number
// SS: schedule statics
//================================================================
void PrintScheduleStat(FILE *fp, int P, int N, int px, int pk, ScheduleStats SS[]) {
  int i;
  int fs;
  fprintf(fp, "*************************************************************\n");
  fprintf(fp, "      Schedule Statistics\n");

  if (P == 1 || P == N)
    fprintf(fp, "Port=%d No Partition\n", P);
  else
    fprintf(fp, "Port=%d Partition=%dX %dK \n", P, px, pk);

  #ifdef MC_SCHEDULE
    fs = 3;
  #endif

  #ifdef RC_SCHEDULE 
    fs = 2;
  #endif
    
  for (i = 0; i < fs; i++) {
    fprintf(fp, "%s Delay <%3d %3d> Mux <%3d %3d> Fan <%3d %3d> Reg <%3d %3d>\n",
      (i == 0) ? "ASAP" : (i == 1) ? "ALAP" : (i == 2) ? "Iter" : "XXXX",
      SS[i].min_dly, SS[i].max_dly,
      SS[i].min_mux, SS[i].max_mux,
      SS[i].min_fan, SS[i].max_fan,
      SS[i].min_reg, SS[i].max_reg);

    fprintf(fp, "%s RFO <%3d %3d> FFO <%3d %3d> PFO <%3d %3d> TFO <%3d %3d> Fanout<%3d %3d>\n",
      (i == 0) ? "ASAP" : (i == 1) ? "ALAP" : (i == 2) ? "Iter" : "XXXX",
      SS[i].min_rfo, SS[i].max_rfo,
      SS[i].min_ffo, SS[i].max_ffo,
      SS[i].min_pfo, SS[i].max_pfo,
      SS[i].min_tfo, SS[i].max_tfo,
      SS[i].min_fanout, SS[i].max_fanout);
  }
  fprintf(fp, "*************************************************************\n\n");
}

//================================================================
// Print current schedule stats
//================================================================
void PrintStats(FILE *fp, char *name, int latency, int resource, int delay) {
  enum operation op_type;
  int sum;
  myfprintf(fp, "%s: ", name);
  myfprintf(fp, "minII = %d ",  latency);
  myfprintf(fp, "resource number = %d: ", resource);
  for (op_type = add; op_type < add + OP_NU; op_type++) {
    if (RQ[op_type] != NULL)   // ASAP, ALAP, List
      sum = (*RQ[op_type])->size; 
    else 
      sum = PipeFuNu(op_type); // FDS, Iterative
    myfprintf(fp, "%d ", sum);
  }
  myfprintf(fp, "delay = %d\n", delay);
}


void ResetStats(ScheduleStats SS[]) {
  int i;

  for (i = 0; i < 3; i++) { 
    SS[i].min_dly = 10000; // 10000 an arbitrary big number
    SS[i].max_dly = 0;
    SS[i].min_mux = 10000;
    SS[i].max_mux = 0;
    SS[i].min_fan = 10000;
    SS[i].max_fan = 0;
    SS[i].min_reg = 10000;
    SS[i].max_reg = 0;
    SS[i].min_rfo = 10000;
    SS[i].max_rfo = 0;
    SS[i].min_ffo = 10000;
    SS[i].max_ffo = 0;
    SS[i].min_pfo = 10000;
    SS[i].max_pfo = 0;
    SS[i].min_tfo = 10000;
    SS[i].max_tfo = 0;
    SS[i].min_fanout = 10000;
    SS[i].max_fanout = 0;
  }
}

void CollectStats(ScheduleStats SS[], char *ScheduleName) {

  int i, j;
  i = 3;
  if (strstr(ScheduleName, "ASAP") != NULL) i = 0;
  if (strstr(ScheduleName, "ALAP") != NULL) i = 1;
  if (strstr(ScheduleName, "Iter") != NULL) i = 2;
    
  //-------------------------------------------------------
  // WBM works file
  //-------------------------------------------------------
  if (!WBM_FAILED && i <= 2) {
    // '->' has higher priority than '&' 
    assert(i <= 2);
    MyMin(&CSP->mux, &SS[i].min_mux);
    MyMax(&CSP->mux, &SS[i].max_mux);
    MyMin(&CSP->reg, &SS[i].min_reg);
    MyMax(&CSP->reg, &SS[i].max_reg);
    MyMin(&CSP->dly, &SS[i].min_dly);
    MyMax(&CSP->dly, &SS[i].max_dly);
    MyMin(&CSP->fan, &SS[i].min_fan);
    MyMax(&CSP->fan, &SS[i].max_fan);
    MyMin(&CSP->rfo, &SS[i].min_rfo);
    MyMax(&CSP->rfo, &SS[i].max_rfo);
    MyMin(&CSP->ffo, &SS[i].min_ffo);
    MyMax(&CSP->ffo, &SS[i].max_ffo);
    MyMin(&CSP->pfo, &SS[i].min_pfo);
    MyMax(&CSP->pfo, &SS[i].max_pfo);
    MyMin(&CSP->tfo, &SS[i].min_tfo);
    MyMax(&CSP->tfo, &SS[i].max_tfo);
    MyMin(&CSP->fanout, &SS[i].min_fanout);
    MyMax(&CSP->fanout, &SS[i].max_fanout);
  }
}
