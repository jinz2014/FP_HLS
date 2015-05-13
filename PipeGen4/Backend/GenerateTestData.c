#include <stdio.h>
#include <stdlib.h>
#include "Schedule.h"
#include "Benchmark.c"

int GenerateTestData(int testNu) {

  float (*benchmark) (FILE *, int32_or_float *, int) = NULL;
  double (*benchmark_d) (FILE *, int32_or_float *, int) = NULL;

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

  #ifdef LOAD_TEST_DATA
  fp1 = fopen(dataFileName, "r");
  #else
  fp1 = fopen(dataFileName, "w+");
  #endif

  if (fp1 == NULL) perror("Open TestData file error");

  fp2 = fopen(resultFileName, "w+");
  if (fp2 == NULL) perror("Open result file error");

  ports = (int32_or_float *) malloc (sizeof(int32_or_float) * MAX_PORT_NU);

  for (i = 0; i < testNu; i++) {
    for (j = 0; j < MAX_PORT_NU; j++) {  // 1/4/2011 based on port
      #ifdef LOAD_TEST_DATA
      fscanf(fp1, "%x", &(ports[j].i));
      #else
      ports[j].f = rand() / (float)(RAND_MAX);
      #endif
      fprintf(fp1, "%08X\n", ports[j].i);
      myprintf("port %d = %.11f(%x)\n", j + 1, ports[j].f, ports[j].i);
    }


    //-------------------------------------------------------
    // Call benchmark functions
    //-------------------------------------------------------

    // function return type is int/float
    if (!strcmp(CircuitName, "sample"))   benchmark = sample;
    if (!strcmp(CircuitName, "pos"))      benchmark = pos;
    if (!strcmp(CircuitName, "fir16"))    benchmark = fir16;
    if (!strcmp(CircuitName, "sop"))      benchmark = sop;
    if (!strcmp(CircuitName, "ratelaw"))  benchmark = ratelaw;
    if (!strcmp(CircuitName, "ratelaw1")) benchmark = ratelaw1;
    if (!strcmp(CircuitName, "sample1"))  benchmark = sample1;
    if (!strcmp(CircuitName, "sample8"))  benchmark = sample8;
    if (!strcmp(CircuitName, "sample9"))  benchmark = sample9;
    if (!strcmp(CircuitName, "power"))    benchmark = power;
    if (!strcmp(CircuitName, "tmax"))     benchmark = tmax;
    if (!strcmp(CircuitName, "mux4"))     benchmark = mux4;
    if (!strcmp(CircuitName, "mux2"))     benchmark = mux2;
    if (!strcmp(CircuitName, "log"))      benchmark = log_cheby;
    if (!strcmp(CircuitName, "uur"))      benchmark = uur;
    if (!strcmp(CircuitName, "uctr"))     benchmark = uctr;
    if (!strcmp(CircuitName, "umar"))     benchmark = umar;
    if (!strcmp(CircuitName, "uhmr"))     benchmark = uhmr;
    if (!strcmp(CircuitName, "umr"))      benchmark = umr;
    if (!strcmp(CircuitName, "ucti"))     benchmark = ucti;
    if (!strcmp(CircuitName, "umai"))     benchmark = umai;
    if (!strcmp(CircuitName, "umi"))      benchmark = umi;
    if (!strcmp(CircuitName, "unii"))     benchmark = unii;
    if (!strcmp(CircuitName, "uuci"))     benchmark = uuci;
    if (!strcmp(CircuitName, "uucr"))     benchmark = uucr;
    if (!strcmp(CircuitName, "ucii"))     benchmark = ucii;
    if (!strcmp(CircuitName, "ucir"))     benchmark = ucir;
    if (!strcmp(CircuitName, "uaii"))     benchmark = uaii;
    if (!strcmp(CircuitName, "uar"))      benchmark = uar;
    if (!strcmp(CircuitName, "ordbur"))   benchmark = ordbur;
    if (!strcmp(CircuitName, "ordubr"))   benchmark = ordubr;
    if (!strcmp(CircuitName, "ordbbr"))   benchmark = ordbbr;
    if (!strcmp(CircuitName, "ppbr"))     benchmark = ppbr;
    if (!strcmp(CircuitName, "random1"))  benchmark = random1;
    if (!strcmp(CircuitName, "random2"))  benchmark = random2;

    // function return type is long/double
    if (!strcmp(CircuitName, "f2d"))      benchmark_d = f2d;
    //if (!strcmp(CircuitName, "mrbay"))    benchmark_d = mrbay;
    //
    if (!strcmp(CircuitName, "mrbay"))    benchmark = mrbay;

    if (!strcmp(CircuitName, "plf"))    benchmark = plf;

    if (!strcmp(CircuitName, "dct"))    benchmark = dct;

    if (benchmark == NULL && benchmark_d == NULL) 
      fprintf(stderr, "Undefined circuit name\n");

    else if (benchmark)
      benchmark(fp2, ports, 1);

    else if (benchmark_d)
      benchmark_d(fp2, ports, 1);
  }

  free(ports);
  fclose(fp1);
  fclose(fp2);

  return testNu;
}

