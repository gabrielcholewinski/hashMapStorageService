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
#include "list-private.h"
#include "list.h"

struct list_t *list_create(){
  struct list_t *lista = (struct list_t *) malloc(sizeof(struct list_t));
  lista->head = NULL;
  lista->tail = NULL;
  lista->size = 0;
  return lista;
}

void list_destroy(struct list_t *list){
  struct node_t *atual = list->head;
  while(atual != NULL){
    entry_destroy(atual->entrada);
    struct node_t *prox = atual->proximo;
    free(atual);
    atual = prox;
  }
  free(list);
}

int list_add(struct list_t *list, struct entry_t *entry){
  if(list == NULL || entry == NULL){
    return -1;
  }

  if(list_get(list, entry->key) != NULL){
    struct node_t *no = list->head;
    while(no != NULL){
      if(strcmp(no->entrada->key, entry->key) == 0){
        entry_destroy(no->entrada);
        no->entrada = entry;
        return 0;
      }
      no = no->proximo;
    }
    return -1;
  }


struct node_t* addNode = (struct node_t *) malloc(sizeof(struct node_t));
  //addNode->entrada = (struct entry_t *) malloc(sizeof(struct entry_t));
  addNode->entrada = entry;
	addNode->proximo = NULL;

  if(list->head == NULL){
    //list->head = (struct node_t *) malloc(sizeof(struct node_t));
    list->head = addNode;
    list->size++;
    return 0;
  } else if (list->tail == NULL){
    //list->tail = (struct node_t *) malloc(sizeof(struct node_t));
    list->tail = addNode;
    list->head->proximo = addNode;
    list->size++;
    return 0;
  } else {
    //list->tail->proximo = (struct node_t *) malloc(sizeof(struct node_t));
    list->tail->proximo = addNode;
    list->tail = addNode;
    list->size++;
    return 0;
  }
  return -1;
}


int list_remove(struct list_t *list, char *key){
  if(list == NULL || key == NULL || list->head == NULL){
    return -1;
  }
  struct node_t *atual = list->head;
  if(strcmp(atual->entrada->key, key) == 0){
    list->head = atual->proximo;
    entry_destroy(atual->entrada);
    free(atual);
    list->size--;
    return 0;
  }

  while(atual != NULL && atual->proximo != list->tail){
    if(strcmp(atual->proximo->entrada->key, key) == 0){
      struct node_t *del = atual->proximo;
      atual->proximo = atual->proximo->proximo;
      entry_destroy(del->entrada);
      free(del);
      list->size--;
      return 0;
    }
    atual = atual->proximo;
  }
  if(strcmp(atual->proximo->entrada->key, key) == 0){
    struct node_t *del = atual->proximo;
    atual->proximo = atual->proximo->proximo;
    entry_destroy(del->entrada);
    free(del);
    list->size--;
    list->tail = atual;
    return 0;
  }

  return -1;
}


struct entry_t *list_get(struct list_t *list, char *key){
  if(list == NULL || key == NULL){
    return NULL;
  }
  struct node_t *node = list->head;
  while(node != NULL){
    if(strcmp(key, node->entrada->key) == 0){
      return node->entrada;
    }
    node = node->proximo;
  }
  return NULL;
}


int list_size(struct list_t *list){
  return list->size;
}


char **list_get_keys(struct list_t *list){
  char **rt;
  if(list == NULL || list->size == 0){
    rt = (char**) malloc(sizeof(char*)+1);
    return rt;
  }
  int i = 0;
  struct node_t *node = list->head;
  rt = (char**) malloc(sizeof(char *)*(list->size + 1));
  while (node != NULL) {
    rt[i] = strdup(node->entrada->key);
    i++;
    node = node->proximo;
  }
  rt[i] = NULL;
  return rt;
}


void list_free_keys(char **keys){
  int i = 0;
  while(keys[i] != NULL){
    free(keys[i]);
    i++;
  }
  free(keys);
}


void list_print(struct list_t* list){
  struct node_t *atual = list->head;
  while(atual != NULL){
    atual = atual->proximo;
  }
  printf("\n");
}
