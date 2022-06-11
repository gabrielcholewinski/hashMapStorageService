/*
Grupo 49
Guilherme Teixeira - 49021
Ruhan Azevedo - 51779
Gabriel Freitas - 51035
*/
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "data.h"

struct data_t *data_create(int size){
  if(size <= 0){
    return NULL;
  }
  struct data_t *pointer = (struct data_t *) malloc(sizeof(struct data_t));
  pointer->datasize = size;
  pointer->data = (void *) malloc(size * sizeof(void));
  return pointer;
}

struct data_t *data_create2(int size, void *data){
  if(size <= 0 || data == NULL){
    return NULL;
  }
  struct data_t *pointer = (struct data_t *) malloc(sizeof(struct data_t));
  pointer->datasize = size;
  pointer->data = data;
  return pointer;
}

void data_destroy(struct data_t *data){
  if(data == NULL){
    return;
  }
  free(data->data);
  free(data);
}

struct data_t *data_dup(struct data_t *data){
  if(data == NULL || data->data == NULL || data->datasize <= 0){
    return NULL;
  }
  struct data_t *pointer = (struct data_t *) malloc(sizeof(struct data_t));
  pointer->data = data_create(data->datasize);
  if(pointer->data == NULL){
    free(pointer->data);
    return NULL;
  }
  pointer->datasize = data->datasize;
  memcpy(pointer->data, data->data, pointer->datasize);
  return pointer;
}
