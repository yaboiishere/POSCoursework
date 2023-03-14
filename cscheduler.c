#include "cscheduler.h"

// 10ms in nsec
#define INTERVAL 10000000

#define handle_error(msg)                                                      \
  do {                                                                         \
    perror(msg);                                                               \
    exit(EXIT_FAILURE);                                                        \
  } while (0)

static ucontext_t uctx_main;
node *process_queue;

sem_t empty, taken;
sem_t mutex;

void timer_handler(int signum) {
  sem_wait(&empty);
  sem_wait(&mutex);
  process_queue = process_queue->next;
  if (swapcontext(peek(process_queue->prev), peek(process_queue))) {
    handle_error("swapcontext");
  }
  sem_post(&mutex);
  sem_post(&empty);
}

void init_library() {
  sem_init(&mutex, 1, 1);
  sem_init(&empty, 1, 1);
  sem_init(&taken, 0, 1);
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
  
  sem_wait(&taken);
  sem_wait(&mutex);
  new_context->uc_link =
      is_empty(process_queue) ? &uctx_main : peek(process_queue);
  makecontext(new_context, start_routine, 0);
  process_queue = enqueue(process_queue, new_context);
  sem_post(&mutex);

  // add extra 1 to the value of the semaphore so that this scales with input
  // functions
  sem_post(&mutex);
  sem_post(&empty);

  return 0;
}
