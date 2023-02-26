#include "include/circular_queue.h"
#include <assert.h>
#include <semaphore.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <ucontext.h>

void init_library();
int create_task(void *(start_routine));
