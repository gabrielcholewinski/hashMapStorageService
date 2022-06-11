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
#include "entry.h"
#include "serialization.h"

int data_to_buffer(struct data_t *data, char **data_buf){
  if(data == NULL || data_buf == NULL){
    return -1;
  }
  int bufferSize = data->datasize + sizeof(data->datasize);
  *data_buf = (char *) malloc(bufferSize);
  if(*data_buf == NULL){
    return -1;
  }
  memcpy(*data_buf, (char *) &data->datasize, sizeof(data->datasize));
  memcpy(*data_buf + sizeof(data->datasize), data->data, data->datasize);
  return bufferSize;
}


struct data_t *buffer_to_data(char *data_buf, int data_buf_size){
  if(data_buf_size <= 0 || data_buf == NULL){
    return NULL;
  }
  int t = *(int *) data_buf;
  char *d = (char *) malloc(t);
  memcpy(d, data_buf + sizeof(t), t);
  return data_create2(t, d);
}


int entry_to_buffer(struct entry_t *data, char **entry_buf){
  if(entry_buf == NULL || data == NULL) return -1;
  int dataBufferSize = data_to_buffer(data->value, entry_buf);
  if(dataBufferSize < 0) return -1;
  int keySize = strlen(data->key) + 1;
  if(realloc(*entry_buf, dataBufferSize + keySize + sizeof(int)) == NULL) return -1;
  memcpy(*entry_buf + dataBufferSize, (char *) &keySize, sizeof(int));
  memcpy(*entry_buf + dataBufferSize + sizeof(int), data->key, keySize);
  return dataBufferSize + keySize + sizeof(int);
}

struct entry_t *buffer_to_entry(char *entry_buf, int entry_buf_size){
  if(entry_buf == NULL || entry_buf_size <= 0) return NULL;
  struct data_t *data = data_dup(buffer_to_data(entry_buf, entry_buf_size));
  if(data == NULL) return NULL;
  int espaco = sizeof(data->datasize) + data->datasize;
  //printf("\nO espaco eh %d\n", espaco);
  int tamKey = *(int *) entry_buf + espaco;
  char *key = (char *) malloc(tamKey);
  memcpy(key, entry_buf + espaco + sizeof(int), tamKey);
  return entry_create(key, data);
}
