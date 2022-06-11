/*
Grupo 49
Guilherme Teixeira - 49021
Ruhan Azevedo - 51779
Gabriel Freitas - 51035
*/
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "fifo.h"

struct fifo *fifo_create(){
  struct fifo *fifo = (struct fifo *) malloc(sizeof(struct fifo));
  fifo->head = NULL;
  fifo->tail = NULL;
  fifo->size = 0;
  return fifo;
}

int fifo_add(struct fifo *fifo, struct task_t *task){
  if(fifo->size == 0){
    fifo->head = task;
    fifo->size++;
    return 0;
  } else if (fifo->tail == NULL){
    fifo->tail = task;
    fifo->head->proximo = task;
    fifo->size++;
    return 0;
  } else {
    fifo->tail->proximo = task;
    fifo->tail = task;
    fifo->size++;
    return 0;
  }
  return -1;
}

struct task_t *pop(struct fifo *fifo){
  if(fifo->size <= 0){
    return NULL;
  }
  struct task_t *ret;
  ret = fifo->head;
  fifo->head = ret->proximo;
  fifo->size--;
  return ret;
}

void fifo_delete(struct fifo *fifo){
  if(fifo == NULL) return;
  struct task_t *task;
  while(fifo->size != 0){
    task = pop(fifo);
    if(task->key != NULL) free(task->key);
    if(task->op == 1){
      free(task->data);
    }
    free(task);
  }
  free(fifo);
}
