/*
Grupo 49
Guilherme Teixeira - 49021
Ruhan Azevedo - 51779
Gabriel Freitas - 51035
*/

#ifndef _TABLE_PRIVATE_H
#define _TABLE_PRIVATE_H

#include "list.h"

struct table_t {
  struct list_t **list;
  int size;
  int countLists;
};

/* Função para criar/inicializar uma nova tabela hash, com n linhas
 * (módulo da função HASH).
 */
struct table_t *table_create(int n);

void table_print(struct table_t *table);

#endif
