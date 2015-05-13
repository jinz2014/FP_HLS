#include <stdlib.h> 
#include <stdio.h> 
#include <assert.h> 
#include <string.h> 
#include "Test_DFG.h"

// Globals
char *CircuitName;
int  maxPortNu;
int  NODE_NU;
opNode **DFG;

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
const int SINK = -1;


// Generate test stimulus and gold test results
int GenerateTestData(int testNu) {

  int i, j; 
  FILE *fp1, *fp2;
  int32_or_float *ports;

  int32_or_float res;
  char dataFileName [50];
  char resultFileName [50];

  srand(11); // stdlib.h

  // Test stimulus file
  sprintf(dataFileName, "TestData/%s_test.data", CircuitName);
  sprintf(resultFileName, "TestData/%s_sw_result.txt", CircuitName);
  fp1 = fopen(dataFileName, "w+");
  if (fp1 == NULL) perror("Open TestData file error");
  fp2 = fopen(resultFileName, "w+");
  if (fp2 == NULL) perror("Open result file error");

  ports = (int32_or_float *) malloc (sizeof(int32_or_float) * maxPortNu);

  for (i = 0; i < testNu; i++) {
    for (j = 0; j < maxPortNu; j++) {  // 1/4/2011 based on port
      ports[j].f = rand() / (float)(RAND_MAX);
      fprintf(fp1, "%08X\n", ports[j].i);
      printf("port %d = %.11f\n", j + 1, ports[j].f);
    }

    //--------------------------------------------------------------------
    // Call gold benchmark functions in Benchmark.c
    //--------------------------------------------------------------------

    if (!strcmp(CircuitName, "ORDBBR"))
      ordbbr(fp2, ports, maxPortNu);

    else if (!strcmp(CircuitName, "ORDBUR"))
      ordbur(fp2, ports, maxPortNu);

    //else if (!strcmp(CircuitName, "ordubr"))
     // ordubr(fp2, ports, maxPortNu);

    else if (!strcmp(CircuitName, "PPBR"))
      ppbr(fp2, ports, maxPortNu);

    else if (!strcmp(CircuitName, "UUCI"))
      uuci(fp2, ports, maxPortNu);

    else if (!strcmp(CircuitName, "SAMPLE8"))
      sample8(fp2, ports, maxPortNu);

    else if (!strcmp(CircuitName, "SAMPLE9"))
      sample9(fp2, ports, maxPortNu);

    else if (!strcmp(CircuitName, "PLF"))
      plf(fp2, ports, maxPortNu);

    else
      fprintf(stderr, "Undefined circuit name\n");
  }

  free(ports);
  fclose(fp1);
  fclose(fp2);

  return testNu;
}

// Use test stimulus to test DFG generated by the tool
void GenerateSWDebugData(FILE *cfp, int testCount) {

  // Open CircuitName_test.data and CircuitName_debug.txt
  char srcFileName[50];

  sprintf(srcFileName, "TestData/%s_test.data", CircuitName);
  //const float MINUS1 = -1.0; 

  FILE *fs = fopen(srcFileName, "r");

  int32_or_float opA, opB, res;

  int m, n;
  int i, j, k;
  int destNu;

  // Set up debug structures
  for (i = 0; i < NODE_NU; i++) {
    if (DFG[i]->opSrc[0] < 0)
      continue;
    else {
      DFG[i]->opVal = (float *) malloc (sizeof(float) * 3);
    }
  }

  // load test data
  int32_or_float *ports = 
  (int32_or_float *) malloc (sizeof(int32_or_float) * maxPortNu);

  for (m = 0; m < testCount; m++) {

    for (n = 0; n < maxPortNu; n++)
      fscanf(fs, "%x", &(ports[n].i));

    for (i = 0; i < NODE_NU; i++) {

      // proprogate input port values
      if (DFG[i]->opSrc[0] < 0) {
        for (j = 0; j < DFG[i]->opDestNu; j++) {
          destNu = DFG[i]->opDest[j];
          for (k = 0; k < 2; k++) {
            if (i == DFG[destNu]->opSrc[k])
              DFG[destNu]->opVal[k] = ports[i].f;
          }
        }
      } else {

        // do node operation
        opA.f = DFG[i]->opVal[0];
        opB.f = DFG[i]->opConst ? DFG[i]->opConstVal : DFG[i]->opVal[1];
        switch(DFG[i]->op) {
          case 0: res.f = opA.f + opB.f; break;
          case 1: res.f = opA.f * opB.f; break;
          case 2: res.f = opA.f / opB.f; break;
          case 3: res.f = opA.f - opB.f; break;
          default: {
            printf("Unknown op %d\n", DFG[i]->op); 
          }
        }

        // proprogate operation output result
        DFG[i]->opVal[2] = res.f;

        for (j = 0; j < DFG[i]->opDestNu; j++) {
          destNu = DFG[i]->opDest[j];
          if (destNu > 0) {
            for (k = 0; k < 2; k++) {
              if (i == DFG[destNu]->opSrc[k]) {
                if (k == 0)
                  DFG[destNu]->opVal[0] = DFG[i]->opVal[2];
                else if (!DFG[destNu]->opConst)
                  DFG[destNu]->opVal[1] = DFG[i]->opVal[2];
                else
                  DFG[destNu]->opVal[1] = DFG[destNu]->opConstVal;
              }
            }
          }
        }
      }
    }

    res.f = DFG[NODE_NU-1]->opVal[2]; // last numbered node
    //fprintf(cfp, "res = %08x(%.6f)\n", res.i, res.f);
    fprintf(cfp, "%08x\n", res.i);
  } // for testCount

  free(ports);
  fclose(fs); 
  fclose(cfp);

  // free debug structure 
  for (i = 0; i < NODE_NU; i++) {
    if (DFG[i]->opSrc[0] < 0) continue;
    else free(DFG[i]->opVal);
  } 
}

void CreateDFG() {
  int i;

  DFG = (opNode **) malloc(sizeof(opNode *) * NODE_NU);

  for (i = 0; i < NODE_NU; i++) {
    DFG[i] = (opNode *) malloc(sizeof(opNode));
  }

#ifdef PPBR
#include "ppbr.txt"
#endif
#ifdef ORDBBR
#include "ordbbr.txt"
#endif
#ifdef ORDUBR
#include "ordubr.txt"
#endif
#ifdef ORDBUR 
#include "ordbur.txt" 
#endif
#ifdef UUCI   
#include "uuci.txt"   
#endif
#ifdef SAMPLE8   
#include "sample8.txt"   
#endif
#ifdef SAMPLE9   
#include "sample9.txt"   
#endif
#ifdef PLF 
#include "plf.txt"   
#endif
}

void FreeDFG() {
  int i;
  for (i = 0; i < NODE_NU; i++) {
    free(DFG[i]->opSrc);
    free(DFG[i]->opDest);
    free(DFG[i]);
  }
  free(DFG);
}

int main (int argc, char **argv) {

  int testCnt     = 128;

#ifdef PPBR
  CircuitName = "PPBR";
  maxPortNu   = 13;
  NODE_NU     = 38;
#endif
#ifdef ORDBBR 
  CircuitName = "ORDBBR";
  maxPortNu   = 14;
  NODE_NU     = 48;
#endif
#ifdef ORDBUR 
  CircuitName = "ORDBUR";
  maxPortNu   = 10;
  NODE_NU     = 28;
#endif
#ifdef UUCI 
  CircuitName = "UUCI";
  maxPortNu   = 5;
  NODE_NU     = 13;
#endif
#ifdef SAMPLE8 
  CircuitName = "SAMPLE8";
  maxPortNu   = 3;
  NODE_NU     = 21;
#endif
#ifdef SAMPLE9 
  CircuitName = "SAMPLE9";
  maxPortNu   = 6;
  NODE_NU     = 32;
#endif
#ifdef PLF 
  CircuitName = "PLF";
  maxPortNu   = 8;
  NODE_NU     = 23;
#endif

  char result[50];
  sprintf(result, "TestData/%s.out", CircuitName);
  FILE *cfp = fopen(result, "w+");

  CreateDFG();

  GenerateTestData(testCnt);
  GenerateSWDebugData(cfp, testCnt);

  FreeDFG();

  return 0;
}
