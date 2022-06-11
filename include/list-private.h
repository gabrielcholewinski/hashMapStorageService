/*
Grupo 49
Guilherme Teixeira - 49021
Ruhan Azevedo - 51779
Gabriel Freitas - 51035
*/

#ifndef _LIST_PRIVATE_H
#define _LIST_PRIVATE_H

#include "list.h"
#include "entry.h"

struct node_t {
  struct entry_t *entrada;
  struct node_t *proximo;
};

struct list_t {
  struct node_t *head;
  struct node_t *tail;
  int size;
};

void list_print(struct list_t* list);

#endif
