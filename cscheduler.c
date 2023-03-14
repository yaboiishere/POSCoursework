#include "cscheduler.h"
#include "include/circular_queue.h"
#include <stdio.h>

// 10ms in nsec
#define INTERVAL 10000000

#define handle_error(msg)                                                      \
  do {                                                                         \
    perror(msg);                                                               \
    exit(EXIT_FAILURE);                                                        \
  } while (0)

static ucontext_t uctx_main;
node *process_queue;

// implement a semaphore

sem_t *sem_init(unsigned int value) {
  sem_t *sem = (sem_t *)malloc(sizeof(sem_t));
  sem->value = value;
  sem->blocked_processes = NULL;
  return sem;
}

void sem_wait(sem_t *sem) {
  sem->value--;
  if (sem->value < 0) {
    sem->blocked_processes =
        enqueue(sem->blocked_processes, peek(process_queue));
  }
}

void sem_post(sem_t *sem) {
  sem->value++;
  if (sem->value <= 0) {
    if (sem->blocked_processes == NULL)
      return;
    process_queue = enqueue(process_queue, peek(sem->blocked_processes));
    sem->blocked_processes = dequeue(sem->blocked_processes);
  }
}

void sem_destroy(sem_t *sem) { free(sem); }

void timer_handler(int signum) {
  process_queue = process_queue->next;
  if (swapcontext(peek(process_queue->prev), peek(process_queue))) {
    handle_error("swapcontext");
  }
}

void init_library() {
  getcontext(&uctx_main);
  process_queue = enqueue(NULL, &uctx_main);

  struct sigaction act;
  clockid_t clock_id;
  timer_t timer_id;
  struct sigevent clock_sig_event;
  struct itimerspec clock_timer_spec;
  int ret;

  // Set up the signal handler
  memset(&act, 0, sizeof(act));
  act.sa_handler = &timer_handler;
  ret = sigaction(SIGUSR1, &act, NULL);
  assert(ret == 0);

  clock_id = CLOCK_REALTIME;
  memset(&clock_sig_event, 0, sizeof(clock_sig_event));
  clock_sig_event.sigev_notify = SIGEV_SIGNAL;
  clock_sig_event.sigev_signo = SIGUSR1;
  clock_sig_event.sigev_notify_attributes = NULL;

  // Create internal timer
  ret = timer_create(clock_id, &clock_sig_event, &timer_id);
  assert(ret == 0);

  // Set interval 10ms
  clock_timer_spec.it_interval.tv_sec = 0;
  clock_timer_spec.it_interval.tv_nsec = INTERVAL;

  // Set initial expiration 10ms
  clock_timer_spec.it_value.tv_sec = 0;
  clock_timer_spec.it_value.tv_nsec = INTERVAL;

  // Create timer
  ret = timer_settime(timer_id, 0, &clock_timer_spec, NULL);
  assert(ret == 0);
}

int create_task(void *(start_routine)) {
  ucontext_t *new_context = (ucontext_t *)malloc(sizeof(ucontext_t));

  if (getcontext(new_context) == -1)
    handle_error("getcontext");

  new_context->uc_stack.ss_sp = calloc(1, MINSIGSTKSZ);
  new_context->uc_stack.ss_size = MINSIGSTKSZ;
  new_context->uc_stack.ss_flags = 0;

  new_context->uc_link =
      is_empty(process_queue) ? &uctx_main : peek(process_queue);
  makecontext(new_context, start_routine, 0);
  process_queue = enqueue(process_queue, new_context);

  return 0;
}
