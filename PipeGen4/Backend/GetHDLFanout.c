#include <stdlib.h> 
#include <stdio.h> 
#include <assert.h> 
#include <string.h> 
#include "queue.h"
#include "Schedule.h"
#include "PortSchedule.h"

void GetHDLFanout(FILE *fp, int SharedRegNu) {

  int i, j;

  myfprintf(fp, "\n------- Dump Register Fanout Tables ---------\n");
  for (i = 0; i < SharedRegNu; i++) {

    myfprintf(fp, "RegFanout[%d]: ", i);

    int *entry = RegFanout[i].entry;

    for (j = 0; j < RegFanout[i].depth; j++) {

      myfprintf(fp, "%d ", entry[j]);

      CSP->tfo += entry[j];

      //-----------------------------
      //if (entry[j] > CSP->rfo) 
      // CSp->rfo = entry[j];
      //-----------------------------
      MyMax(&entry[j], &CSP->rfo);
    }

    myfprintf(fp, "\n");

    free(RegFanout[i].entry);
  }
  free(RegFanout);

  myfprintf(fp, "\n------- Dump FU Fanout Tables ---------------\n");

  for (i = 0; i < OP_NU; i++) {

    myfprintf(fp, "FuFanout[%d](op = %c): ", i, GetOpSym(i));

    for (j = 0; j < (*RQ[i])->size; j++) {
      myfprintf(fp, "%d ", FuFanout[i][j]);
      CSP->tfo += FuFanout[i][j];
      //------------------------------
      //if (FuFanout[i][j] > CSP->ffo) 
        //CSP->ffo = FuFanout[i][j];
      //------------------------------
      MyMax(&FuFanout[i][j], &CSP->ffo);
    }

    myfprintf(fp, "\n");

    free(FuFanout[i]);
  }

  free(FuFanout);

  myfprintf(fp, "\n------- Dump Port Fanout Tables ---------------\n");

  myfprintf(fp, "PortFanout: ");

  for (i = 0; i < PORT_NU; i++) {
    myfprintf(fp, "%d ", PortFanout[i]);
    CSP->tfo += PortFanout[i];

    //------------------------------
    //if (PortFanout[i] > CSP->pfo) 
    //  CSP->pfo = PortFanout[i];
    //------------------------------
    MyMax(&PortFanout[i], &CSP->pfo);
  }

  myfprintf(fp, "\n------- Dump Max Fanout ---------------\n");
  // compare pfo, rfo and ffo
  CSP->fanout = (CSP->pfo > CSP->rfo) ? CSP->pfo : CSP->rfo; 
  if (CSP->fanout < CSP->ffo) CSP->fanout = CSP->ffo;

  myfprintf(fp, "\n\n");
  free(PortFanout);
}

