//==============================================================================
// Xilinx Modelling of Floating-point operator v5.0
//==============================================================================

#include <stdio.h>
main() {

  union int32_or_single {
    int i;
    float f;
  } a, b, r, rs;

  union int64_or_double {
    //long long int i; // Please specify 64-bit integer type for platform
    long i;
    double f;
  } rd;

  // Uncomment to assign values in decimal
  //a.f = 15161.0;
  //b.f = 1077.00048828125;
  // Assign values in hexadecimal
  a.i=0x466CE400;
  b.i=0x4486A004;

  //Do the deed (in this example subtraction)
  r.f=a.f - b.f;

  //Repeat, but this time do in double precision to avoid rounding
  rd.f=(double) a.f - (double) b.f;
  printf("a: Hex=%08X ", a.i);
  printf("Float=%.11f\n", a.f);
  printf("b: Hex=%08X ", b.i);
  printf("Float=%.11f\n", b.f);
  printf("Single result: Hex=%08X ", r.i);
  printf("Float=%.11f\n", r.f);

  // A complex way ...
  printf("Double result: Hex=%08X%08X ",(int)(rd.i>>32),(int)(rd.i&0xFFFFFFFF));

  // A simpler way ...
  printf("Double result: Hex=%016llX ",rd.i);

  printf("Float=%.11f\n", rd.f);
  rs.f = (float) rd.f; // Round result from double to single
  printf("Double rounded to single: Hex=%08X ", rs.i);
  printf("Float=%.11f\n", rs.f);
  // Expect the following output:
  // a: Hex=466CE400 Float=15161.00000000000
  // b: Hex=4486A004 Float=1077.00048828125
  // Single precision result: Hex=465C1000 Float=14084.00000000000
  // Double precision result: Hex=40CB81FFF0000000 Float=14083.99951171875
  // Double rounded to single: Hex=465C1000 Float=14084.00000000000
}
