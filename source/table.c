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
#include "list.h"
#include "list-private.h"
#include "table-private.h"
#include "table.h"

struct table_t *table_create(int n){
  if(n <= 0) return NULL;
  struct table_t *table = (struct table_t *) malloc(sizeof(struct table_t));
  table->size = 0;
  table->countLists = n;
  table->list = (struct list_t **) malloc(sizeof(struct list_t*) * n);
  for(int i = 0; i < n; i++){
    table->list[i] = list_create();
  }
  return table;
}

void table_destroy(struct table_t *table){
  if(table == NULL || table->list == NULL) return;
  for(int i = 0; i < table->countLists; i++){
    list_destroy(table->list[i]);
  }
  free(table->list);
  free(table);
}

int hash(char *key, int size){
 int count = 0;
 for(int i = 0; key[i] != '\0'; i++){
  int ascii = (int) key[i];
  count += ascii;
 }
 return count % size;
}

int table_put(struct table_t *table, char *key, struct data_t *value){
  if(table == NULL || key == NULL || value == NULL || table->countLists == 0) return -1;
  int result = -1;
  char *newKey = strdup(key);
  struct data_t *newValue = data_dup(value);
  if(newValue == NULL) return -1;
  struct entry_t *entry = entry_create(newKey, newValue);
  if(entry == NULL){
    entry_destroy(entry);
    return -1;
  }
  int indiceLista = hash(newKey, table->countLists);
  if(list_get(table->list[indiceLista], key) != NULL ){
    list_remove(table->list[indiceLista], key);
    list_add(table->list[indiceLista], entry);
    result = 0;
  }else {
    list_add(table->list[indiceLista], entry);
    table->size++;
    result = 0;
  }
  return result;
}

struct data_t *table_get(struct table_t *table, char *key){
  if(table == NULL || key == NULL || table->countLists == 0) return NULL;
  for(int i = 0; i < table->countLists; i++){
    struct entry_t *entry = list_get(table->list[i],key);
    if(entry != NULL){
      struct data_t *value = data_dup(entry->value);
      return value;
    }
  }
  return NULL;
}

int table_del(struct table_t *table, char *key){
  if(table == NULL || key == NULL || table->countLists == 0) return -1;
  int indiceLista = hash(key, table->countLists);
  struct entry_t *entry = list_get(table->list[indiceLista],key);
  if(entry != NULL){
    list_remove(table->list[indiceLista],key);
    table->size--;
    return 0;
  }
  return -1;
}

int table_size(struct table_t *table){
  if(table == NULL) return -1;
  return table->size;
}

char **table_get_keys(struct table_t *table){
  char **finalArray = (char**) malloc(sizeof(char *)*(table->size+1));
  if(table == NULL || finalArray == NULL) return NULL;
  int tamanho = 0;
  for(int i = 0; i < table->countLists; i++){
    char **newArray = list_get_keys(table->list[i]);
    for(int j = 0; j < table->list[i]->size; j++){
        finalArray[tamanho] = newArray[j];
        tamanho++;
    }
    free(newArray);
  }
  finalArray[tamanho++] = NULL;
  return finalArray;
}

void table_free_keys(char **keys){
  list_free_keys(keys);
}
