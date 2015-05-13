#include "Schedule.h"
#include <assert.h>

/* A sample test */
float sample(FILE *fp, int32_or_float* ports, int maxPortPair) {
  int32_or_float res;
 
  res.f = ((ports[0].f + ports[1].f) / (ports[0].f * ports[1].f));

  myprintf("Single sample res=%.11f\n", res.f);
  fprintf(fp, "%08x\n", res.i);

  return res.i;
}


  /* PLF 
    res = (a1 * a2 + b1 * b2 + c1 * c2 + d1 * d2) * 
          (e1 * e2 + f1 * f2 + g1 * g2 + h1 * h2);
   */
float plf(FILE *fp, int32_or_float* ports, int maxPortPair) {
  int n;
  int32_or_float res1, res2, res;
  res1.f = 0;
  res2.f = 0;

  for (n = 0; n < maxPortPair / 2; n++) {
    res1.f += ports[n].f * ((n + 1) / 10.0);
  }

  for (n = maxPortPair / 2; n < maxPortPair; n++) {
    res2.f += ports[n].f * ((n + 1) / 10.0);
  }

  res.f = res1.f * res2.f;

  //myprintf("Single plf res=%.11f\n", res.f); 

  fprintf(fp, "%08x\n", res.i);

  return res.f;
}


  /* PLF 
    res = (a1 * a2 + b1 * b2 + c1 * c2 + d1 * d2) * 
          (e1 * e2 + f1 * f2 + g1 * g2 + h1 * h2);
   */
float pos(FILE *fp, int32_or_float* ports, int maxPortPair) {
  int n;
  int32_or_float res1, res2, res;
  res1.f = 0;
  res2.f = 0;

  for (n = 0; n < maxPortPair / 2; n += 2) {
    res1.f += ports[n].f * ports[n+1].f;
  }

  for (n = maxPortPair / 2; n < maxPortPair; n += 2) {
    res2.f += ports[n].f * ports[n+1].f;
  }

  res.f = res1.f * res2.f;

  //myprintf("Single plf res=%.11f\n", res.f); 

  fprintf(fp, "%08x\n", res.i);

  return res.f;
}

/*  Inner product
  res = (a1 * a2 + b1 * b2 + c1 * c2 + d1 * d2) +
        (e1 * e2 + f1 * f2 + g1 * g2 + h1 * h2);
   */
float sop(FILE *fp, int32_or_float* ports, int maxPortPair) {
  int n;
  int32_or_float res;

  for (n = 0; n < maxPortPair; n =+ 2) {
    res.f += ports[n].f * ports[n + 1].f;
  }

  myprintf("Single sop res=%.11f\n", res.f); 

  fprintf(fp, "%08x\n", res.i);

  return res.i;
}

/* 16-point FIR */
float fir16(FILE *fp, int32_or_float* ports, int maxPortPair) {
  int n;
  int32_or_float res;
  res.f = 0;

  for (n = 0; n < maxPortPair; n += 2) {
    res.f += (ports[n].f + ports[n+1].f) * (-1);
  }

  myprintf("Single fir16 res=%.11f\n", res.f);
  fprintf(fp, "%08x\n", res.i);

  return res.i;
}

/* port-constrained scheduling 09 */
float ratelaw(FILE *fp, int32_or_float* a, int maxPortPair) {
  int32_or_float res;
  res.f = 0;
 
  res.f = ((a[0].f * a[1].f) / (a[1].f / a[2].f)) / 
          (-1 + (a[1].f / a[2].f) + (a[3].f / a[4].f));

  myprintf("Single ratelaw res=%.11f\n", res.f);
  fprintf(fp, "%08x\n", res.i);

  return res.i;
}

/* Efficient scheduling 2005 */
float ratelaw1(FILE *fp, int32_or_float* a, int maxPortPair) {
  int32_or_float res;
  res.f = 0;
 
  res.f = ((a[0].f * a[1].f) / a[4].f) / 
          ((a[3].f + a[2].f + a[0].f + a[1].f) * (a[3].f + a[2].f));

  myprintf("Single ratelaw1 res=%.11f\n", res.f);
  fprintf(fp, "%08x\n", res.i);

  return res.i;
}
  
/* motivating example in Kim's register allocation paper */
float sample1(FILE *fp, int32_or_float* ports, int maxPortPair) {
  int32_or_float res;
 
  res.f = (ports[0].f + ports[1].f + ports[1].f) + (ports[0].f + ports[1].f) +
          (ports[2].f + ports[3].f + ports[2].f) + (ports[2].f + ports[3].f);

  myprintf("Single sample1 res=%.11f\n", res.f);
  fprintf(fp, "%08x\n", res.i);

  return res.i;
}

float uur(FILE *fp, int32_or_float* ports, int maxPortPair) {
  int32_or_float res;

  res.f = (((ports[0].f * ports[1].f) / ports[2].f) - 
           ((ports[3].f * ports[4].f) / ports[5].f)) / 
          (-1 + (ports[1].f / ports[2].f) + (ports[4].f / ports[5].f));

  myprintf("Single uur res=%.11f\n", res.f);
  fprintf(fp, "%08x\n", res.i);

  return res.i;
}


float uctr(FILE *fp, int32_or_float* ports, int maxPortPair) {
  int32_or_float res;
  //
  //        Vf*S/KmS - Vr*P/KmP
  // -------------------------------------
  // 1 + Ka/Ac + (S/Kms + P/Kmp)*(1+Ka/Ac)
  //
  // Ports
  // Vf, S, Vr, P, Kms, Kmp, Ka, Kc

  //res.f = (((ports[0].f * ports[1].f) / ports[4].f) - 
           //((ports[2].f * ports[3].f) / ports[5].f)) / 
  res.f = (((ports[0].f * ports[1].f) / ports[4].f) +
           ((ports[2].f * ports[3].f) / ports[5].f)) / 
          (-1 + (ports[6].f / ports[7].f) + 
          (ports[1].f / ports[4].f + ports[3].f / ports[5].f) * 
          (-1 + ports[6].f / ports[7].f));

  myprintf("Single uctr res=%.11f\n", res.f);
  fprintf(fp, "%08x\n", res.i);

  return res.i;
}

float umar(FILE *fp, int32_or_float* ports, int maxPortPair) {
  int32_or_float res;
  int32_or_float temp1, temp2, temp3, temp4, temp5, temp6;
  //
  //        Vf*S/KmS - Vr*P/KmP
  // -------------------------------------
  // 1 + Kas/Ac + (S/Kms + P/Kmp)*(1+Kac/Ac)
  //
  // Ports
  // 0   1  2   3   4    5   6    7   8
  // Vf, S, Vr, P, Kms, Kmp, Kac, Ac, Kas 

  //res.f = (((ports[0].f * ports[1].f) / ports[4].f) - 
  res.f = (((ports[0].f * ports[1].f) / ports[4].f) +
           ((ports[2].f * ports[3].f) / ports[5].f)) / 
          (-1 + (ports[8].f / ports[7].f) + 
          (ports[1].f / ports[4].f + ports[3].f / ports[5].f) * 
          (-1 + ports[6].f / ports[7].f));

  myprintf("Single umar res=%.11f\n", res.f);
  fprintf(fp, "%08x\n", res.i);

  return res.i;
}

float uai(FILE *fp, int32_or_float* ports, int maxPortPair) {
  int32_or_float res, 
                 S, Ksa, Ksc, V,
                 temp1, temp2;
  //
  //        V * (S/Ksa)^2
  // ----------------------------------
  //   -1 + S/Ksc + (S/Ksa)^2 + S/Ksa
  //
  //
  // Ports
  // 0
  // S V Ksa  Ksc
  //
  S   = ports[0];
  Ksa = ports[1];
  Ksc = ports[2];
  V   = ports[3];

  temp1.f = V.f * (S.f / Ksa.f) * (S.f / Ksa.f);
  temp2.f = -1 + (S.f / Ksc.f)  + (S.f / Ksa.f) * (S.f / Ksa.f) + S.f / Ksa.f;

  myprintf("uai temp1=%08x(%.11f)\n", temp1.i, temp1.f);
  myprintf("uai temp2=%08x(%.11f)\n", temp2.i, temp2.f);

  res.f = temp1.f / temp2.f;

  myprintf("Single uai res=%08x(%.11f)\n", res.i, res.f);
  fprintf(fp, "%08x\n", res.i);
  
  return res.i;
}

float uhmr(FILE *fp, int32_or_float* ports, int maxPortPair) {
  int32_or_float res, 
                 S, P, M,
                 Vf, Vr, 
                 Kms, Kmp, Kd,
                 a, b,
                 temp1, temp2;
  //
  //        (Vf*S/Kms - Vr*P/Kmp)[-1 + bM/(aKd)]
  // ----------------------------------------------
  //   -1 + M/Kd + (S/Kms + P/Kmp)[1 + M/(aKd)]
  //
  // K1 = [Vf/(Vr*Keq)][Kmq*P(-1+A/Kia)+Q*(Kmp + P)] 
  //
  // Ports
  // 0                 6                    12
  // A B Kmb Kma Q Kiq Vr Keq Kmq P Kia Kmp Vf 

  //
  Vf  = ports[0];
  S   = ports[1];
  Vr  = ports[2];
  P   = ports[3];
  b   = ports[4];
  M   = ports[5];
  a   = ports[6];
  Kd  = ports[7];
  Kms = ports[8];
  Kmp = ports[9];
  
  temp1.f = (Vf.f * S.f / Kms.f - Vr.f * P.f / Kmp.f) * (-1 + b.f * M.f / (a.f * Kd.f));
  temp2.f = -1 + M.f / Kd.f + (S.f / Kms.f + P.f / Kmp.f) * (-1 + M.f / (a.f * Kd.f));

  myprintf("uhmr temp1=%08x(%.11f)\n", temp1.i, temp1.f);
  myprintf("uhmr temp2=%08x(%.11f)\n", temp2.i, temp2.f);

  res.f = temp1.f / temp2.f;

  myprintf("Single uhmr res=%08x(%.11f)\n", res.i, res.f);
  fprintf(fp, "%08x\n", res.i);
  
  return res.i;
}


float umr(FILE *fp, int32_or_float* ports, int maxPortPair) {
  int32_or_float res;
  //
  //        Vf*S/KmS - Vr*P/KmP
  // -------------------------------------
  // 1 + I/Kis + (S/Kms + P/Kmp)*(1+I/Kic)
  //
  // Ports
  // 0                                8
  // Vf, S, Vr, P, Kms, Kmp, I, Kic, Kis 

  //res.f = (((ports[0].f * ports[1].f) / ports[4].f) - 
  res.f = (((ports[0].f * ports[1].f) / ports[4].f) +
           ((ports[2].f * ports[3].f) / ports[5].f)) / 
          (-1 + (ports[6].f / ports[8].f) + 
          (ports[1].f / ports[4].f + ports[3].f / ports[5].f) * 
          (-1 + ports[6].f / ports[7].f));

  myprintf("Single umr res=%.11f\n", res.f);
  fprintf(fp, "%08x\n", res.i);

  return res.i;
}

float ucti(FILE *fp, int32_or_float* ports, int maxPortPair) {
  int32_or_float res;
  //
  //        V*S/Km
  // ------------------------------
  // -1 + Ka/Ac + (S/Km)*(-1+Ka/Ac)
  //
  // Ports
  // 0             4
  // V, S, Km, Ka, Ac
  // 4, 2, 2,   2  1 (2)
  // 9, 4, 2,   2  1 (6)

  res.f = ((ports[0].f * ports[1].f) / ports[2].f) /
          (-1 + (ports[3].f / ports[4].f) + 
          (ports[1].f / ports[2].f) * (-1 + ports[3].f / ports[4].f));

  myprintf("Single ucti res=%.11f\n", res.f);
  fprintf(fp, "%08x(%.6f)\n", res.i, res.f);

  return res.i;
}

float umai(FILE *fp, int32_or_float* ports, int maxPortPair) {
  int32_or_float res;
  //
  //        V*S/Km
  // ------------------------------
  // -1 + Kas/Ac + (S/Km)*(-1+Kac/Ac)
  //
  // Ports
  // 0             4
  // V, S, Km, Kac, Ac, Kas
  // 4, 2, 2,   2    1   2 (2)
  // 9, 4, 2,   2    1   2 (6)

  res.f = ((ports[0].f * ports[1].f) / ports[2].f) /
          (-1 + (ports[5].f / ports[4].f) + 
          (ports[1].f / ports[2].f) * (-1 + ports[3].f / ports[4].f));

  myprintf("Single umai res=%.11f\n", res.f);
  fprintf(fp, "%08x\n", res.i);

  return res.i;
}


float umi(FILE *fp, int32_or_float* ports, int maxPortPair) {
  int32_or_float res;
  //
  //        V*S/Km
  // ------------------------------
  // -1 + I/Kis + (S/Km)*(-1+I/Kic)
  //
  // Ports
  // 0                  5
  // V, S, Km,  I, Kic, Kis

  res.f = ((ports[0].f * ports[1].f) / ports[2].f) /
          (-1 + (ports[3].f / ports[5].f) + 
          (ports[1].f / ports[2].f) * (-1 + ports[3].f / ports[4].f));

  myprintf("Single umi res=%.11f\n", res.f);
  fprintf(fp, "%08x\n", res.i);

  return res.i;
}

float unii(FILE *fp, int32_or_float* ports, int maxPortPair) {
  int32_or_float res;
  //
  //        V*S/Km
  // ------------------------------
  // -1 + I/Kis + (S/Km)*(-1+I/Kic)
  //
  // Ports
  // 0                  5
  // V, S, Km,  I, Kic, Kis

  res.f = ((ports[0].f * ports[1].f) / ports[2].f) /
          (-1 + (ports[3].f / ports[5].f) + 
          (ports[1].f / ports[2].f) * (-1 + ports[3].f / ports[4].f));

  myprintf("Single unii res=%.11f\n", res.f);
  fprintf(fp, "%08x\n", res.i);

  return res.i;
}

float uuci(FILE *fp, int32_or_float* ports, int maxPortPair) {
  int32_or_float res;
  //
  //        V*S/Km
  // ----------------------
  // -1 + (S/Km)*(-1+I/Ki)
  //
  // Ports
  // 0             4
  // V, S, Km,  I, Ki
  // 4, 2, 2,   4   1   4/-1+(1)*3 = 2
  // 9, 4, 2,   2   1   18/-1+(2)*1 = 18(0x12)

  res.f = ((ports[0].f * ports[1].f) / ports[2].f) /
          (-1 + 
          (ports[1].f / ports[2].f) * (-1 + ports[3].f / ports[4].f));

  myprintf("Single uuci res=%.11f\n", res.f);
  fprintf(fp, "%08x(%.6f)\n", res.i, res.f);

  return res.i;
}

float uaii(FILE *fp, int32_or_float* ports, int maxPortPair) {
  int32_or_float res;
  //
  //        V*S/Km
  // ----------------------
  // -1 + (S/Km)+(Ka/Ac)
  //
  // Ports
  // 0             4
  // V, S, Km,  Ka, Ac
  // 4, 2, 2,   4   1  4/-1+1+4 = 1
  // 9, 4, 2,   2   1  18/-1+2+2 = 6

  res.f = ((ports[0].f * ports[1].f) / ports[2].f) /
          (-1 + (ports[1].f / ports[2].f) + (ports[3].f / ports[4].f));

  myprintf("Single uaii res=%.11f\n", res.f);
  fprintf(fp, "%08x(%.6f)\n", res.i, res.f);

  return res.i;
}

float ucii(FILE *fp, int32_or_float* ports, int maxPortPair) {
  int32_or_float res;
  //
  //        V*S/Km
  // ----------------------
  // -1 + (S/Km)+(Ka/Ac)
  //
  // Ports
  // 0              4
  // V, S, Km,  I,  Ki
  // 4, 2, 2,   4   1  4/-1+1+4 = 1
  // 9, 4, 2,   2   1  18/-1+2+2 = 6

  res.f = ((ports[0].f * ports[1].f) / ports[2].f) /
          (-1 + (ports[1].f / ports[2].f) + (ports[3].f / ports[4].f));

  myprintf("Single ucii res=%.11f\n", res.f);
  fprintf(fp, "%08x\n", res.i);

  return res.i;
}

float uar(FILE *fp, int32_or_float* ports, int maxPortPair) {
  int32_or_float res;
  //
  //        Vf*S/KmS - Vr*P/KmP
  // ------------------------------
  //   -1 + S/Kms + P/Kmp + Ka/Ac
  //
  // Ports
  // 0                           7
  // Vf, S, Vr, P, Kms, Kmp, Ka, Ac
  // 8   2  2   2   2    1   6   3   
  // 8*2/2 - 2*2/1 = 4
  // -1 + 1 + 2 + 2 = 4
  // 
  // 8   2  1   2   2    1   4   4   
  // 8*2/2 - 1*2/1 = 6
  // -1 + 1 + 2 + 1 = 3

  //res.f = (((ports[0].f * ports[1].f) / ports[4].f) - 
  res.f = (((ports[0].f * ports[1].f) / ports[4].f) +
           ((ports[2].f * ports[3].f) / ports[5].f)) / 
          (-1 + ports[1].f / ports[4].f + 
          ports[3].f / ports[5].f + ports[6].f / ports[7].f);

  myprintf("Single uar res=%.11f\n", res.f);
  fprintf(fp, "%08x\n", res.i);

  return res.i;
}

float ucir(FILE *fp, int32_or_float* ports, int maxPortPair) {
  int32_or_float res;
  //
  //        Vf*S/KmS - Vr*P/KmP
  // ------------------------------
  //   -1 + S/Kms + P/Kmp + Ka/Ac
  //
  // Ports
  // 0                           7
  // Vf, S, Vr, P, Kms, Kmp, I, Ki
  // 8   2  2   2   2    1   6   3   
  // 8*2/2 - 2*2/1 = 4
  // -1 + 1 + 2 + 2 = 4
  // 
  // 8   2  1   2   2    1   4   4   
  // 8*2/2 - 1*2/1 = 6
  // -1 + 1 + 2 + 1 = 3

  res.f = (((ports[0].f * ports[1].f) / ports[4].f) - 
           ((ports[2].f * ports[3].f) / ports[5].f)) / 
          (-1 + ports[1].f / ports[4].f + 
          ports[3].f / ports[5].f + ports[6].f / ports[7].f);

  myprintf("Single ucir res=%.11f\n", res.f);
  fprintf(fp, "%08x\n", res.i);

  return res.i;
}

float ordbbr(FILE *fp, int32_or_float* ports, int maxPortPair) {
  int32_or_float res, Vf, A, B, P, Q, 
                 Vr, Keq, 
                 Kip, Kia, Kib, 
                 Kma, Kmb, Kmq, Kmp,
                 K1, K2,
                 temp1, temp2;
  //
  //        Vf(AB - PQ/Keq) = 5(2-1)
  // -------------------------------------
  // AB(-1+P/Kip) + Kmb(A+Kia) + Kma*B + K1 = 2(-1+1/2) + 1(2+1) + 1 * 1 + 1 =
  // 5/(1+1+1) = 1
  //
  // K1 = [Vf/(Vr*Keq)][Kmq*P(-1+A/Kia)+Q*K2)] 
  //      [5 / (5*1)] * [1 * 1 (-1 + 2/1) + 0] = 1  
  // K2 = Kmp[-1+Kma*B/(Kia*Kmb)+P(-1+B/Kib)]]
  //      1 * (-1 + 1 * 1 / (1 * 1) + 1 (-1 + 1/1)) = 0
  //
  // Ports
  // 0                 6                        13
  // A B P Kip Kia Kma Vr Keq Kmq Kmb Kib Vf Q Kmp
  // 2 1 1 2   1   1   5  1  1   1    1   5 1  1

  //        Vf(AB - PQ/Keq) = 12(2-1)
  // -------------------------------------
  // AB(-1+P/Kip) + Kmb(A+Kia) + Kma*B + K1 = 2(-1+1/2) + 1(2+1) + 1 * 1 + 4 =
  // 12/(1+1+4) = 2
  //
  // K1 = [Vf/(Vr*Keq)][Kmq*P(-1+A/Kia)+Q*K2)] 
  //      [12 / (3*1)] * [1 * 1 (-1 + 2/1) + 0] = 4  
  // K2 = Kmp[-1+Kma*B/(Kia*Kmb)+P(-1+B/Kib)]]
  //      1 * (-1 + 1 * 1 / (1 * 1) + 1 (-1 + 1/1)) = 0

  // 2 1 1 2   1   1   3  1  1   1    1   12 1  1
  // 
  //
  A   = ports[0];
  B   = ports[1];
  P   = ports[2];
  Kip = ports[3];
  Kia = ports[4];
  Kma = ports[5];
  Vr  = ports[6];
  Keq = ports[7];
  Kmq = ports[8];
  Kmb = ports[9];
  Kib = ports[10];
  Vf  = ports[11];
  Q   = ports[12];
  Kmp = ports[13];
  
  // A B P Kip Kia Kma Vr Keq Kmq Kmb Kib Vf Q Kmp
  // 2 1 1 2   1   1   5  1  1   1    1   5 1  1
  K2.f = Kmp.f*(-1+(Kma.f*B.f)/(Kia.f * Kmb.f) + P.f * (-1 + B.f/Kib.f));

  K1.f = (Vf.f / (Vr.f * Keq.f)) * (Kmq.f*P.f*(-1+A.f/Kia.f) + Q.f* K2.f);

  //temp1.f = Vf.f * (A.f*B.f - P.f*Q.f/ Keq.f);
  temp1.f = Vf.f * (A.f*B.f + P.f*Q.f/ Keq.f);

  temp2.f = A.f*B.f*(-1 + P.f/Kip.f) + Kmb.f * (A.f + Kia.f) + Kma.f*B.f + K1.f;
  res.f = temp1.f / temp2.f;

  myprintf("Single ordbbr res=%d\n", res.i);
  //fprintf(fp, "%08x\n", res.i);
  fprintf(fp, "%08x(%.6f)\n", res.i, res.f);
  
  return res.i;
}

float ordbur(FILE *fp, int32_or_float* ports, int maxPortPair) {
  int32_or_float res, 
                 A, B, P,
                 Vf, Vr, 
                 Kma, Kmb, Kmp, 
                 Kia, Keq, 
                 temp1, temp2;
  //
  //                  Vf(AB - P/Keq)
  // --------------------------------------------------------
  //  A*B + Kma*B + Kmb*A + [Vf/(Vr*Keq)][Kmp + P(-1+A/Kia)]
  //
  // Ports
  // 0                 6                    12
  // A B Kmb Kma Q Kiq Vr Keq Kmq P Kia Kmp Vf 

  //
  A   = ports[0];
  B   = ports[1];
  Kma = ports[2];
  Kmb = ports[3];
  Vr  = ports[4];
  Keq = ports[5];
  P   = ports[6];
  Kia = ports[7];
  Kmp = ports[8];
  Vf  = ports[9];
  
  //temp1.f = Vf.f*(A.f*B.f - P.f/ Keq.f);
  temp1.f = Vf.f*(A.f*B.f + P.f/ Keq.f);
  temp2.f = A.f*B.f + Kma.f * B.f + Kmb.f * A.f + 
            (Vf.f / (Vr.f * Keq.f)) * (Kmp.f + P.f * (-1+A.f / Kia.f));

  myprintf("ordbur temp1=%08x(%.11f)\n", temp1.i, temp1.f);
  myprintf("ordbur temp2=%08x(%.11f)\n", temp2.i, temp2.f);

  res.f = temp1.f / temp2.f;

  myprintf("Single ordbur res=%08x(%.11f)\n", res.i, res.f);
  fprintf(fp, "%08x(%.6f)\n", res.i, res.f);
  
  return res.i;
}

float ordubr(FILE *fp, int32_or_float* ports, int maxPortPair) {
  int32_or_float res, 
                 A, P, Q,
                 Vf, Vr, 
                 Kma, Kmq, Kmp, 
                 Kip, Keq, 
                 temp1, temp2;
  //
  //                  Vf(AB - P/Keq)
  // --------------------------------------------------------
  //  A*B + Kma*B + Kmb*A + [Vf/(Vr*Keq)][Kmp + P(-1+A/Kia)]
  //
  // Ports
  // 0                 6                    12
  // A B Kmb Kma Q Kiq Vr Keq Kmq P Kia Kmp Vf 

  //
  P   = ports[0];
  Q   = ports[1];
  Kip = ports[2];
  Vr  = ports[3];
  Keq = ports[4];
  Kmq = ports[5];
  Kmp = ports[6];
  Vf  = ports[7];
  A   = ports[8];
  Kma = ports[9];
  
  //temp1.f = Vf.f*(A.f - P.f * Q.f / Keq.f);
  temp1.f = Vf.f*(A.f + P.f * Q.f / Keq.f);
  temp2.f = Kma.f + A.f * (-1 + P.f / Kip.f) + 
            (Vf.f / (Vr.f * Keq.f)) * (Kmq.f * P.f + Kmp.f * Q.f + P.f * Q.f);

  myprintf("ordubr temp1=%08x(%.11f)\n", temp1.i, temp1.f);
  myprintf("ordubr temp2=%08x(%.11f)\n", temp2.i, temp2.f);

  res.f = temp1.f / temp2.f;

  myprintf("Single ordubr res=%08x(%.11f)\n", res.i, res.f);
  fprintf(fp, "%08x\n", res.i);
  
  return res.i;
}


float ppbr(FILE *fp, int32_or_float* ports, int maxPortPair) {
  int32_or_float res, 
                 A, B, P, Q, 
                 Vf, Vr, 
                 Kia, Kiq, 
                 Kma, Kmb, Kmp, Kmq,
                 Keq, 
                 K1, temp1, temp2;
  //
  //        Vf(AB - PQ/Keq)
  // -------------------------------------
  //  A*B + Kmb*A + Kma*B(1+Q/Kiq) + K1 
  //
  // K1 = [Vf/(Vr*Keq)][Kmq*P(-1+A/Kia)+Q*(Kmp + P)] 
  //
  // Ports
  // 0                 6                    12
  // A B Kmb Kma Q Kiq Vr Keq Kmq P Kia Kmp Vf 

  //
  A   = ports[0];
  B   = ports[1];
  Kmb = ports[2];
  Kma = ports[3];
  Q   = ports[4];
  Kiq = ports[5];
  Vr  = ports[6];
  Keq = ports[7];
  Kmq = ports[8];
  P   = ports[9];
  Kia = ports[10];
  Kmp = ports[11];
  Vf  = ports[12];
  
  K1.f = (Vf.f / (Vr.f * Keq.f)) * (Kmq.f * P.f * (-1+A.f / Kia.f) + Q.f * (Kmp.f + P.f));
  myprintf("Single ppbr K1=%08x(%.11f)\n", K1.i, K1.f);

  //temp1.f = Vf.f*(A.f*B.f - P.f*Q.f/ Keq.f);
  temp1.f = Vf.f*(A.f*B.f + P.f*Q.f/ Keq.f);
  temp2.f = A.f*B.f + Kmb.f * A.f + Kma.f*B.f*(-1+Q.f/Kiq.f) + K1.f;

  myprintf("ppbr temp1=%08x(%.11f)\n", temp1.i, temp1.f);
  myprintf("ppbr temp2=%08x(%.11f)\n", temp2.i, temp2.f);

  res.f = temp1.f / temp2.f;

  myprintf("Single ppbr res=%08x(%.11f)\n", res.i, res.f);
  //fprintf(fp, "%08x\n", res.i);
  fprintf(fp, "%08x(%.6f)\n", res.i, res.f);
  
  return res.i;
}

float random1(FILE *fp, int32_or_float* ports, int maxPortPair) {
  int32_or_float res, 
                 a, b, c, d,
                 t1, t2, t3, t4, t5, t6, t7,
                 t8, t10, t11, t13, t14,
                 t19, t23, t30, t37;
  //
  //        V * (S/Ksa)^2
  // ----------------------------------
  //   -1 + S/Ksc + (S/Ksa)^2 + S/Ksa
  //
  //
  // Ports
  // 0
  // a,b,c,d
  //
  a = ports[0];
  b = ports[1];
  c = ports[2];
  d = ports[3];

  t1.f = a.f + b.f;
  t2.f = c.f * d.f;
  t3.f = (t1.f + t2.f);
  t4.f = (t1.f + t3.f);
  t5.f = (t3.f * t3.f);
  t6.f = (t4.f + t5.f);
  t7.f = (t1.f * t4.f);
  t8.f = (t3.f + t5.f);
  t10.f = (t3.f + t7.f);
  t11.f = (t8.f + t2.f);
  t13.f = (t7.f + t6.f);
  t14.f = (t6.f * t8.f);
  t19.f = (t14.f + t5.f);
  t23.f = (t10.f * t13.f);
  t30.f = (t19.f * t11.f);
  t37.f = (t23.f + t14.f);

  res.f = t37.f + t30.f;

  myprintf("Single random1 res=%08x(%.11f)\n", res.i, res.f);
  fprintf(fp, "%08x\n", res.i);
  
  return res.i;
}

/*
float random2(FILE *fp, int32_or_float* ports, int maxPortPair) {
  int32_or_float res, 
                 a, b, c, d,
                 t1, t2, t3, t4, t5, t6, t7,
                 t8, t10, t11, t13, t14,
                 t19, t23, t30, t37;
  // Ports
  // 0
  // a,b,c,d
  //
  a = ports[0];
  b = ports[1];
  c = ports[2];
  d = ports[3];

  t1.f = a.f + b.f;
  t2.f = c.f * d.f;
  t3.f = (t1.f + t2.f);
  t4.f = (t1.f + t3.f);
  t5.f = (t3.f * t3.f);
  t6.f = (t4.f + t5.f);
  t7.f = (t1.f * t4.f);
  t8.f = (t3.f + t5.f);
  t10.f = (t3.f + t7.f);
  t11.f = (t8.f + t2.f);
  t13.f = (t7.f + t6.f);
  t14.f = (t6.f * t8.f);
  t19.f = (t13.f + t6.f);
  t30.f = (t19.f * t11.f);
  t23.f = (t10.f * t3.f);
  t37.f = (t23.f + t14.f);

  res.f = t37.f + t30.f;

  myprintf("Single random2 res=%08x(%.11f)\n", res.i, res.f);
  fprintf(fp, "%08x\n", res.i);
  
  return res.i;
}
*/
float random2(FILE *fp, int32_or_float* ports, int maxPortPair) {
  int32_or_float res, 
                 a, b, c, d,
                 t1, t2, t3, t4, t5, t6, t7,
                 t8, t10, t11, t13, t14,
                 t19, t23, t30, t37;
  a = ports[0];
  b = ports[1];
  c = ports[2];
  d = ports[3];

  t1.f = a.f + b.f;
  t2.f = c.f + d.f;
  t3.f = (t1.f + t2.f);
  t4.f = (t1.f + t3.f);
  t5.f = (t3.f + t3.f);
  t6.f = (t4.f + t5.f);
  t7.f = (t1.f + t4.f);
  t8.f = (t3.f + t5.f);
  t10.f = (t3.f + t7.f);
  t11.f = (t8.f + t2.f);
  t13.f = (t7.f + t6.f);
  t14.f = (t6.f + t8.f);
  t19.f = (t13.f + t6.f);
  t30.f = (t19.f + t11.f);
  t23.f = (t10.f + t3.f);
  t37.f = (t23.f + t14.f);

  res.f = t37.f + t30.f;

  myprintf("Single random2 res=%08x(%.11f)\n", res.i, res.f);
  fprintf(fp, "%08x\n", res.i);
  
  return res.i;
}


float uucr(FILE *fp, int32_or_float* ports, int maxPortPair) {
  int32_or_float res;
  //
  //        Vf*S/KmS - Vr*P/KmP
  // -------------------------------------
  // 1 + (S/Kms + P/Kmp)*(1+I/Ki)
  //
  // Ports
  // 0                          7
  // Vf, S, Vr, P, Kms, Kmp, I, Ki

  //res.f = (((ports[0].f * ports[1].f) / ports[4].f) - 
  res.f = (((ports[0].f * ports[1].f) / ports[4].f) +
           ((ports[2].f * ports[3].f) / ports[5].f)) / 
          (-1 + (ports[1].f / ports[4].f + ports[3].f / ports[5].f) * 
          (-1 + ports[6].f / ports[7].f));

  myprintf("Single uucr res=%.11f\n", res.f);
  fprintf(fp, "%08x\n", res.i);

  return res.i;
}

float sample8(FILE *fp, int32_or_float* ports, int maxPortPair) {
  int32_or_float res;
  res.f = (ports[0].f * 3 + (ports[1].f * 0.5 + 2 + 4 * ports[0].f * ports[2].f) + 0.5) / 
          ((ports[0].f + 0.5) * 4 + ports[0].f * 4 + ports[1].f * 0.5 + ports[2].f * 3 + 0.5);
  myprintf("Single sample8 res=%.11f\n", res.f);
  fprintf(fp, "%08x\n", res.i);
  return res.i;
}

float sample9(FILE *fp, int32_or_float* ports, int maxPortPair) {
  int32_or_float res;
  res.f = (ports[0].f * 3 + (ports[1].f * 0.5 + 3 + 4 * ports[0].f * ports[2].f + ports[3].f * 2 + ports[4].f * 3) + 0.5) /           ((ports[0].f + 0.5) * 4 + ports[0].f * 4 + ports[5].f * 0.5 + ports[2].f * 3 + 0.5 + ports[3].f * 0.5 + ports[4].f * 0.5);
  myprintf("Single sample9 res=%.11f\n", res.f);
  fprintf(fp, "%08x\n", res.i);
  return res.i;
}

float log_cheby(FILE *fp, int32_or_float* ports, int log) {
  int32_or_float res;
  int32_or_float x, x2, x3, x4;
  int32_or_float mx2, mx4, mx5, mx6, mx8;
  int32_or_float mx9, mx10, mx11, mx12, mx13, mx14;
  int a3, a2, a1, a0;
  int i, j;

  //----------------------------------------------------------------
  // A simple memory (depth x words)
  //----------------------------------------------------------------
  float val = 0.01; // initial memory fill value
  int32_or_float **mem;

  mem = (int32_or_float **) malloc (sizeof(int32_or_float *) * WORDS);
  for (j = 0; j < WORDS; j++) {
    mem[j] = (int32_or_float *) malloc (sizeof(int32_or_float) * DEPTH);
    for (i = 0; i < DEPTH; i++) {
      mem[j][i].f = val++;  
      myprintf("single memory %08x(%.11f)\n", mem[j][i].i, mem[j][i].f);
    }
    myprintf("\n");
  }

  //-------------------------------------------------------------
  // log input x
  //-------------------------------------------------------------
  x.f  = ports[0].f;

  //-------------------------------------------------------------
  // select range
  //-------------------------------------------------------------
  // 2-1 MUX
  mx2.f = (x.f > 10e-16) ? 10e-8 : 10e-24;

  // 4-1 MUX
  mx4.f = (x.f > mx2.f)  ? 10e-20 : 10e-28;
  mx5.f = (x.f > mx2.f)  ? 10e-4  : 10e-12;
  mx6.f = (x.f > 10e-16) ? mx5.f  : mx4.f;

  // 8-1 MUX
  mx8.f  = (x.f > mx6.f) ? 10e-26 : 10e-30;
  mx9.f  = (x.f > mx6.f) ? 10e-18 : 10e-22;
  mx10.f = (x.f > mx6.f) ? 10e-10 : 10e-14; 
  mx11.f = (x.f > mx6.f) ? 10e-2  : 10e-6; 

  mx12.f = (x.f > mx2.f) ? mx9.f  : mx8.f;
  mx13.f = (x.f > mx2.f) ? mx11.f : mx10.f;

  mx14.f = (x.f > 10e-16) ? mx13.f : mx12.f;

  //------------------------------------------------------------
  // memory address
  //------------------------------------------------------------
  a0 = x.f > mx14.f;
  a1 = x.f > mx6.f;
  a2 = x.f > mx2.f;
  a3 = x.f > 10e-16;

  int addr = a3 * 8 + a2 * 4 + a1 * 2 + a0;
  assert (addr < DEPTH);

  //------------------------------------------------------------
  // memory data output
  //------------------------------------------------------------
  int32_or_float *c = (int32_or_float *) malloc (sizeof(int32_or_float) * WORDS);

  // WORDS x DEPTH
  for (i = 0; i < WORDS; i++) c[i] = mem[i][addr];

  //-------------------------------------------------------------
  // x's power
  //-------------------------------------------------------------
  x2.f = x.f * x.f;
  x3.f = x.f * x2.f;
  x4.f = x2.f * x2.f;

  res.f = c[0].f + c[1].f * x.f + c[2].f * x2.f + c[3].f * x3.f + c[4].f * x4.f;

  //-------------------------------------------------------------
  // Free
  //-------------------------------------------------------------
  free (c);
  for (i = 0; i < WORDS; i++) free(mem[i]);
  free(mem);

  myprintf("Single log(x) res=%.11f\n", res.f);
  if (log) fprintf(fp, "%08x\n", res.i);
  return res.f;
}

// coeffecient memory read + power 2, 4, 3
double log_d1_cheby(FILE *fp, int32_or_float* ports, int log) {

  int32_or_float mx2, mx4, mx5, mx6, mx8;
  int32_or_float mx9, mx10, mx11, mx12, mx13, mx14;
  int32_or_float x;

  int64_or_double x_d, x2_d, x3_d, x4_d;
  int64_or_double res;
  int a3, a2, a1, a0;
  int i, j;

  //----------------------------------------------------------------
  // A simple memory (depth x words)
  //----------------------------------------------------------------
  double val = 0.01; // initial memory fill value
  int64_or_double **mem;

  mem = (int64_or_double **) malloc (sizeof(int64_or_double *) * WORDS);
  for (j = 0; j < WORDS; j++) {
    mem[j] = (int64_or_double *) malloc (sizeof(int64_or_double) * DEPTH);
    for (i = 0; i < DEPTH; i++) {
      mem[j][i].f = val++;  
      myprintf("double memory : %016llx(%.11f)\n", mem[j][i].i, mem[j][i].f);
    }
  }

  //-------------------------------------------------------------
  // log input x
  //-------------------------------------------------------------
  x.f    = ports[0].f;
  x_d.f  = (double)(ports[0].f);

  //-------------------------------------------------------------
  // select range
  //-------------------------------------------------------------
  // 2-1 MUX
  mx2.f = (x.f > 10e-16) ? 10e-8 : 10e-24;

  // 4-1 MUX
  mx4.f = (x.f > mx2.f)  ? 10e-20 : 10e-28;
  mx5.f = (x.f > mx2.f)  ? 10e-4  : 10e-12;
  mx6.f = (x.f > 10e-16) ? mx5.f  : mx4.f;

  // 8-1 MUX
  mx8.f  = (x.f > mx6.f) ? 10e-26 : 10e-30;
  mx9.f  = (x.f > mx6.f) ? 10e-18 : 10e-22;
  mx10.f = (x.f > mx6.f) ? 10e-10 : 10e-14; 
  mx11.f = (x.f > mx6.f) ? 10e-2  : 10e-6; 

  mx12.f = (x.f > mx2.f) ? mx9.f  : mx8.f;
  mx13.f = (x.f > mx2.f) ? mx11.f : mx10.f;

  mx14.f = (x.f > 10e-16) ? mx13.f : mx12.f;

  //------------------------------------------------------------
  // memory address
  //------------------------------------------------------------
  a0 = x.f > mx14.f;
  a1 = x.f > mx6.f;
  a2 = x.f > mx2.f;
  a3 = x.f > 10e-16;

  int addr = a3 * 8 + a2 * 4 + a1 * 2 + a0;
  assert (addr < DEPTH);

  //------------------------------------------------------------
  // memory data output
  //------------------------------------------------------------
  int64_or_double *c = (int64_or_double *) malloc (sizeof(int64_or_double) * WORDS);

  // WORDS x DEPTH
  for (i = 0; i < WORDS; i++) c[i] = mem[i][addr];

  //-------------------------------------------------------------
  // x's power
  //-------------------------------------------------------------
  x2_d.f =  x_d.f *  x_d.f;
  x3_d.f =  x_d.f * x2_d.f;
  x4_d.f = x2_d.f * x2_d.f;

  res.f = c[0].f + c[1].f * x_d.f + c[2].f * x2_d.f + c[3].f * x3_d.f + c[4].f * x4_d.f;
  /*
  myprintf("c0 =%.11f\n", c[0].f);
  myprintf("c1 =%.11f\n", c[1].f);
  myprintf("c2 =%.11f\n", c[2].f);
  myprintf("c3 =%.11f\n", c[3].f);
  myprintf("c4 =%.11f\n", c[4].f);
  */

  //-------------------------------------------------------------
  // Free
  //-------------------------------------------------------------
  free (c);
  for (i = 0; i < WORDS; i++) free(mem[i]);
  free(mem);

  //myprintf("Double log(x) res=%.11f\n", res.f);
  if (log) fprintf(fp, "%016llx\n", res.i);
  return res.f;
}


float log_d_cheby(FILE *fp, int64_or_double* ports, int log) {
  int64_or_double res;
  int64_or_double x, x2, x3, x4;
  int64_or_double mx2, mx4, mx5, mx6, mx8;
  int64_or_double mx9, mx10, mx11, mx12, mx13, mx14;
  int a3, a2, a1, a0;
  int i, j;

  //----------------------------------------------------------------
  // A simple memory (depth x words)
  //----------------------------------------------------------------
  double val = 0.01; // initial memory fill value
  int64_or_double **mem;

  mem = (int64_or_double **) malloc (sizeof(int64_or_double *) * WORDS);
  for (j = 0; j < WORDS; j++) {
    mem[j] = (int64_or_double *) malloc (sizeof(int64_or_double) * DEPTH);
    for (i = 0; i < DEPTH; i++) {
      mem[j][i].f = val++;  
      myprintf("%08x ", mem[j][i].i);
    }
    myprintf("\n");
  }

  //-------------------------------------------------------------
  // log input x
  //-------------------------------------------------------------
  x.f  = ports[0].f;

  //-------------------------------------------------------------
  // select range
  //-------------------------------------------------------------
  // 2-1 MUX
  mx2.f = (x.f > 10e-16) ? 10e-8 : 10e-24;

  // 4-1 MUX
  mx4.f = (x.f > mx2.f)  ? 10e-20 : 10e-28;
  mx5.f = (x.f > mx2.f)  ? 10e-4  : 10e-12;
  mx6.f = (x.f > 10e-16) ? mx5.f  : mx4.f;

  // 8-1 MUX
  mx8.f  = (x.f > mx6.f) ? 10e-26 : 10e-30;
  mx9.f  = (x.f > mx6.f) ? 10e-18 : 10e-22;
  mx10.f = (x.f > mx6.f) ? 10e-10 : 10e-14; 
  mx11.f = (x.f > mx6.f) ? 10e-2  : 10e-6; 

  mx12.f = (x.f > mx2.f) ? mx9.f  : mx8.f;
  mx13.f = (x.f > mx2.f) ? mx11.f : mx10.f;

  mx14.f = (x.f > 10e-16) ? mx13.f : mx12.f;

  //------------------------------------------------------------
  // memory address
  //------------------------------------------------------------
  a0 = x.f > mx14.f;
  a1 = x.f > mx6.f;
  a2 = x.f > mx2.f;
  a3 = x.f > 10e-16;

  int addr = a3 * 8 + a2 * 4 + a1 * 2 + a0;
  assert (addr < DEPTH);

  //------------------------------------------------------------
  // memory data output
  //------------------------------------------------------------
  int64_or_double *c = (int64_or_double *) malloc (sizeof(int64_or_double) * WORDS);

  // WORDS x DEPTH
  for (i = 0; i < WORDS; i++) c[i] = mem[i][addr];

  //-------------------------------------------------------------
  // x's power
  //-------------------------------------------------------------
  x2.f = x.f * x.f;
  x3.f = x.f * x2.f;
  x4.f = x2.f * x2.f;

  res.f = c[0].f + c[1].f * x.f + c[2].f * x2.f + c[3].f * x3.f + c[4].f * x4.f;

  //-------------------------------------------------------------
  // Free
  //-------------------------------------------------------------
  free (c);
  for (i = 0; i < WORDS; i++) free(mem[i]);
  free(mem);

  //myprintf("Double log(x) res=%.11f\n", res.f);
  if (log) fprintf(fp, "%016llx\n", res.i);
  return res.f;
}


float power(FILE *fp, int32_or_float* ports, int maxPortPair) {
  int32_or_float res, x, x2, x3, x4;

  x.f  = ports[0].f;
  x2.f = x.f * x.f;
  x3.f = x.f * x2.f;
  x4.f = x2.f * x2.f;

  res.f = x.f + 2 * x2.f + 3 * x3.f + 4 * x4.f + 5.0;

  myprintf("Single power res=%.11f\n", res.f);
  fprintf(fp, "%08x(%.6f)\n", res.i, res.f);
  return res.i;
}


/*
float tmax(FILE *fp, int32_or_float* ports, int maxPortPair) {
  int32_or_float res, s1, s2, s3, s4, m1, m2;

  s1.f  = ports[0].f + ports[1].f;
  s2.f  = ports[2].f + ports[3].f;
  s3.f  = ports[4].f + ports[5].f;
  s4.f  = ports[6].f + ports[7].f;

  m1.f = s1.f > s2.f ? s1.f : s2.f;
  m2.f = s3.f > s4.f ? s3.f : s4.f;

  res.f = m1.f > m2.f ? m1.f : m2.f;

  myprintf("Single tmax res=%.11f\n", res.f);
  fprintf(fp, "%08x(%.6f)\n", res.i, res.f);
  return res.i;
}
*/

float tmax(FILE *fp, int32_or_float* ports, int maxPortPair) {
  int32_or_float res, s1, s2, s3, s4, m1, m2;

  s1.f  = ports[0].f + ports[1].f;
  s2.f  = ports[2].f * ports[3].f;
  s3.f  = ports[4].f / ports[5].f;
  s4.f  = ports[6].f - ports[7].f;

  m1.f = s1.f > s2.f ? s1.f : s2.f;
  m2.f = s3.f > s4.f ? s3.f : s4.f;

  res.i = (m1.f > m2.f) ? 1 : 0;

  myprintf("Single tmax res=%d\n", res.i);
  fprintf(fp, "fadd1 = %08x(%.6f)\n", s1.i, s1.f);
  fprintf(fp, "fmul1 = %08x(%.6f)\n", s2.i, s2.f);
  fprintf(fp, "fdiv1 = %08x(%.6f)\n", s3.i, s3.f);
  fprintf(fp, "fsub1 = %08x(%.6f)\n", s4.i, s4.f);
  fprintf(fp, "fmax1 = %08x(%.6f)\n", m1.i, m1.f);
  fprintf(fp, "fgt1 = %d\n", res.i); 
  return res.i;
}

float mux2(FILE *fp, int32_or_float* ports, int maxPortPair) {
  int32_or_float res, a, b, c, s;

  a.f = ports[0].f;
  b.f = ports[1].f;
  c.f = ports[2].f;
  s.f = ports[3].i & 1;

  res.f =  s.f ? (a.f * b.f + c.f) : ((a.f + b.f) * c.f);

  myprintf("Single mux2 res=%.11f\n", res.f);
  fprintf(fp, "%08x(%.6f)\n", res.i, res.f);
  return res.f;
}

/*
float mux4(FILE *fp, int32_or_float* ports, int maxPortPair) {
  int32_or_float res, x, x1, x2, x3, x4;

  x.f  = ports[0].f;
  if (x.f < 1e-2)
    x2.f = 1e-4;
  else
    x2.f = 1e-3;

  if (x.f < x2.f) {
    x3.f = 1e-1;
    x4.f = 1e-4;
  } else {
    x3.f = 1e-2;
    x4.f = 1e-3;
  }

  if (x.f < 1e-2)
    x1.f = x4.f;
  else
    x1.f = x3.f;

  res.f = x.f + x1.f;

  myprintf("Single mux4 res=%.11f\n", res.f);
  fprintf(fp, "%08x\n", res.i);
  return res.i;
}
*/

// another mux4 test
float mux4(FILE *fp, int32_or_float* ports, int maxPortPair) {
  int32_or_float res, sum, x, x1, x2, x3, x4, x5;

  x.f  = ports[0].f;
  if (x.f < 1e-2)
    x1.f = 1e-4;
  else
    x1.f = 1e-3;

  myprintf("x1=%.6f\n", x1.f);

  if (x.f < x1.f) {
    x2.f = 1e-1;
    x3.f = 1e-4;
  } else {
    x2.f = 1e-2;
    x3.f = 1e-3;
  }

  myprintf("x2=%.6f\n", x2.f);
  myprintf("x3=%.6f\n", x3.f);

  if (x.f < 1e-2)
    x4.f = x3.f;
  else
    x4.f = x2.f;

  myprintf("x4=%.6f\n", x4.f);

  sum.f = x.f + x4.f;
  myprintf("sum=%.6f\n", sum.f);

  if (x3.f < x1.f)
    x5.f = 1e-2;
  else
    x5.f = x4.f;

  myprintf("x5=%.6f\n", x5.f);

  if (x2.f < x3.f)
    res.f = x5.f;
  else
    res.f = sum.f;

  myprintf("Single mux4 res=%.11f\n", res.f);
  fprintf(fp, "%08x\n", res.i);
  return res.i;
}

double f2d (FILE *fp, int32_or_float* ports, int maxPortPair) {
  int32_or_float tmp, slog, dlog, inS, numS;
  int64_or_double res;

  slog.f = ports[0].f;
  inS.f  = ports[1].f;
  dlog.f = ports[2].f;
  numS.f = ports[3].f;

  tmp.f = slog.f + inS.f;
  res.f = ((double)tmp.f + (double)dlog.f) * (double)numS.f;

  myprintf("Double f2d res=%.11f\n", res.f);
  fprintf(fp, "%016llx\n", res.i);
  return res.i;
}

/* original
double mrbay (FILE *fp, int32_or_float* ports, int maxPortPair) {

  int32_or_float resA, resC, resG, resT;
  int32_or_float IIA, IIC, IIG, IIT;
  int32_or_float max1, max2, max3;
  int32_or_float Norm, InScaler, NumSites;
  int32_or_float slog;
  int64_or_double xd, dlog, res;

  resA.f = resC.f = resG.f = resT.f = 0;

  const int N = 16; // number of inputs per pos

  resA.f = pos(stdout, ports+N*0, N);
  myprintf("LA PLF = %08x(%.6f)\n", resA.i, resA.f);

  resC.f = pos(stdout, ports+N*1, N);
  myprintf("LC PLF = %08x(%.6f)\n", resC.i, resC.f);

  resG.f = pos(stdout, ports+N*2, N);
  myprintf("LG PLF = %08x(%.6f)\n", resG.i, resG.f);

  resT.f = pos(stdout, ports+N*3, N);
  myprintf("LT PLF = %08x(%.6f)\n", resT.i, resT.f);

  max1.f = resA.f > resC.f ? resA.f : resC.f;
  max2.f = resG.f > resT.f ? resG.f : resT.f;
  max3.f = max1.f > max2.f ? max1.f : max2.f;

  IIA.f = ports[N*4+0].f;
  IIC.f = ports[N*4+1].f;
  IIG.f = ports[N*4+2].f;
  IIT.f = ports[N*4+3].f;

  // Suppose circuit input port (width > 1) 
  // connected to mux sel (width = 1) (see also in GenerateDebugData.c)
  // Use bit zero
  Norm.f     = ports[N*4+4].i & 1; 

  InScaler.f = ports[N*4+5].f;
  NumSites.f = ports[N*4+6].f;

  assert(N*4+6 == MAX_PORT_NU - 1);

  resA.f = Norm.f ? resA.f / max3.f : resA.f;
  resC.f = Norm.f ? resC.f / max3.f : resC.f;
  resG.f = Norm.f ? resG.f / max3.f : resG.f;
  resT.f = Norm.f ? resT.f / max3.f : resT.f;

  xd.f = (double) resA.f * (double) IIA.f + (double) resG.f * (double) IIG.f + 
         (double) resC.f * (double) IIC.f + (double) resT.f * (double) IIT.f;

  myprintf("slog input = %08x(%.6f)\n", max3.i, max3.f);
  myprintf("dlog input = %016llx(%.11f)\n", xd.i, xd.f);
  
  slog.f = log_cheby(fp, &max3, 0);
  myprintf("slog output = %08x(%.6f)\n", slog.i, slog.f);

  dlog.f = log_d_cheby(fp, &xd, 0);
  myprintf("dlog output = %016llx(%.11f)\n", dlog.i, dlog.f);

  res.f = ((double)(slog.f + InScaler.f) + dlog.f) * (double)NumSites.f;
  fprintf(fp, "%016llx(%.11f)\n", res.i, res.f);
  return res.f;
}
*/

float dct (FILE *fp, int32_or_float* ports, int maxPortPair) {
 float sample00_o,
 sample01_o,
 sample02_o,
 sample03_o,
 sample04_o,
 sample05_o,
 sample06_o,
 sample07_o,
 sample10_o,
 sample11_o,
 sample12_o,
 sample13_o,
 sample14_o,
 sample15_o,
 sample16_o,
 sample17_o,
 sample20_o,
 sample21_o,
 sample22_o,
 sample23_o,
 sample24_o,
 sample25_o,
 sample26_o,
 sample27_o,
 sample30_o,
 sample31_o,
 sample32_o,
 sample33_o,
 sample34_o,
 sample35_o,
 sample36_o,
 sample37_o,
 sample40_o,
 sample41_o,
 sample42_o,
 sample43_o,
 sample44_o,
 sample45_o,
 sample46_o,
 sample47_o,
 sample50_o,
 sample51_o,
 sample52_o,
 sample53_o,
 sample54_o,
 sample55_o,
 sample56_o,
 sample57_o,
 sample60_o,
 sample61_o,
 sample62_o,
 sample63_o,
 sample64_o,
 sample65_o,
 sample66_o,
 sample67_o,
 sample70_o,
 sample71_o,
 sample72_o,
 sample73_o,
 sample74_o,
 sample75_o,
 sample76_o,
 sample77_o;

float r_tmp0_0, c_tmp0_0;
float r_z0_0, c_z0_0;
float r_tmp0_1, c_tmp0_1;
float r_z0_1, c_z0_1;
float r_tmp0_2, c_tmp0_2;
float r_z0_2, c_z0_2;
float r_tmp0_3, c_tmp0_3;
float r_z0_3, c_z0_3;
float r_tmp0_4, c_tmp0_4;
float r_z0_4, c_z0_4;
float r_tmp0_5, c_tmp0_5;
float r_z0_5, c_z0_5;
float r_tmp0_6, c_tmp0_6;
float r_z0_6, c_z0_6;
float r_tmp0_7, c_tmp0_7;
float r_z0_7, c_z0_7;
float r_tmp1_0, c_tmp1_0;
float r_z1_0, c_z1_0;
float r_tmp1_1, c_tmp1_1;
float r_z1_1, c_z1_1;
float r_tmp1_2, c_tmp1_2;
float r_z1_2, c_z1_2;
float r_tmp1_3, c_tmp1_3;
float r_z1_3, c_z1_3;
float r_tmp1_4, c_tmp1_4;
float r_z1_4, c_z1_4;
float r_tmp1_5, c_tmp1_5;
float r_z1_5, c_z1_5;
float r_tmp1_6, c_tmp1_6;
float r_z1_6, c_z1_6;
float r_tmp1_7, c_tmp1_7;
float r_z1_7, c_z1_7;
float r_tmp2_0, c_tmp2_0;
float r_z2_0, c_z2_0;
float r_tmp2_1, c_tmp2_1;
float r_z2_1, c_z2_1;
float r_tmp2_2, c_tmp2_2;
float r_z2_2, c_z2_2;
float r_tmp2_3, c_tmp2_3;
float r_z2_3, c_z2_3;
float r_tmp2_4, c_tmp2_4;
float r_z2_4, c_z2_4;
float r_tmp2_5, c_tmp2_5;
float r_z2_5, c_z2_5;
float r_tmp2_6, c_tmp2_6;
float r_z2_6, c_z2_6;
float r_tmp2_7, c_tmp2_7;
float r_z2_7, c_z2_7;
float r_tmp3_0, c_tmp3_0;
float r_z3_0, c_z3_0;
float r_tmp3_1, c_tmp3_1;
float r_z3_1, c_z3_1;
float r_tmp3_2, c_tmp3_2;
float r_z3_2, c_z3_2;
float r_tmp3_3, c_tmp3_3;
float r_z3_3, c_z3_3;
float r_tmp3_4, c_tmp3_4;
float r_z3_4, c_z3_4;
float r_tmp3_5, c_tmp3_5;
float r_z3_5, c_z3_5;
float r_tmp3_6, c_tmp3_6;
float r_z3_6, c_z3_6;
float r_tmp3_7, c_tmp3_7;
float r_z3_7, c_z3_7;
float r_tmp4_0, c_tmp4_0;
float r_z4_0, c_z4_0;
float r_tmp4_1, c_tmp4_1;
float r_z4_1, c_z4_1;
float r_tmp4_2, c_tmp4_2;
float r_z4_2, c_z4_2;
float r_tmp4_3, c_tmp4_3;
float r_z4_3, c_z4_3;
float r_tmp4_4, c_tmp4_4;
float r_z4_4, c_z4_4;
float r_tmp4_5, c_tmp4_5;
float r_z4_5, c_z4_5;
float r_tmp4_6, c_tmp4_6;
float r_z4_6, c_z4_6;
float r_tmp4_7, c_tmp4_7;
float r_z4_7, c_z4_7;
float r_tmp5_0, c_tmp5_0;
float r_z5_0, c_z5_0;
float r_tmp5_1, c_tmp5_1;
float r_z5_1, c_z5_1;
float r_tmp5_2, c_tmp5_2;
float r_z5_2, c_z5_2;
float r_tmp5_3, c_tmp5_3;
float r_z5_3, c_z5_3;
float r_tmp5_4, c_tmp5_4;
float r_z5_4, c_z5_4;
float r_tmp5_5, c_tmp5_5;
float r_z5_5, c_z5_5;
float r_tmp5_6, c_tmp5_6;
float r_z5_6, c_z5_6;
float r_tmp5_7, c_tmp5_7;
float r_z5_7, c_z5_7;
float r_tmp6_0, c_tmp6_0;
float r_z6_0, c_z6_0;
float r_tmp6_1, c_tmp6_1;
float r_z6_1, c_z6_1;
float r_tmp6_2, c_tmp6_2;
float r_z6_2, c_z6_2;
float r_tmp6_3, c_tmp6_3;
float r_z6_3, c_z6_3;
float r_tmp6_4, c_tmp6_4;
float r_z6_4, c_z6_4;
float r_tmp6_5, c_tmp6_5;
float r_z6_5, c_z6_5;
float r_tmp6_6, c_tmp6_6;
float r_z6_6, c_z6_6;
float r_tmp6_7, c_tmp6_7;
float r_z6_7, c_z6_7;
float r_tmp7_0, c_tmp7_0;
float r_z7_0, c_z7_0;
float r_tmp7_1, c_tmp7_1;
float r_z7_1, c_z7_1;
float r_tmp7_2, c_tmp7_2;
float r_z7_2, c_z7_2;
float r_tmp7_3, c_tmp7_3;
float r_z7_3, c_z7_3;
float r_tmp7_4, c_tmp7_4;
float r_z7_4, c_z7_4;
float r_tmp7_5, c_tmp7_5;
float r_z7_5, c_z7_5;
float r_tmp7_6, c_tmp7_6;
float r_z7_6, c_z7_6;
float r_tmp7_7, c_tmp7_7;
float r_z7_7, c_z7_7;
float r_tmp8_0, c_tmp8_0;
float r_z8_0, c_z8_0;
float r_tmp8_1, c_tmp8_1;
float r_z8_1, c_z8_1;
float r_tmp8_2, c_tmp8_2;
float r_z8_2, c_z8_2;
float r_tmp8_3, c_tmp8_3;
float r_z8_3, c_z8_3;
float r_tmp8_4, c_tmp8_4;
float r_z8_4, c_z8_4;
float r_tmp8_5, c_tmp8_5;
float r_z8_5, c_z8_5;
float r_tmp8_6, c_tmp8_6;
float r_z8_6, c_z8_6;
float r_tmp8_7, c_tmp8_7;
float r_z8_7, c_z8_7;
float r_tmp9_0, c_tmp9_0;
float r_z9_0, c_z9_0;
float r_tmp9_1, c_tmp9_1;
float r_z9_1, c_z9_1;
float r_tmp9_2, c_tmp9_2;
float r_z9_2, c_z9_2;
float r_tmp9_3, c_tmp9_3;
float r_z9_3, c_z9_3;
float r_tmp9_4, c_tmp9_4;
float r_z9_4, c_z9_4;
float r_tmp9_5, c_tmp9_5;
float r_z9_5, c_z9_5;
float r_tmp9_6, c_tmp9_6;
float r_z9_6, c_z9_6;
float r_tmp9_7, c_tmp9_7;
float r_z9_7, c_z9_7;
float r_tmp10_0, c_tmp10_0;
float r_z10_0, c_z10_0;
float r_tmp10_1, c_tmp10_1;
float r_z10_1, c_z10_1;
float r_tmp10_2, c_tmp10_2;
float r_z10_2, c_z10_2;
float r_tmp10_3, c_tmp10_3;
float r_z10_3, c_z10_3;
float r_tmp10_4, c_tmp10_4;
float r_z10_4, c_z10_4;
float r_tmp10_5, c_tmp10_5;
float r_z10_5, c_z10_5;
float r_tmp10_6, c_tmp10_6;
float r_z10_6, c_z10_6;
float r_tmp10_7, c_tmp10_7;
float r_z10_7, c_z10_7;
float r_tmp11_0, c_tmp11_0;
float r_z11_0, c_z11_0;
float r_tmp11_1, c_tmp11_1;
float r_z11_1, c_z11_1;
float r_tmp11_2, c_tmp11_2;
float r_z11_2, c_z11_2;
float r_tmp11_3, c_tmp11_3;
float r_z11_3, c_z11_3;
float r_tmp11_4, c_tmp11_4;
float r_z11_4, c_z11_4;
float r_tmp11_5, c_tmp11_5;
float r_z11_5, c_z11_5;
float r_tmp11_6, c_tmp11_6;
float r_z11_6, c_z11_6;
float r_tmp11_7, c_tmp11_7;
float r_z11_7, c_z11_7;
float r_tmp12_0, c_tmp12_0;
float r_z12_0, c_z12_0;
float r_tmp12_1, c_tmp12_1;
float r_z12_1, c_z12_1;
float r_tmp12_2, c_tmp12_2;
float r_z12_2, c_z12_2;
float r_tmp12_3, c_tmp12_3;
float r_z12_3, c_z12_3;
float r_tmp12_4, c_tmp12_4;
float r_z12_4, c_z12_4;
float r_tmp12_5, c_tmp12_5;
float r_z12_5, c_z12_5;
float r_tmp12_6, c_tmp12_6;
float r_z12_6, c_z12_6;
float r_tmp12_7, c_tmp12_7;
float r_z12_7, c_z12_7;
float r_tmp13_0, c_tmp13_0;
float r_z13_0, c_z13_0;
float r_tmp13_1, c_tmp13_1;
float r_z13_1, c_z13_1;
float r_tmp13_2, c_tmp13_2;
float r_z13_2, c_z13_2;
float r_tmp13_3, c_tmp13_3;
float r_z13_3, c_z13_3;
float r_tmp13_4, c_tmp13_4;
float r_z13_4, c_z13_4;
float r_tmp13_5, c_tmp13_5;
float r_z13_5, c_z13_5;
float r_tmp13_6, c_tmp13_6;
float r_z13_6, c_z13_6;
float r_tmp13_7, c_tmp13_7;
float r_z13_7, c_z13_7;

  float
      sample00 = ports[0].f,
      sample01 = ports[1].f,
      sample02 = ports[2].f,
      sample03 = ports[3].f,
      sample04 = ports[4].f,
      sample05 = ports[5].f,
      sample06 = ports[6].f,
      sample07 = ports[7].f,
      sample10 = ports[8].f,
      sample11 = ports[9].f,
       sample12 =ports[10].f,
       sample13 =ports[11].f,
       sample14 =ports[12].f,
       sample15 =ports[13].f,
       sample16 =ports[14].f,
       sample17 =ports[15].f,
       sample20 =ports[16].f,
       sample21 =ports[17].f,
       sample22 =ports[18].f,
       sample23 =ports[19].f,
       sample24 =ports[20].f,
       sample25 =ports[21].f,
       sample26 =ports[22].f,
       sample27 =ports[23].f,
       sample30 =ports[24].f,
       sample31 =ports[25].f,
       sample32 =ports[26].f,
       sample33 =ports[27].f,
       sample34 =ports[28].f,
       sample35 =ports[29].f,
       sample36 =ports[30].f,
       sample37 =ports[31].f,
       sample40 =ports[32].f,
       sample41 =ports[33].f,
       sample42 =ports[34].f,
       sample43 =ports[35].f,
       sample44 =ports[36].f,
       sample45 =ports[37].f,
       sample46 =ports[38].f,
       sample47 =ports[39].f,
       sample50 =ports[40].f,
       sample51 =ports[41].f,
       sample52 =ports[42].f,
       sample53 =ports[43].f,
       sample54 =ports[44].f,
       sample55 =ports[45].f,
       sample56 =ports[46].f,
       sample57 =ports[47].f,
       sample60 =ports[48].f,
       sample61 =ports[49].f,
       sample62 =ports[50].f,
       sample63 =ports[51].f,
       sample64 =ports[52].f,
       sample65 =ports[53].f,
       sample66 =ports[54].f,
       sample67 =ports[55].f,
       sample70 =ports[56].f,
       sample71 =ports[57].f,
       sample72 =ports[58].f,
       sample73 =ports[59].f,
       sample74 =ports[60].f,
       sample75 =ports[61].f,
       sample76 =ports[62].f,
       sample77 =ports[63].f;

      r_tmp0_0 = sample00 + sample07;
      r_tmp7_0 = sample00 - sample07;
      r_tmp1_0 = sample01 + sample06;
      r_tmp6_0 = sample01 - sample06;
      r_tmp2_0 = sample02 + sample05;
      r_tmp5_0 = sample02 - sample05;
      r_tmp3_0 = sample03 + sample04;
      r_tmp4_0 = sample03 - sample04;
      r_tmp10_0 = r_tmp0_0 + r_tmp3_0;
      r_tmp13_0 = r_tmp0_0 - r_tmp3_0;
      r_tmp11_0 = r_tmp1_0 + r_tmp2_0;
      r_tmp12_0 = r_tmp1_0 - r_tmp2_0;
      sample00 = r_tmp10_0 + r_tmp11_0;
      sample04 = r_tmp10_0 - r_tmp11_0;
      r_z1_0 = (r_tmp12_0 + r_tmp13_0) * 0.707106781f;
      sample02 = r_tmp13_0 + r_z1_0;
      sample06 = r_tmp13_0 - r_z1_0;
      r_tmp10_0 = r_tmp4_0 + r_tmp5_0;
      r_tmp11_0 = r_tmp5_0 + r_tmp6_0;
      r_tmp12_0 = r_tmp6_0 + r_tmp7_0;
      r_z5_0 = (r_tmp10_0 - r_tmp12_0) * 0.382683433f;
      r_z2_0 = (r_tmp10_0 * 0.541196100f) + r_z5_0;
      r_z4_0 = (r_tmp12_0 * 1.306562965f) + r_z5_0;
      r_z3_0 = (r_tmp11_0 * 0.707106781f);
      r_z11_0 = r_tmp7_0 + r_z3_0;
      r_z13_0 = r_tmp7_0 - r_z3_0;
      sample05 = r_z13_0 + r_z2_0;
      sample03 = r_z13_0 - r_z2_0;
      sample01 = r_z11_0 + r_z4_0;
      sample07 = r_z11_0 - r_z4_0;
      r_tmp0_1 = sample10 + sample17;
      r_tmp7_1 = sample10 - sample17;
      r_tmp1_1 = sample11 + sample16;
      r_tmp6_1 = sample11 - sample16;
      r_tmp2_1 = sample12 + sample15;
      r_tmp5_1 = sample12 - sample15;
      r_tmp3_1 = sample13 + sample14;
      r_tmp4_1 = sample13 - sample14;
      r_tmp10_1 = r_tmp0_1 + r_tmp3_1;
      r_tmp13_1 = r_tmp0_1 - r_tmp3_1;
      r_tmp11_1 = r_tmp1_1 + r_tmp2_1;
      r_tmp12_1 = r_tmp1_1 - r_tmp2_1;
      sample10 = r_tmp10_1 + r_tmp11_1;
      sample14 = r_tmp10_1 - r_tmp11_1;
      r_z1_1 = (r_tmp12_1 + r_tmp13_1) * 0.707106781f;
      sample12 = r_tmp13_1 + r_z1_1;
      sample16 = r_tmp13_1 - r_z1_1;
      r_tmp10_1 = r_tmp4_1 + r_tmp5_1;
      r_tmp11_1 = r_tmp5_1 + r_tmp6_1;
      r_tmp12_1 = r_tmp6_1 + r_tmp7_1;
      r_z5_1 = (r_tmp10_1 - r_tmp12_1) * 0.382683433f;
      r_z2_1 = (r_tmp10_1 * 0.541196100f) + r_z5_1;
      r_z4_1 = (r_tmp12_1 * 1.306562965f) + r_z5_1;
      r_z3_1 = (r_tmp11_1 * 0.707106781f);
      r_z11_1 = r_tmp7_1 + r_z3_1;
      r_z13_1 = r_tmp7_1 - r_z3_1;
      sample15 = r_z13_1 + r_z2_1;
      sample13 = r_z13_1 - r_z2_1;
      sample11 = r_z11_1 + r_z4_1;
      sample17 = r_z11_1 - r_z4_1;
      r_tmp0_2 = sample20 + sample27;
      r_tmp7_2 = sample20 - sample27;
      r_tmp1_2 = sample21 + sample26;
      r_tmp6_2 = sample21 - sample26;
      r_tmp2_2 = sample22 + sample25;
      r_tmp5_2 = sample22 - sample25;
      r_tmp3_2 = sample23 + sample24;
      r_tmp4_2 = sample23 - sample24;
      r_tmp10_2 = r_tmp0_2 + r_tmp3_2;
      r_tmp13_2 = r_tmp0_2 - r_tmp3_2;
      r_tmp11_2 = r_tmp1_2 + r_tmp2_2;
      r_tmp12_2 = r_tmp1_2 - r_tmp2_2;
      sample20 = r_tmp10_2 + r_tmp11_2;
      sample24 = r_tmp10_2 - r_tmp11_2;
      r_z1_2 = (r_tmp12_2 + r_tmp13_2) * 0.707106781f;
      sample22 = r_tmp13_2 + r_z1_2;
      sample26 = r_tmp13_2 - r_z1_2;
      r_tmp10_2 = r_tmp4_2 + r_tmp5_2;
      r_tmp11_2 = r_tmp5_2 + r_tmp6_2;
      r_tmp12_2 = r_tmp6_2 + r_tmp7_2;
      r_z5_2 = (r_tmp10_2 - r_tmp12_2) * 0.382683433f;
      r_z2_2 = (r_tmp10_2 * 0.541196100f) + r_z5_2;
      r_z4_2 = (r_tmp12_2 * 1.306562965f) + r_z5_2;
      r_z3_2 = (r_tmp11_2 * 0.707106781f);
      r_z11_2 = r_tmp7_2 + r_z3_2;
      r_z13_2 = r_tmp7_2 - r_z3_2;
      sample25 = r_z13_2 + r_z2_2;
      sample23 = r_z13_2 - r_z2_2;
      sample21 = r_z11_2 + r_z4_2;
      sample27 = r_z11_2 - r_z4_2;
      r_tmp0_3 = sample30 + sample37;
      r_tmp7_3 = sample30 - sample37;
      r_tmp1_3 = sample31 + sample36;
      r_tmp6_3 = sample31 - sample36;
      r_tmp2_3 = sample32 + sample35;
      r_tmp5_3 = sample32 - sample35;
      r_tmp3_3 = sample33 + sample34;
      r_tmp4_3 = sample33 - sample34;
      r_tmp10_3 = r_tmp0_3 + r_tmp3_3;
      r_tmp13_3 = r_tmp0_3 - r_tmp3_3;
      r_tmp11_3 = r_tmp1_3 + r_tmp2_3;
      r_tmp12_3 = r_tmp1_3 - r_tmp2_3;
      sample30 = r_tmp10_3 + r_tmp11_3;
      sample34 = r_tmp10_3 - r_tmp11_3;
      r_z1_3 = (r_tmp12_3 + r_tmp13_3) * 0.707106781f;
      sample32 = r_tmp13_3 + r_z1_3;
      sample36 = r_tmp13_3 - r_z1_3;
      r_tmp10_3 = r_tmp4_3 + r_tmp5_3;
      r_tmp11_3 = r_tmp5_3 + r_tmp6_3;
      r_tmp12_3 = r_tmp6_3 + r_tmp7_3;
      r_z5_3 = (r_tmp10_3 - r_tmp12_3) * 0.382683433f;
      r_z2_3 = (r_tmp10_3 * 0.541196100f) + r_z5_3;
      r_z4_3 = (r_tmp12_3 * 1.306562965f) + r_z5_3;
      r_z3_3 = (r_tmp11_3 * 0.707106781f);
      r_z11_3 = r_tmp7_3 + r_z3_3;
      r_z13_3 = r_tmp7_3 - r_z3_3;
      sample35 = r_z13_3 + r_z2_3;
      sample33 = r_z13_3 - r_z2_3;
      sample31 = r_z11_3 + r_z4_3;
      sample37 = r_z11_3 - r_z4_3;
      r_tmp0_4 = sample40 + sample47;
      r_tmp7_4 = sample40 - sample47;
      r_tmp1_4 = sample41 + sample46;
      r_tmp6_4 = sample41 - sample46;
      r_tmp2_4 = sample42 + sample45;
      r_tmp5_4 = sample42 - sample45;
      r_tmp3_4 = sample43 + sample44;
      r_tmp4_4 = sample43 - sample44;
      r_tmp10_4 = r_tmp0_4 + r_tmp3_4;
      r_tmp13_4 = r_tmp0_4 - r_tmp3_4;
      r_tmp11_4 = r_tmp1_4 + r_tmp2_4;
      r_tmp12_4 = r_tmp1_4 - r_tmp2_4;
      sample40 = r_tmp10_4 + r_tmp11_4;
      sample44 = r_tmp10_4 - r_tmp11_4;
      r_z1_4 = (r_tmp12_4 + r_tmp13_4) * 0.707106781f;
      sample42 = r_tmp13_4 + r_z1_4;
      sample46 = r_tmp13_4 - r_z1_4;
      r_tmp10_4 = r_tmp4_4 + r_tmp5_4;
      r_tmp11_4 = r_tmp5_4 + r_tmp6_4;
      r_tmp12_4 = r_tmp6_4 + r_tmp7_4;
      r_z5_4 = (r_tmp10_4 - r_tmp12_4) * 0.382683433f;
      r_z2_4 = (r_tmp10_4 * 0.541196100f) + r_z5_4;
      r_z4_4 = (r_tmp12_4 * 1.306562965f) + r_z5_4;
      r_z3_4 = (r_tmp11_4 * 0.707106781f);
      r_z11_4 = r_tmp7_4 + r_z3_4;
      r_z13_4 = r_tmp7_4 - r_z3_4;
      sample45 = r_z13_4 + r_z2_4;
      sample43 = r_z13_4 - r_z2_4;
      sample41 = r_z11_4 + r_z4_4;
      sample47 = r_z11_4 - r_z4_4;
      r_tmp0_5 = sample50 + sample57;
      r_tmp7_5 = sample50 - sample57;
      r_tmp1_5 = sample51 + sample56;
      r_tmp6_5 = sample51 - sample56;
      r_tmp2_5 = sample52 + sample55;
      r_tmp5_5 = sample52 - sample55;
      r_tmp3_5 = sample53 + sample54;
      r_tmp4_5 = sample53 - sample54;
      r_tmp10_5 = r_tmp0_5 + r_tmp3_5;
      r_tmp13_5 = r_tmp0_5 - r_tmp3_5;
      r_tmp11_5 = r_tmp1_5 + r_tmp2_5;
      r_tmp12_5 = r_tmp1_5 - r_tmp2_5;
      sample50 = r_tmp10_5 + r_tmp11_5;
      sample54 = r_tmp10_5 - r_tmp11_5;
      r_z1_5 = (r_tmp12_5 + r_tmp13_5) * 0.707106781f;
      sample52 = r_tmp13_5 + r_z1_5;
      sample56 = r_tmp13_5 - r_z1_5;
      r_tmp10_5 = r_tmp4_5 + r_tmp5_5;
      r_tmp11_5 = r_tmp5_5 + r_tmp6_5;
      r_tmp12_5 = r_tmp6_5 + r_tmp7_5;
      r_z5_5 = (r_tmp10_5 - r_tmp12_5) * 0.382683433f;
      r_z2_5 = (r_tmp10_5 * 0.541196100f) + r_z5_5;
      r_z4_5 = (r_tmp12_5 * 1.306562965f) + r_z5_5;
      r_z3_5 = (r_tmp11_5 * 0.707106781f);
      r_z11_5 = r_tmp7_5 + r_z3_5;
      r_z13_5 = r_tmp7_5 - r_z3_5;
      sample55 = r_z13_5 + r_z2_5;
      sample53 = r_z13_5 - r_z2_5;
      sample51 = r_z11_5 + r_z4_5;
      sample57 = r_z11_5 - r_z4_5;
      r_tmp0_6 = sample60 + sample67;
      r_tmp7_6 = sample60 - sample67;
      r_tmp1_6 = sample61 + sample66;
      r_tmp6_6 = sample61 - sample66;
      r_tmp2_6 = sample62 + sample65;
      r_tmp5_6 = sample62 - sample65;
      r_tmp3_6 = sample63 + sample64;
      r_tmp4_6 = sample63 - sample64;
      r_tmp10_6 = r_tmp0_6 + r_tmp3_6;
      r_tmp13_6 = r_tmp0_6 - r_tmp3_6;
      r_tmp11_6 = r_tmp1_6 + r_tmp2_6;
      r_tmp12_6 = r_tmp1_6 - r_tmp2_6;
      sample60 = r_tmp10_6 + r_tmp11_6;
      sample64 = r_tmp10_6 - r_tmp11_6;
      r_z1_6 = (r_tmp12_6 + r_tmp13_6) * 0.707106781f;
      sample62 = r_tmp13_6 + r_z1_6;
      sample66 = r_tmp13_6 - r_z1_6;
      r_tmp10_6 = r_tmp4_6 + r_tmp5_6;
      r_tmp11_6 = r_tmp5_6 + r_tmp6_6;
      r_tmp12_6 = r_tmp6_6 + r_tmp7_6;
      r_z5_6 = (r_tmp10_6 - r_tmp12_6) * 0.382683433f;
      r_z2_6 = (r_tmp10_6 * 0.541196100f) + r_z5_6;
      r_z4_6 = (r_tmp12_6 * 1.306562965f) + r_z5_6;
      r_z3_6 = (r_tmp11_6 * 0.707106781f);
      r_z11_6 = r_tmp7_6 + r_z3_6;
      r_z13_6 = r_tmp7_6 - r_z3_6;
      sample65 = r_z13_6 + r_z2_6;
      sample63 = r_z13_6 - r_z2_6;
      sample61 = r_z11_6 + r_z4_6;
      sample67 = r_z11_6 - r_z4_6;
      r_tmp0_7 = sample70 + sample77;
      r_tmp7_7 = sample70 - sample77;
      r_tmp1_7 = sample71 + sample76;
      r_tmp6_7 = sample71 - sample76;
      r_tmp2_7 = sample72 + sample75;
      r_tmp5_7 = sample72 - sample75;
      r_tmp3_7 = sample73 + sample74;
      r_tmp4_7 = sample73 - sample74;
      r_tmp10_7 = r_tmp0_7 + r_tmp3_7;
      r_tmp13_7 = r_tmp0_7 - r_tmp3_7;
      r_tmp11_7 = r_tmp1_7 + r_tmp2_7;
      r_tmp12_7 = r_tmp1_7 - r_tmp2_7;
      sample70 = r_tmp10_7 + r_tmp11_7;
      sample74 = r_tmp10_7 - r_tmp11_7;
      r_z1_7 = (r_tmp12_7 + r_tmp13_7) * 0.707106781f;
      sample72 = r_tmp13_7 + r_z1_7;
      sample76 = r_tmp13_7 - r_z1_7;
      r_tmp10_7 = r_tmp4_7 + r_tmp5_7;
      r_tmp11_7 = r_tmp5_7 + r_tmp6_7;
      r_tmp12_7 = r_tmp6_7 + r_tmp7_7;
      r_z5_7 = (r_tmp10_7 - r_tmp12_7) * 0.382683433f;
      r_z2_7 = (r_tmp10_7 * 0.541196100f) + r_z5_7;
      r_z4_7 = (r_tmp12_7 * 1.306562965f) + r_z5_7;
      r_z3_7 = (r_tmp11_7 * 0.707106781f);
      r_z11_7 = r_tmp7_7 + r_z3_7;
      r_z13_7 = r_tmp7_7 - r_z3_7;
      sample75 = r_z13_7 + r_z2_7;
      sample73 = r_z13_7 - r_z2_7;
      sample71 = r_z11_7 + r_z4_7;
      sample77 = r_z11_7 - r_z4_7;


      c_tmp0_0 = sample00 + sample70;
      c_tmp7_0 = sample00 - sample70;
      c_tmp1_0 = sample10 + sample60;
      c_tmp6_0 = sample10 - sample60;
      c_tmp2_0 = sample20 + sample50;
      c_tmp5_0 = sample20 - sample50;
      c_tmp3_0 = sample30 + sample40;
      c_tmp4_0 = sample30 - sample40;
      c_tmp10_0 = c_tmp0_0 + c_tmp3_0;
      c_tmp13_0 = c_tmp0_0 - c_tmp3_0;
      c_tmp11_0 = c_tmp1_0 + c_tmp2_0;
      c_tmp12_0 = c_tmp1_0 - c_tmp2_0;
      sample00_o = c_tmp10_0 + c_tmp11_0;
      sample40_o = c_tmp10_0 - c_tmp11_0;
      c_z1_0 = (c_tmp12_0 + c_tmp13_0) * 0.707106781f;
      sample20_o = c_tmp13_0 + c_z1_0;
      sample60_o = c_tmp13_0 - c_z1_0;
      c_tmp10_0 = c_tmp4_0 + c_tmp5_0;
      c_tmp11_0 = c_tmp5_0 + c_tmp6_0;
      c_tmp12_0 = c_tmp6_0 + c_tmp7_0;
      c_z5_0 = (c_tmp10_0 - c_tmp12_0) * 0.382683433f;
      c_z2_0 = (c_tmp10_0 * 0.541196100f) + c_z5_0;
      c_z4_0 = (c_tmp12_0 * 1.306562965f) + c_z5_0;
      c_z3_0 = (c_tmp11_0 * 0.707106781f);
      c_z11_0 = c_tmp7_0 + c_z3_0;
      c_z13_0 = c_tmp7_0 - c_z3_0;
      sample50_o = c_z13_0 + c_z2_0;
      sample30_o = c_z13_0 - c_z2_0;
      sample10_o = c_z11_0 + c_z4_0;
      sample70_o = c_z11_0 - c_z4_0;
      c_tmp0_1 = sample01 + sample71;
      c_tmp7_1 = sample01 - sample71;
      c_tmp1_1 = sample11 + sample61;
      c_tmp6_1 = sample11 - sample61;
      c_tmp2_1 = sample21 + sample51;
      c_tmp5_1 = sample21 - sample51;
      c_tmp3_1 = sample31 + sample41;
      c_tmp4_1 = sample31 - sample41;
      c_tmp10_1 = c_tmp0_1 + c_tmp3_1;
      c_tmp13_1 = c_tmp0_1 - c_tmp3_1;
      c_tmp11_1 = c_tmp1_1 + c_tmp2_1;
      c_tmp12_1 = c_tmp1_1 - c_tmp2_1;
      sample01_o = c_tmp10_1 + c_tmp11_1;
      sample41_o = c_tmp10_1 - c_tmp11_1;
      c_z1_1 = (c_tmp12_1 + c_tmp13_1) * 0.707106781f;
      sample21_o = c_tmp13_1 + c_z1_1;
      sample61_o = c_tmp13_1 - c_z1_1;
      c_tmp10_1 = c_tmp4_1 + c_tmp5_1;
      c_tmp11_1 = c_tmp5_1 + c_tmp6_1;
      c_tmp12_1 = c_tmp6_1 + c_tmp7_1;
      c_z5_1 = (c_tmp10_1 - c_tmp12_1) * 0.382683433f;
      c_z2_1 = (c_tmp10_1 * 0.541196100f) + c_z5_1;
      c_z4_1 = (c_tmp12_1 * 1.306562965f) + c_z5_1;
      c_z3_1 = (c_tmp11_1 * 0.707106781f);
      c_z11_1 = c_tmp7_1 + c_z3_1;
      c_z13_1 = c_tmp7_1 - c_z3_1;
      sample51_o = c_z13_1 + c_z2_1;
      sample31_o = c_z13_1 - c_z2_1;
      sample11_o = c_z11_1 + c_z4_1;
      sample71_o = c_z11_1 - c_z4_1;
      c_tmp0_2 = sample02 + sample72;
      c_tmp7_2 = sample02 - sample72;
      c_tmp1_2 = sample12 + sample62;
      c_tmp6_2 = sample12 - sample62;
      c_tmp2_2 = sample22 + sample52;
      c_tmp5_2 = sample22 - sample52;
      c_tmp3_2 = sample32 + sample42;
      c_tmp4_2 = sample32 - sample42;
      c_tmp10_2 = c_tmp0_2 + c_tmp3_2;
      c_tmp13_2 = c_tmp0_2 - c_tmp3_2;
      c_tmp11_2 = c_tmp1_2 + c_tmp2_2;
      c_tmp12_2 = c_tmp1_2 - c_tmp2_2;
      sample02_o = c_tmp10_2 + c_tmp11_2;
      sample42_o = c_tmp10_2 - c_tmp11_2;
      c_z1_2 = (c_tmp12_2 + c_tmp13_2) * 0.707106781f;
      sample22_o = c_tmp13_2 + c_z1_2;
      sample62_o = c_tmp13_2 - c_z1_2;
      c_tmp10_2 = c_tmp4_2 + c_tmp5_2;
      c_tmp11_2 = c_tmp5_2 + c_tmp6_2;
      c_tmp12_2 = c_tmp6_2 + c_tmp7_2;
      c_z5_2 = (c_tmp10_2 - c_tmp12_2) * 0.382683433f;
      c_z2_2 = (c_tmp10_2 * 0.541196100f) + c_z5_2;
      c_z4_2 = (c_tmp12_2 * 1.306562965f) + c_z5_2;
      c_z3_2 = (c_tmp11_2 * 0.707106781f);
      c_z11_2 = c_tmp7_2 + c_z3_2;
      c_z13_2 = c_tmp7_2 - c_z3_2;
      sample52_o = c_z13_2 + c_z2_2;
      sample32_o = c_z13_2 - c_z2_2;
      sample12_o = c_z11_2 + c_z4_2;
      sample72_o = c_z11_2 - c_z4_2;
      c_tmp0_3 = sample03 + sample73;
      c_tmp7_3 = sample03 - sample73;
      c_tmp1_3 = sample13 + sample63;
      c_tmp6_3 = sample13 - sample63;
      c_tmp2_3 = sample23 + sample53;
      c_tmp5_3 = sample23 - sample53;
      c_tmp3_3 = sample33 + sample43;
      c_tmp4_3 = sample33 - sample43;
      c_tmp10_3 = c_tmp0_3 + c_tmp3_3;
      c_tmp13_3 = c_tmp0_3 - c_tmp3_3;
      c_tmp11_3 = c_tmp1_3 + c_tmp2_3;
      c_tmp12_3 = c_tmp1_3 - c_tmp2_3;
      sample03_o = c_tmp10_3 + c_tmp11_3;
      sample43_o = c_tmp10_3 - c_tmp11_3;
      c_z1_3 = (c_tmp12_3 + c_tmp13_3) * 0.707106781f;
      sample23_o = c_tmp13_3 + c_z1_3;
      sample63_o = c_tmp13_3 - c_z1_3;
      c_tmp10_3 = c_tmp4_3 + c_tmp5_3;
      c_tmp11_3 = c_tmp5_3 + c_tmp6_3;
      c_tmp12_3 = c_tmp6_3 + c_tmp7_3;
      c_z5_3 = (c_tmp10_3 - c_tmp12_3) * 0.382683433f;
      c_z2_3 = (c_tmp10_3 * 0.541196100f) + c_z5_3;
      c_z4_3 = (c_tmp12_3 * 1.306562965f) + c_z5_3;
      c_z3_3 = (c_tmp11_3 * 0.707106781f);
      c_z11_3 = c_tmp7_3 + c_z3_3;
      c_z13_3 = c_tmp7_3 - c_z3_3;
      sample53_o = c_z13_3 + c_z2_3;
      sample33_o = c_z13_3 - c_z2_3;
      sample13_o = c_z11_3 + c_z4_3;
      sample73_o = c_z11_3 - c_z4_3;
      c_tmp0_4 = sample04 + sample74;
      c_tmp7_4 = sample04 - sample74;
      c_tmp1_4 = sample14 + sample64;
      c_tmp6_4 = sample14 - sample64;
      c_tmp2_4 = sample24 + sample54;
      c_tmp5_4 = sample24 - sample54;
      c_tmp3_4 = sample34 + sample44;
      c_tmp4_4 = sample34 - sample44;
      c_tmp10_4 = c_tmp0_4 + c_tmp3_4;
      c_tmp13_4 = c_tmp0_4 - c_tmp3_4;
      c_tmp11_4 = c_tmp1_4 + c_tmp2_4;
      c_tmp12_4 = c_tmp1_4 - c_tmp2_4;
      sample04_o = c_tmp10_4 + c_tmp11_4;
      sample44_o = c_tmp10_4 - c_tmp11_4;
      c_z1_4 = (c_tmp12_4 + c_tmp13_4) * 0.707106781f;
      sample24_o = c_tmp13_4 + c_z1_4;
      sample64_o = c_tmp13_4 - c_z1_4;
      c_tmp10_4 = c_tmp4_4 + c_tmp5_4;
      c_tmp11_4 = c_tmp5_4 + c_tmp6_4;
      c_tmp12_4 = c_tmp6_4 + c_tmp7_4;
      c_z5_4 = (c_tmp10_4 - c_tmp12_4) * 0.382683433f;
      c_z2_4 = (c_tmp10_4 * 0.541196100f) + c_z5_4;
      c_z4_4 = (c_tmp12_4 * 1.306562965f) + c_z5_4;
      c_z3_4 = (c_tmp11_4 * 0.707106781f);
      c_z11_4 = c_tmp7_4 + c_z3_4;
      c_z13_4 = c_tmp7_4 - c_z3_4;
      sample54_o = c_z13_4 + c_z2_4;
      sample34_o = c_z13_4 - c_z2_4;
      sample14_o = c_z11_4 + c_z4_4;
      sample74_o = c_z11_4 - c_z4_4;
      c_tmp0_5 = sample05 + sample75;
      c_tmp7_5 = sample05 - sample75;
      c_tmp1_5 = sample15 + sample65;
      c_tmp6_5 = sample15 - sample65;
      c_tmp2_5 = sample25 + sample55;
      c_tmp5_5 = sample25 - sample55;
      c_tmp3_5 = sample35 + sample45;
      c_tmp4_5 = sample35 - sample45;
      c_tmp10_5 = c_tmp0_5 + c_tmp3_5;
      c_tmp13_5 = c_tmp0_5 - c_tmp3_5;
      c_tmp11_5 = c_tmp1_5 + c_tmp2_5;
      c_tmp12_5 = c_tmp1_5 - c_tmp2_5;
      sample05_o = c_tmp10_5 + c_tmp11_5;
      sample45_o = c_tmp10_5 - c_tmp11_5;
      c_z1_5 = (c_tmp12_5 + c_tmp13_5) * 0.707106781f;
      sample25_o = c_tmp13_5 + c_z1_5;
      sample65_o = c_tmp13_5 - c_z1_5;
      c_tmp10_5 = c_tmp4_5 + c_tmp5_5;
      c_tmp11_5 = c_tmp5_5 + c_tmp6_5;
      c_tmp12_5 = c_tmp6_5 + c_tmp7_5;
      c_z5_5 = (c_tmp10_5 - c_tmp12_5) * 0.382683433f;
      c_z2_5 = (c_tmp10_5 * 0.541196100f) + c_z5_5;
      c_z4_5 = (c_tmp12_5 * 1.306562965f) + c_z5_5;
      c_z3_5 = (c_tmp11_5 * 0.707106781f);
      c_z11_5 = c_tmp7_5 + c_z3_5;
      c_z13_5 = c_tmp7_5 - c_z3_5;
      sample55_o = c_z13_5 + c_z2_5;
      sample35_o = c_z13_5 - c_z2_5;
      sample15_o = c_z11_5 + c_z4_5;
      sample75_o = c_z11_5 - c_z4_5;
      c_tmp0_6 = sample06 + sample76;
      c_tmp7_6 = sample06 - sample76;
      c_tmp1_6 = sample16 + sample66;
      c_tmp6_6 = sample16 - sample66;
      c_tmp2_6 = sample26 + sample56;
      c_tmp5_6 = sample26 - sample56;
      c_tmp3_6 = sample36 + sample46;
      c_tmp4_6 = sample36 - sample46;
      c_tmp10_6 = c_tmp0_6 + c_tmp3_6;
      c_tmp13_6 = c_tmp0_6 - c_tmp3_6;
      c_tmp11_6 = c_tmp1_6 + c_tmp2_6;
      c_tmp12_6 = c_tmp1_6 - c_tmp2_6;
      sample06_o = c_tmp10_6 + c_tmp11_6;
      sample46_o = c_tmp10_6 - c_tmp11_6;
      c_z1_6 = (c_tmp12_6 + c_tmp13_6) * 0.707106781f;
      sample26_o = c_tmp13_6 + c_z1_6;
      sample66_o = c_tmp13_6 - c_z1_6;
      c_tmp10_6 = c_tmp4_6 + c_tmp5_6;
      c_tmp11_6 = c_tmp5_6 + c_tmp6_6;
      c_tmp12_6 = c_tmp6_6 + c_tmp7_6;
      c_z5_6 = (c_tmp10_6 - c_tmp12_6) * 0.382683433f;
      c_z2_6 = (c_tmp10_6 * 0.541196100f) + c_z5_6;
      c_z4_6 = (c_tmp12_6 * 1.306562965f) + c_z5_6;
      c_z3_6 = (c_tmp11_6 * 0.707106781f);
      c_z11_6 = c_tmp7_6 + c_z3_6;
      c_z13_6 = c_tmp7_6 - c_z3_6;
      sample56_o = c_z13_6 + c_z2_6;
      sample36_o = c_z13_6 - c_z2_6;
      sample16_o = c_z11_6 + c_z4_6;
      sample76_o = c_z11_6 - c_z4_6;
      c_tmp0_7 = sample07 + sample77;
      c_tmp7_7 = sample07 - sample77;
      c_tmp1_7 = sample17 + sample67;
      c_tmp6_7 = sample17 - sample67;
      c_tmp2_7 = sample27 + sample57;
      c_tmp5_7 = sample27 - sample57;
      c_tmp3_7 = sample37 + sample47;
      c_tmp4_7 = sample37 - sample47;
      c_tmp10_7 = c_tmp0_7 + c_tmp3_7;
      c_tmp13_7 = c_tmp0_7 - c_tmp3_7;
      c_tmp11_7 = c_tmp1_7 + c_tmp2_7;
      c_tmp12_7 = c_tmp1_7 - c_tmp2_7;
      sample07_o = c_tmp10_7 + c_tmp11_7;
      sample47_o = c_tmp10_7 - c_tmp11_7;
      c_z1_7 = (c_tmp12_7 + c_tmp13_7) * 0.707106781f;
      sample27_o = c_tmp13_7 + c_z1_7;
      sample67_o = c_tmp13_7 - c_z1_7;
      c_tmp10_7 = c_tmp4_7 + c_tmp5_7;
      c_tmp11_7 = c_tmp5_7 + c_tmp6_7;
      c_tmp12_7 = c_tmp6_7 + c_tmp7_7;
      c_z5_7 = (c_tmp10_7 - c_tmp12_7) * 0.382683433f;
      c_z2_7 = (c_tmp10_7 * 0.541196100f) + c_z5_7;
      c_z4_7 = (c_tmp12_7 * 1.306562965f) + c_z5_7;
      c_z3_7 = (c_tmp11_7 * 0.707106781f);
      c_z11_7 = c_tmp7_7 + c_z3_7;
      c_z13_7 = c_tmp7_7 - c_z3_7;
      sample57_o = c_z13_7 + c_z2_7;
      sample37_o = c_z13_7 - c_z2_7;
      sample17_o = c_z11_7 + c_z4_7;
      sample77_o = c_z11_7 - c_z4_7;

      /*
     fprintf(fp, "r_tmp0_0 = %.6f\n", r_tmp0_0);
     fprintf(fp, "r_tmp0_1 = %.6f\n", r_tmp0_1);
     fprintf(fp, "r_tmp0_2 = %.6f\n", r_tmp0_2);
     fprintf(fp, "r_tmp0_3 = %.6f\n", r_tmp0_3);
     fprintf(fp, "r_tmp0_4 = %.6f\n", r_tmp0_4);
     fprintf(fp, "r_tmp0_5 = %.6f\n", r_tmp0_5);
     fprintf(fp, "r_tmp0_6 = %.6f\n", r_tmp0_6);
     fprintf(fp, "r_tmp0_7 = %.6f\n", r_tmp0_7);
     fprintf(fp, "r_tmp1_0 = %.6f\n", r_tmp1_0);
     fprintf(fp, "r_tmp1_1 = %.6f\n", r_tmp1_1);
     fprintf(fp, "r_tmp1_2 = %.6f\n", r_tmp1_2);
     fprintf(fp, "r_tmp1_3 = %.6f\n", r_tmp1_3);
     fprintf(fp, "r_tmp1_4 = %.6f\n", r_tmp1_4);
     fprintf(fp, "r_tmp1_5 = %.6f\n", r_tmp1_5);
     fprintf(fp, "r_tmp1_6 = %.6f\n", r_tmp1_6);
     fprintf(fp, "r_tmp1_7 = %.6f\n", r_tmp1_7);
     fprintf(fp, "r_tmp2_0 = %.6f\n", r_tmp2_0);
     fprintf(fp, "r_tmp2_1 = %.6f\n", r_tmp2_1);
     fprintf(fp, "r_tmp2_2 = %.6f\n", r_tmp2_2);
     fprintf(fp, "r_tmp2_3 = %.6f\n", r_tmp2_3);
     fprintf(fp, "r_tmp2_4 = %.6f\n", r_tmp2_4);
     fprintf(fp, "r_tmp2_5 = %.6f\n", r_tmp2_5);
     fprintf(fp, "r_tmp2_6 = %.6f\n", r_tmp2_6);
     fprintf(fp, "r_tmp2_7 = %.6f\n", r_tmp2_7);
     fprintf(fp, "r_tmp3_0 = %.6f\n", r_tmp3_0);
     fprintf(fp, "r_tmp3_1 = %.6f\n", r_tmp3_1);
     fprintf(fp, "r_tmp3_2 = %.6f\n", r_tmp3_2);
     fprintf(fp, "r_tmp3_3 = %.6f\n", r_tmp3_3);
     fprintf(fp, "r_tmp3_4 = %.6f\n", r_tmp3_4);
     fprintf(fp, "r_tmp3_5 = %.6f\n", r_tmp3_5);
     fprintf(fp, "r_tmp3_6 = %.6f\n", r_tmp3_6);
     fprintf(fp, "r_tmp3_7 = %.6f\n", r_tmp3_7);
     fprintf(fp, "r_tmp4_0 = %.6f\n", r_tmp4_0);
     fprintf(fp, "r_tmp4_1 = %.6f\n", r_tmp4_1);
     fprintf(fp, "r_tmp4_2 = %.6f\n", r_tmp4_2);
     fprintf(fp, "r_tmp4_3 = %.6f\n", r_tmp4_3);
     fprintf(fp, "r_tmp4_4 = %.6f\n", r_tmp4_4);
     fprintf(fp, "r_tmp4_5 = %.6f\n", r_tmp4_5);
     fprintf(fp, "r_tmp4_6 = %.6f\n", r_tmp4_6);
     fprintf(fp, "r_tmp4_7 = %.6f\n", r_tmp4_7);
     fprintf(fp, "r_tmp5_0 = %.6f\n", r_tmp5_0);
     fprintf(fp, "r_tmp5_1 = %.6f\n", r_tmp5_1);
     fprintf(fp, "r_tmp5_2 = %.6f\n", r_tmp5_2);
     fprintf(fp, "r_tmp5_3 = %.6f\n", r_tmp5_3);
     fprintf(fp, "r_tmp5_4 = %.6f\n", r_tmp5_4);
     fprintf(fp, "r_tmp5_5 = %.6f\n", r_tmp5_5);
     fprintf(fp, "r_tmp5_6 = %.6f\n", r_tmp5_6);
     fprintf(fp, "r_tmp5_7 = %.6f\n", r_tmp5_7);
     fprintf(fp, "r_tmp6_0 = %.6f\n", r_tmp6_0);
     fprintf(fp, "r_tmp6_1 = %.6f\n", r_tmp6_1);
     fprintf(fp, "r_tmp6_2 = %.6f\n", r_tmp6_2);
     fprintf(fp, "r_tmp6_3 = %.6f\n", r_tmp6_3);
     fprintf(fp, "r_tmp6_4 = %.6f\n", r_tmp6_4);
     fprintf(fp, "r_tmp6_5 = %.6f\n", r_tmp6_5);
     fprintf(fp, "r_tmp6_6 = %.6f\n", r_tmp6_6);
     fprintf(fp, "r_tmp6_7 = %.6f\n", r_tmp6_7);
     fprintf(fp, "r_tmp7_0 = %.6f\n", r_tmp7_0);
     fprintf(fp, "r_tmp7_1 = %.6f\n", r_tmp7_1);
     fprintf(fp, "r_tmp7_2 = %.6f\n", r_tmp7_2);
     fprintf(fp, "r_tmp7_3 = %.6f\n", r_tmp7_3);
     fprintf(fp, "r_tmp7_4 = %.6f\n", r_tmp7_4);
     fprintf(fp, "r_tmp7_5 = %.6f\n", r_tmp7_5);
     fprintf(fp, "r_tmp7_6 = %.6f\n", r_tmp7_6);
     fprintf(fp, "r_tmp7_7 = %.6f\n", r_tmp7_7);

     fprintf(fp, "sample00_t = %.6f\n", sample00);
     fprintf(fp, "sample01_t = %.6f\n", sample01);
     fprintf(fp, "sample02_t = %.6f\n", sample02);
     fprintf(fp, "sample03_t = %.6f\n", sample03);
     fprintf(fp, "sample04_t = %.6f\n", sample04);
     fprintf(fp, "sample05_t = %.6f\n", sample05);
     fprintf(fp, "sample06_t = %.6f\n", sample06);
     fprintf(fp, "sample07_t = %.6f\n", sample07);
     fprintf(fp, "sample10_t = %.6f\n", sample10);
     fprintf(fp, "sample11_t = %.6f\n", sample11);
     fprintf(fp, "sample12_t = %.6f\n", sample12);
     fprintf(fp, "sample13_t = %.6f\n", sample13);
     fprintf(fp, "sample14_t = %.6f\n", sample14);
     fprintf(fp, "sample15_t = %.6f\n", sample15);
     fprintf(fp, "sample16_t = %.6f\n", sample16);
     fprintf(fp, "sample17_t = %.6f\n", sample17);
     fprintf(fp, "sample20_t = %.6f\n", sample20);
     fprintf(fp, "sample21_t = %.6f\n", sample21);
     fprintf(fp, "sample22_t = %.6f\n", sample22);
     fprintf(fp, "sample23_t = %.6f\n", sample23);
     fprintf(fp, "sample24_t = %.6f\n", sample24);
     fprintf(fp, "sample25_t = %.6f\n", sample25);
     fprintf(fp, "sample26_t = %.6f\n", sample26);
     fprintf(fp, "sample27_t = %.6f\n", sample27);
     fprintf(fp, "sample30_t = %.6f\n", sample30);
     fprintf(fp, "sample31_t = %.6f\n", sample31);
     fprintf(fp, "sample32_t = %.6f\n", sample32);
     fprintf(fp, "sample33_t = %.6f\n", sample33);
     fprintf(fp, "sample34_t = %.6f\n", sample34);
     fprintf(fp, "sample35_t = %.6f\n", sample35);
     fprintf(fp, "sample36_t = %.6f\n", sample36);
     fprintf(fp, "sample37_t = %.6f\n", sample37);
     fprintf(fp, "sample40_t = %.6f\n", sample40);
     fprintf(fp, "sample41_t = %.6f\n", sample41);
     fprintf(fp, "sample42_t = %.6f\n", sample42);
     fprintf(fp, "sample43_t = %.6f\n", sample43);
     fprintf(fp, "sample44_t = %.6f\n", sample44);
     fprintf(fp, "sample45_t = %.6f\n", sample45);
     fprintf(fp, "sample46_t = %.6f\n", sample46);
     fprintf(fp, "sample47_t = %.6f\n", sample47);
     fprintf(fp, "sample50_t = %.6f\n", sample50);
     fprintf(fp, "sample51_t = %.6f\n", sample51);
     fprintf(fp, "sample52_t = %.6f\n", sample52);
     fprintf(fp, "sample53_t = %.6f\n", sample53);
     fprintf(fp, "sample54_t = %.6f\n", sample54);
     fprintf(fp, "sample55_t = %.6f\n", sample55);
     fprintf(fp, "sample56_t = %.6f\n", sample56);
     fprintf(fp, "sample57_t = %.6f\n", sample57);
     fprintf(fp, "sample60_t = %.6f\n", sample60);
     fprintf(fp, "sample61_t = %.6f\n", sample61);
     fprintf(fp, "sample62_t = %.6f\n", sample62);
     fprintf(fp, "sample63_t = %.6f\n", sample63);
     fprintf(fp, "sample64_t = %.6f\n", sample64);
     fprintf(fp, "sample65_t = %.6f\n", sample65);
     fprintf(fp, "sample66_t = %.6f\n", sample66);
     fprintf(fp, "sample67_t = %.6f\n", sample67);
     fprintf(fp, "sample70_t = %.6f\n", sample70);
     fprintf(fp, "sample71_t = %.6f\n", sample71);
     fprintf(fp, "sample72_t = %.6f\n", sample72);
     fprintf(fp, "sample73_t = %.6f\n", sample73);
     fprintf(fp, "sample74_t = %.6f\n", sample74);
     fprintf(fp, "sample75_t = %.6f\n", sample75);
     fprintf(fp, "sample76_t = %.6f\n", sample76);
     fprintf(fp, "sample77_t = %.6f\n", sample77);
     */

     fprintf(fp, "sample00_o = %.6f\n", sample00_o);
     fprintf(fp, "sample01_o = %.6f\n", sample01_o);
     fprintf(fp, "sample02_o = %.6f\n", sample02_o);
     fprintf(fp, "sample03_o = %.6f\n", sample03_o);
     fprintf(fp, "sample04_o = %.6f\n", sample04_o);
     fprintf(fp, "sample05_o = %.6f\n", sample05_o);
     fprintf(fp, "sample06_o = %.6f\n", sample06_o);
     fprintf(fp, "sample07_o = %.6f\n", sample07_o);
     fprintf(fp, "sample10_o = %.6f\n", sample10_o);
     fprintf(fp, "sample11_o = %.6f\n", sample11_o);
     fprintf(fp, "sample12_o = %.6f\n", sample12_o);
     fprintf(fp, "sample13_o = %.6f\n", sample13_o);
     fprintf(fp, "sample14_o = %.6f\n", sample14_o);
     fprintf(fp, "sample15_o = %.6f\n", sample15_o);
     fprintf(fp, "sample16_o = %.6f\n", sample16_o);
     fprintf(fp, "sample17_o = %.6f\n", sample17_o);
     fprintf(fp, "sample20_o = %.6f\n", sample20_o);
     fprintf(fp, "sample21_o = %.6f\n", sample21_o);
     fprintf(fp, "sample22_o = %.6f\n", sample22_o);
     fprintf(fp, "sample23_o = %.6f\n", sample23_o);
     fprintf(fp, "sample24_o = %.6f\n", sample24_o);
     fprintf(fp, "sample25_o = %.6f\n", sample25_o);
     fprintf(fp, "sample26_o = %.6f\n", sample26_o);
     fprintf(fp, "sample27_o = %.6f\n", sample27_o);
     fprintf(fp, "sample30_o = %.6f\n", sample30_o);
     fprintf(fp, "sample31_o = %.6f\n", sample31_o);
     fprintf(fp, "sample32_o = %.6f\n", sample32_o);
     fprintf(fp, "sample33_o = %.6f\n", sample33_o);
     fprintf(fp, "sample34_o = %.6f\n", sample34_o);
     fprintf(fp, "sample35_o = %.6f\n", sample35_o);
     fprintf(fp, "sample36_o = %.6f\n", sample36_o);
     fprintf(fp, "sample37_o = %.6f\n", sample37_o);
     fprintf(fp, "sample40_o = %.6f\n", sample40_o);
     fprintf(fp, "sample41_o = %.6f\n", sample41_o);
     fprintf(fp, "sample42_o = %.6f\n", sample42_o);
     fprintf(fp, "sample43_o = %.6f\n", sample43_o);
     fprintf(fp, "sample44_o = %.6f\n", sample44_o);
     fprintf(fp, "sample45_o = %.6f\n", sample45_o);
     fprintf(fp, "sample46_o = %.6f\n", sample46_o);
     fprintf(fp, "sample47_o = %.6f\n", sample47_o);
     fprintf(fp, "sample50_o = %.6f\n", sample50_o);
     fprintf(fp, "sample51_o = %.6f\n", sample51_o);
     fprintf(fp, "sample52_o = %.6f\n", sample52_o);
     fprintf(fp, "sample53_o = %.6f\n", sample53_o);
     fprintf(fp, "sample54_o = %.6f\n", sample54_o);
     fprintf(fp, "sample55_o = %.6f\n", sample55_o);
     fprintf(fp, "sample56_o = %.6f\n", sample56_o);
     fprintf(fp, "sample57_o = %.6f\n", sample57_o);
     fprintf(fp, "sample60_o = %.6f\n", sample60_o);
     fprintf(fp, "sample61_o = %.6f\n", sample61_o);
     fprintf(fp, "sample62_o = %.6f\n", sample62_o);
     fprintf(fp, "sample63_o = %.6f\n", sample63_o);
     fprintf(fp, "sample64_o = %.6f\n", sample64_o);
     fprintf(fp, "sample65_o = %.6f\n", sample65_o);
     fprintf(fp, "sample66_o = %.6f\n", sample66_o);
     fprintf(fp, "sample67_o = %.6f\n", sample67_o);
     fprintf(fp, "sample70_o = %.6f\n", sample70_o);
     fprintf(fp, "sample71_o = %.6f\n", sample71_o);
     fprintf(fp, "sample72_o = %.6f\n", sample72_o);
     fprintf(fp, "sample73_o = %.6f\n", sample73_o);
     fprintf(fp, "sample74_o = %.6f\n", sample74_o);
     fprintf(fp, "sample75_o = %.6f\n", sample75_o);
     fprintf(fp, "sample76_o = %.6f\n", sample76_o);
     fprintf(fp, "sample77_o = %.6f\n", sample77_o);
}

double mrbay (FILE *fp, int32_or_float* ports, int maxPortPair) {

  return 0;  // the model is not updated with 8 input ports

  int32_or_float resAL, resCL, resGL, resTL;
  int32_or_float resAR, resCR, resGR, resTR;
  int32_or_float resA, resC, resG, resT;
  int32_or_float max1, max2, max3;
  int32_or_float Norm, InScaler, NumSites;
  int32_or_float InScaler_out, scP;

  int64_or_double IIA, IIC, IIG, IIT;
  int64_or_double xd, log_d, log_d1, res;

  int i;

  for (i = 0; i < 42; i++)
    printf("dbg: port %d = %x\n", i, ports[i].i);

  // clear !
  resAL.f = resCL.f = resGL.f = resTL.f = 0;
  resAR.f = resCR.f = resGR.f = resTR.f = 0;

  for (i = 0; i < 4; i++) {
    resAL.f += ports[i + 0].f * ports[32+i].f;
  }

  for (i = 4; i < 8; i++) {
    resAR.f += ports[i + 0].f * ports[32+i].f;
  }

  resA.f = resAL.f * resAR.f;

  for (i = 0; i < 4; i++) {
    resCL.f += ports[i + 8].f * ports[32+i].f;
  }

  for (i = 4; i < 8; i++) {
    resCR.f += ports[i + 8].f * ports[32+i].f;
  }

  resC.f = resCL.f * resCR.f;

  for (i = 0; i < 4; i++) {
    resGL.f += ports[i + 16].f * ports[32+i].f;
  }

  for (i = 4; i < 8; i++) {
    resGR.f += ports[i + 16].f * ports[32+i].f;
  }

  resG.f = resGL.f * resGR.f;

  for (i = 0; i < 4; i++) {
    resTL.f += ports[i + 24].f * ports[32+i].f;
  }

  for (i = 4; i < 8; i++) {
    resTR.f += ports[i + 24].f * ports[32+i].f;
  }

  resT.f = resTL.f * resTR.f;

  myprintf("LA PLF = %08x(%.6f)\n", resA.i, resA.f);

  myprintf("LC PLF = %08x(%.6f)\n", resC.i, resC.f);

  myprintf("LG PLF = %08x(%.6f)\n", resG.i, resG.f);

  myprintf("LT PLF = %08x(%.6f)\n", resT.i, resT.f);

  max1.f = resA.f > resC.f ? resA.f : resC.f;
  max2.f = resG.f > resT.f ? resG.f : resT.f;
  max3.f = max1.f > max2.f ? max1.f : max2.f;

  IIA.f = 0.1; //1.0;
  IIC.f = 0.1; //2.0;
  IIG.f = 0.1; //3.0;
  IIT.f = 0.1; //4.0;

  // Suppose circuit input port (width > 1) 
  // connected to mux sel (width = 1) (see also in GenerateDebugData.c)
  // Use bit zero
  Norm.f     = 1.0;

  InScaler.f = ports[40].f;
  NumSites.f = ports[41].f;

  resA.f = Norm.f ? resA.f / max3.f : resA.f;
  resC.f = Norm.f ? resC.f / max3.f : resC.f;
  resG.f = Norm.f ? resG.f / max3.f : resG.f;
  resT.f = Norm.f ? resT.f / max3.f : resT.f;

  xd.f = (double) resA.f * (double) IIA.f + (double) resG.f * (double) IIG.f + 
         (double) resC.f * (double) IIC.f + (double) resT.f * (double) IIT.f;

  myprintf("log_d1 input(single) = %08x(%.6f)\n", max3.i, max3.f);
  myprintf("log_d_input (double) = %016llx(%.11f)\n", xd.i, xd.f);
  
  // single-coeff + double-power
  log_d1.f = log_d1_cheby(fp, &max3, 0);
  myprintf("log_d1 output = %016llx(%.11f)\n", log_d1.i, log_d1.f);

  // double-coeff + double-power
  log_d.f = log_d_cheby(fp, &xd, 0);
  myprintf("log_d output = %016llx(%.11f)\n", log_d.i, log_d.f);

  scP.f = (float)log_d1.f;
  InScaler_out.f = (float)log_d1.f + InScaler.f;

  //res.f = (log_d1.f + (double)InScaler.f + log_d.f) * (double)NumSites.f;
  res.f = ((double)InScaler_out.f + log_d.f) * (double)NumSites.f;

  /*
  fprintf(fp, "fmux12 = %08x(%.6f)\n", resA.i, resA.f);
  fprintf(fp, "fmux13 = %08x(%.6f)\n", resC.i, resC.f);
  fprintf(fp, "fmux14 = %08x(%.6f)\n", resG.i, resG.f);
  fprintf(fp, "fmux15 = %08x(%.6f)\n", resT.i, resT.f);
  fprintf(fp, "fadd1 = %08x(%.6f)\n",  InScaler_out.i, InScaler_out.f);
  fprintf(fp, "dtof1 = %08x(%.6f)\n",  scP.i, scP.f);
  fprintf(fp, "fmuld2 = %016llx(%.6f)\n", res.i, res.f);
  */
  fprintf(fp, "fmux12 = %.6f\n", resA.f);
  fprintf(fp, "fmux13 = %.6f\n", resC.f);
  fprintf(fp, "fmux14 = %.6f\n", resG.f);
  fprintf(fp, "fmux15 = %.6f\n", resT.f);
  fprintf(fp, "fadd1 = %.6f\n",  InScaler_out.f);
  fprintf(fp, "dtof1 = %.6f\n",  scP.f);
  fprintf(fp, "fmuld2 = %.6f\n", res.f);
  return res.f;
}




