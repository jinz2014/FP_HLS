#include <stdio.h>

int main() {
  char a[10];
  sprintf(a, "%s", "go");
  printf("%s\n", a); // go

  sprintf(a, "%s", "cock");
  printf("%s\n", a); // cock
}
