#include <stdio.h>
#include "Test_DFG.h"
float ordbur(FILE *fp, int32_or_float* ports, int maxPortPair) {
  int32_or_float res;
  res.f = ports[0].f * (ports[1].f * ports[2].f + ports[3].f/ports[4].f) / (ports[1].f*ports[2].f + ports[5].f*ports[2].f + ports[6].f*ports[1].f + (ports[0].f/(ports[7].f*ports[4].f)) * (ports[8].f + ports[3].f * (-1+ports[1].f/ports[9].f)));
  printf("Single ordbur res=%.11f\n", res.f);
  fprintf(fp, "%08x\n", res.i);
  return res.i;
}

float ppbr(FILE *fp, int32_or_float* ports, int maxPortPair) {
  int32_or_float res;
  res.f = ports[0].f * (ports[1].f * ports[2].f + ports[3].f * ports[4].f / ports[5].f) / ((ports[1].f * ports[2].f + ports[6].f * ports[1].f + ports[7].f * ports[2].f * (-1 + ports[4].f / ports[8].f)) + ports[0].f / (ports[9].f * ports[5].f) * (ports[10].f * ports[3].f * (-1 + ports[1].f / ports[11].f) + ports[4].f * (ports[12].f + ports[3].f)));
  printf("Single ppbr res=%.11f\n", res.f);
  fprintf(fp, "%08x\n", res.i);
  return res.i;
}

float uuci(FILE *fp, int32_or_float* ports, int maxPortPair) {
  int32_or_float res;
  res.f = (ports[0].f * ports[1].f / ports[2].f) / (-1.0 + (ports[1].f / ports[2].f * (-1 + ports[3].f / ports[4].f)));
  printf("Single uuci res=%.11f\n", res.f);
  fprintf(fp, "%08x\n", res.i);
  return res.i;
}

float ordbbr(FILE *fp, int32_or_float* ports, int maxPortPair) {
  int32_or_float res;
  res.f = ports[0].f * (ports[1].f * ports[2].f + ports[3].f * ports[4].f/ports[5].f) /        ((ports[1].f * ports[2].f * (-1 + ports[3].f / ports[6].f) + ports[7].f*(ports[1].f + ports[8].f) + ports[9].f * ports[2].f) +        (ports[0].f/(ports[10].f*ports[5].f)) * (ports[11].f * ports[3].f * (-1+ports[1].f/ports[8].f) +          ports[4].f * (ports[12].f * (-1 + ports[9].f * ports[2].f / (ports[8].f * ports[7].f) + ports[3].f * (-1 + ports[2].f / ports[13].f)))));
  printf("Single ordbbr res=%.11f\n", res.f);
  fprintf(fp, "%08x\n", res.i);
  return res.i;
}

float sample8(FILE *fp, int32_or_float* ports, int maxPortPair) {
  int32_or_float res;
  res.f = (ports[0].f * 3 + (ports[1].f * 0.5 + 2 + 4 * ports[0].f * ports[2].f) + 0.5) / 
          ((ports[0].f + 0.5) * 4 + ports[0].f * 4 + ports[1].f * 0.5 + ports[2].f * 3 + 0.5);
  printf("Single sample8 res=%.11f\n", res.f);
  fprintf(fp, "%08x\n", res.i);
  return res.i;
}

float sample9(FILE *fp, int32_or_float* ports, int maxPortPair) {
  int32_or_float res;
  res.f = (ports[0].f * 3 + (ports[1].f * 0.5 + 3 + 4 * ports[0].f * ports[2].f + ports[3].f * 2 + ports[4].f * 3) + 0.5) /           ((ports[0].f + 0.5) * 4 + ports[0].f * 4 + ports[5].f * 0.5 + ports[2].f * 3 + 0.5 + ports[3].f * 0.5 + ports[4].f * 0.5);
  printf("Single sample9 res=%.11f\n", res.f);
  fprintf(fp, "%08x\n", res.i);
  return res.i;
}
