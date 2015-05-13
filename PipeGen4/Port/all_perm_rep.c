#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

//==========================================================
// each elem of array X is in the range [min, max]
// the length of X is r
//==========================================================
int Next(int min, int max, int r, int *X) {

  if (X == NULL) return 0;

  int k = r - 1;

  while (1) {
    X[k]++;
    if (X[k] <= max) return 1;
    X[k] = min;
    k--;
    if (k < 0) return 0;
  }

}

//==========================================================
// Generate all permutations of a give set(n) with repetition
//
// e.g. (0, 1, 2) n = 2, r = 3 
// 000, 001, 002, 010, ..., 222 

// e.g. (0, 1, 2) n = 2, r = 4 
// 0000, 0001, 0002, 0010, ..., 2222 
// 
// constraint:
// The sum of the digits is less than or equal to sum
//==========================================================


void perm_rep(int m, int n, int r, int sum) {
  int i;
  int tmp;
  int *X;

  if (r > 0) {
    X = (int *) malloc (sizeof(int) * r);

    // enumeration all combinations
    for (i = 0; i < r; i++) X[i] = m;

    do {

      /*
      tmp = 0;
      for (i = 0; i < r; i++) tmp += X[i];
      if (tmp == sum) 
      */
      PrintArray(X, r); 

    } while (Next(m, n, r, X));

    free (X);
  }
}

#ifdef TEST_PERM_REP

int main() {

  int m, n, r;

  for (n = 2; n <= 3; n++) {
    for (m = 1; m < n; m++) {
      for (r = 1; r <= 4; r++) {
        printf("(min max, r) = (%d %d %d)\n", m, n, r);
        perm_rep(m, n, r, 3);
      }
    }
  }
}
#endif

