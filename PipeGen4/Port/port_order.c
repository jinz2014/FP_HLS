#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>

void PrintArray(int *a, int n) {
  int i;
  for (i = 0; i < n; i++) printf("%3d ", a[i]);
  printf("\n");
}

#ifdef TEST_PORT_ORDERING

void port_order (int Nx, int Nk) {

  int N = Nx + Nk;
  int P, px, pk;
  int DII;

  // no port orderings for P = 1 and P = N
  for (P = 2; P < N; P++) {

    DII = (int) ceil((double) N / P);

    for (px = 1; px <= Nx; px++) {
      pk = P - px;  // px + pk = P
      if (pk >= 1 && pk <= Nk && (int) ceil((double) Nx / px) <= DII && (int) ceil((double) Nk / pk) <= DII ) {
        printf("DII=%d port partition (%dX + %dK)\n", DII, px, pk);
        printf("Port X ordering matrix\n");
        perm_rep(px, DII, Nx);

        printf("Port K ordering matrix\n");
        perm_rep(pk, DII, Nk);
      }
    }
  }
}

int main() {

  port_order(2, 3); // uuci
  //port_order(3, 7); // ordbur
  //port_order(4, 10); // ordbur
  //port_order(4, 9); // ppbr

}
#endif

