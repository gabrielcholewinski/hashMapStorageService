/*
Grupo 49
Guilherme Teixeira - 49021
Ruhan Azevedo - 51779
Gabriel Freitas - 51035
*/
#include <stdlib.h>
#include <string.h>

#include "data.h"
#include "entry.h"

struct entry_t *entry_create(char *key, struct data_t *data){
	if(key == NULL || data == NULL) return NULL;
 	struct entry_t *entry = malloc(sizeof(struct entry_t));
	entry->key = key;
	entry->value = data;
	return entry;
}

void entry_initialize(struct entry_t *entry){
	entry->key = NULL;
	entry->value = NULL;
}

void entry_destroy(struct entry_t *entry){
	if(entry != NULL){
		data_destroy(entry->value);
		free(entry->key);
		free(entry);
	}
}

struct entry_t *entry_dup(struct entry_t *entry){
	if(entry == NULL) return NULL;
	struct entry_t *entry_result = malloc(sizeof(struct entry_t));
	entry_result->key = strdup(entry->key);
	/*if(entry_result->key == NULL){
    entry_destroy(entry_result);
    return NULL;
  }
	strcpy(entry_result->key, entry->key);*/
	entry_result->value = data_dup(entry->value);
	return entry_result;
}
