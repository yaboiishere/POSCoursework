#pragma once
#include <stdlib.h>
#include <ucontext.h>

typedef struct node {
  ucontext_t *data;
  struct node *next;
  struct node *prev;
} node;

node *enqueue(node *head, ucontext_t *data);

node *dequeue(node *head);

ucontext_t *peek(node *head);

int is_empty(node *head);
