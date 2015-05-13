/* Parser demo program */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include "ExprParser.h"

char *expr;
char *name;
FILE *fp, *cfp, *gfp;

int main() 
{
  //double answer;
  char *n, *p;
  char filename[20];

  n = (char *) malloc (20); 
  p = (char *) malloc (512); // large enough to accmodate the expr
  if (!n || !p) {
    printf("allocation failure");
    exit(1);
  }

  name = n;
  expr = p;

  do {

    *expr = '\0';
    *name = '\0';

    printf("Enter expression name: "); gets(name);
    printf("read expression name %s\n", name);

    if (!strcmp(name, "uuci")) {
      strcpy(expr, "(V * S / Km) / (-1.0 + (S / Km * (-1 + I / Ki)))");
    }

    if (!strcmp(name, "ppbr")) {
    // -------------------------------------
    // ppbr
    //        Vf(AB - PQ/Keq)
    // -------------------------------------
    //  A*B + Kmb*A + Kma*B(1+Q/Kiq) + K1 
    //
    // K1 = [Vf/(Vr*Keq)][Kmq*P(-1+A/Kia)+Q*(Kmp + P)] 
    // -------------------------------------
      strcpy(expr, "Vf * (A * B + P * Q / Keq) / \
          ((A * B + Kmb * A + Kma * B * (-1 + Q / Kiq)) +\
           Vf / (Vr * Keq) * (Kmq * P * (-1 + A / Kia) + \
           Q * (Kmp + P)))"); 
    }

    if (!strcmp(name, "ordbur")) {

    // --------------------------------------------------------
    // ordbur
    //                  Vf(AB - P/Keq)
    // --------------------------------------------------------
    //  A*B + Kma*B + Kmb*A + [Vf/(Vr*Keq)][Kmp + P(-1+A/Kia)]
    // --------------------------------------------------------

      strcpy(expr, "Vf * (A * B + P/Keq) / \
      (A*B + Kma*B + Kmb*A + (Vf/(Vr*Keq)) * (Kmp + P * (-1+A/Kia)))");
    }

    if (!strcmp(name, "ordbbr")) {
      //        Vf(AB - PQ/Keq)
      // -------------------------------------
      // AB(-1+P/Kip) + Kmb(A+Kia) + Kma*B + K1
      //
      // K1 = [Vf/(Vr*Keq)][Kmq*P(-1+A/Kia)+Q*K2)] 
      // K2 = Kmp[-1+Kma*B/(Kia*Kmb)+P(-1+B/Kib)]]

      strcpy(expr, "Vf * (A * B + P * Q/Keq) / \
       ((A * B * (-1 + P / Kip) + Kmb*(A + Kia) + Kma * B) + \
       (Vf/(Vr*Keq)) * (Kmq * P * (-1+A/Kia) + \
         Q * (Kmp * (-1 + Kma * B / (Kia * Kmb) + P * (-1 + B / Kib)))))");
    }


    if (!strcmp(name, "sample1")) {
      strcpy(expr, "a + b");
    }
    if (!strcmp(name, "sample2")) {
      strcpy(expr, "a + b + b + d");
    }

    if (!strcmp(name, "sample3")) {
      strcpy(expr, "a * b / (-1.0 + c + d) * (e + f)");
    }
    if (!strcmp(name, "sample4")) {
      strcpy(expr, "(a * b +  c * d) / ((-1.0 + c + d) * (e + f))");
    }
    if (!strcmp(name, "sample5")) {
      strcpy(expr, "a + a * (b - c) + (b - c) * d");
    }

    if (!strcmp(name, "sample6")) {
      strcpy(expr, "(B - (B + 2 - 4 * A * C) + 0.5) / (2 * A)");
    }

    if (!strcmp(name, "sample7")) {
      strcpy(expr, "(a + 1.0) * (b + 1.0) * (c + 1.0)");
    }

    if (!strcmp(name, "sample8")) {
      strcpy(expr, "(A * 3 + (B * 0.5 + 2 + 4 * A * C) + 0.5) / \
          ((A + 0.5) * 4 + A * 4 + B * 0.5 + C * 3 + 0.5)");
    }

    if (!strcmp(name, "sample9")) {
      strcpy(expr, "(A * 3 + (B * 0.5 + 3 + 4 * A * D + E * 2 + F * 3) + 0.5) / \
          ((A + 0.5) * 4 + A * 4 + C * 0.5 + D * 3 + 0.5 + E * 0.5 + F * 0.5)");
    }

    if (!strcmp(name, "sample10")) {
      strcpy(expr, "((a + b) * (c + d)) * ((e + f) * (g + h))");
    }

    if (!strcmp(name, "plf")) {
      strcpy(expr, 
          "((a * 0.1 + b * 0.2) + (c * 0.3 + d * 0.4)) * ((e * 0.5 + f * 0.6) + (g * 0.7 + h * 0.8))");
    }

    if (!*expr) break;

    sprintf(filename, "DFG/%s.txt", name);
    fp = fopen(filename, "w+"); 

    sprintf(filename, "Benchmark/%s.c", name);
    cfp = fopen(filename, "w+"); 

    sprintf(filename, "Dot/%s.dot", name);
    gfp = fopen(filename, "w+"); 

    printf("eval expression: %s\n", expr);
    eval_exp();

    fclose(fp);
    fclose(cfp);
    fclose(gfp);

  } while (*p);

  free(p);
  free(n);
  return 0;
}
