#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define SIZE 5

//============================================
// Return total number of permutations
//============================================
int fac(int n) {
  if (n == 1)
    return 1;
  else
    return n * fac(n-1);
}

//============================================
// Generate all permutations of array a[]
// a[]: array to be permutated
// m:   size of a[] 
//============================================
int perm(int a[], int m) {

  int i, j, k, l;
  int tmp;
  int size;
  int idx;

  // step 1
  idx = -1;
  for (k = 0; k < m-1; k++)
    if (a[k] < a[k+1]) idx = k;

  if (idx == -1) {// no index found and it is the last perm
    return idx;
  }

  // step 2 find the largest index l such that a[k] < a[l]
  else { 
    for (l = m - 1; l >= 0 ; l--)
      if (a[idx] < a[l]) break;
  }

  // step 3  Swap a[k] with a[l]
  tmp  = a[idx];
  a[idx] = a[l];
  a[l] = tmp;

  size = m - 1 - idx;

  if (size > 1) {
    int *b = malloc(sizeof(int) * size);
    for (j = idx + 1; j < m; j++)
      b[--size] = a[j];
    assert(size == 0);
    for (j = idx + 1; j < m; j++)
      a[j] = b[size++]; 
    free(b);
  } 
}

#ifdef TEST_PERMUTATION
void PrintArray(int *a, int n) {
  int i;
  for (i = 0; i < n; i++) printf("%3d ", a[i]);
  printf("\n");
}

int main() {

  int a[SIZE] = {0, 1, 2, 3, 4};
  int num;
  int iter;
  int r;
  int k;


  // consider the case of more than !size iterations
  num = 4*fac(SIZE);

  for (iter = 0; iter < num; iter++) {
    PrintArray(a, SIZE);
    r = perm(a, SIZE);

    if (r == -1) 
      for (k = 0; k < SIZE; k++) a[k] = k;
  }
}
#endif

