/*
Grupo 49
Guilherme Teixeira - 49021
Ruhan Azevedo - 51779
Gabriel Freitas - 51035
*/

#ifndef _FIFO_H
#define _FIFO_H

#include "task.h"

struct fifo{
  struct task_t *head;
  struct task_t *tail;
  int size;
};

struct fifo *fifo_create();

int fifo_add(struct fifo *fifo, struct task_t *task);

struct task_t *pop(struct fifo *fifo);

void fifo_delete(struct fifo *fifo);

#endif
