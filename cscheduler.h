#include "include/circular_queue.h"
#include <assert.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <ucontext.h>

typedef struct {
  int value;
  node *blocked_processes;
} sem_t;

sem_t *sem_init(unsigned int value);
void sem_wait(sem_t *sem);
void sem_post(sem_t *sem);
void sem_destroy(sem_t *sem);

void init_library();
int create_task(void *(start_routine));
