#include <stdio.h>
#include "Schedule.h"

int FloatingPointConvert(float num) {

  int32_or_float res;

  res.f = num;
  
  myprintf("Single float convert %.11f to %08x\n", res.f, res.i);

  return res.i;
}

long DoubleFloatingPointConvert(double num) {

  int64_or_double res;

  res.f = num;
  
  // xilinx fp doc
  //myprintf("Double float convert %.11f to %08x%08x\n", \
      res.f, (int)(res.i >> 32), (int)(res.i & 0xFFFFFFFF));

  myprintf("Double float convert %.11f to %016llx\n", res.f, res.i);

  return res.i;
}

