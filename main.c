#include "cscheduler.h"
#include <stdio.h>

void f1() {
  while (1) {
    printf("-");
  }
}
void f2() {
  while (1) {
    printf("+");
  }
}

void f3() {
  while (1) {
    printf("*");
  }
}

void f4() {
  while (1) {
    printf("/");
  }
}

int main() {
  init_library();
  create_task(f4);
  create_task(f3);
  create_task(f2);
  create_task(f1);

  while (1)
    ;

  return 0;
}
