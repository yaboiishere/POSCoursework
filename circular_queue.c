#include "include/circular_queue.h"

node *enqueue(node *head, ucontext_t *data) {
  node *new_node = (node *)malloc(sizeof(node));
  new_node->data = data;
  if (head == NULL) {
    new_node->next = new_node;
    new_node->prev = new_node;
    head = new_node;
  } else {
    new_node->next = head;
    new_node->prev = head->prev;
    head->prev->next = new_node;
    head->prev = new_node;
  }
  return head;
}

node *dequeue(node *head) {
  if (head == NULL) {
    return NULL;
  }
  node *temp = head;
  if (head->next == head) {
    head = NULL;
  } else {
    head->prev->next = head->next;
    head->next->prev = head->prev;
    head = head->next;
  }
  free(temp);
  return head;
}

ucontext_t *peek(node *head) {
  if (head == NULL) {
    return NULL;
  }
  return head->data;
}

int is_empty(node *head) {
  if (head == NULL) {
    return 1;
  }
  return 0;
}
