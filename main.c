#include "cscheduler.h"
#include <stdio.h>
#include <unistd.h>

sem_t *sem;
void f1() {
  while (1) {
    sem_wait(sem);
    printf("-");
    sleep(1);
    sem_post(sem);
  }
}
void f2() {
  while (1) {
    printf("+");
    sleep(1);
  }
}

void f3() {
  while (1) {
    sem_wait(sem);
    printf("*");
    sleep(1);
    sem_post(sem);
  }
}

void f4() {
  while (1) {
    printf("/");
    sleep(1);
  }
}

int main() {
  init_library();
  sem = sem_init(1);
  create_task(f2);
  create_task(f4);

  create_task(f1);
  create_task(f3);

  while (1)
    ;

  return 0;
}
