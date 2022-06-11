/*
Grupo 49
Guilherme Teixeira - 49021
Ruhan Azevedo - 51779
Gabriel Freitas - 51035
*/
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include "data.h"
#include "entry.h"
#include "client_stub.h"
#include "client_stub-private.h"
#include "network_client.h"
#include "message-private.h"

#include "sdmessage.pb-c.h"

struct rtable_t *rtable_connect(const char *address_port){
  struct rtable_t * rtable = (struct rtable_t *) malloc(sizeof(struct rtable_t));
  if(rtable == NULL) return NULL;
  char * hostname = (char *) malloc(20);
  char * port = (char *) malloc(20);
  char delim[] = ":";
  char *ptr = strtok((char*)address_port, delim);
  strcpy(hostname, ptr);
  ptr = strtok(NULL, delim);
  strcpy(port, ptr);

  if(address_port == NULL) return NULL;
  if((rtable->sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){ // Cria socket TCP
      perror("Erro ao criar socket TCP!");
      free(port);
      free(hostname);
      free(rtable);
      return NULL;
  }

  rtable->server.sin_family = AF_INET; // família de endereços
  rtable->server.sin_port = htons(atoi(port)); // Porta TCP
  if (inet_pton(AF_INET, hostname, &rtable->server.sin_addr) < 1) { // Endereço IP
      printf("Erro ao converter IP\n");
      close(rtable->sockfd);
      free(port);
      free(hostname);
      free(rtable);
      return NULL;
  }
  if(network_connect(rtable) == -1){
    free(port);
    free(hostname);
    free(rtable);
    return NULL;
  }
  free(port);
  free(hostname);
  return rtable;
}

int rtable_disconnect(struct rtable_t *rtable){
  if(rtable == NULL) return -1;
  network_close(rtable);
  free(rtable);
  return 0;
}

int rtable_put(struct rtable_t *rtable, struct entry_t *entry){
  if(rtable == NULL || entry == NULL) return -1;
  MessageT request_messageT;
  message_t__init(&request_messageT); //inicializar a mensagem
  request_messageT.opcode = MESSAGE_T__OPCODE__OP_PUT;  //tipo da operaçao
  request_messageT.c_type = MESSAGE_T__C_TYPE__CT_ENTRY;  //tipo de conteudo
  request_messageT.key = entry->key;
  request_messageT.data = entry->value->data;
  request_messageT.data_size = entry->value->datasize;
  request_messageT.keys = NULL;
  request_messageT.n_keys = 0;
  if(request_messageT.key == NULL || request_messageT.data == NULL ||
    request_messageT.data_size < 0){
    return -1;
  }
  struct message_t * request_message = to_message(&request_messageT);
  if(request_message == NULL){
    return -1;
  }
  struct message_t * response_message = network_send_receive(rtable, request_message); //mensagem de resposta
  if(response_message == NULL){
    response_message->opcode = MESSAGE_T__OPCODE__OP_ERROR;
    response_message->c_type = MESSAGE_T__C_TYPE__CT_NONE;
    free_message(response_message);
    return -1;
  } else {
    printf("Operacao enviada com sucesso de numero: %d!\n", response_message->data_size);
  }
  free_message(response_message);
  return 0;
}

struct data_t *rtable_get(struct rtable_t *rtable, char *key){
  if(rtable == NULL || key == NULL) return NULL;
  MessageT request_messageT;
  message_t__init(&request_messageT); //inicializar a mensagem
  request_messageT.opcode = MESSAGE_T__OPCODE__OP_GET;  //tipo da operaçao
  request_messageT.c_type = MESSAGE_T__C_TYPE__CT_KEY;  //tipo de conteudo
  request_messageT.key = key;
  request_messageT.keys = NULL;
  request_messageT.n_keys = 0;
  request_messageT.data = NULL;
  request_messageT.data_size = 0;
  if(request_messageT.key == NULL){
    return NULL;
  }
  struct message_t * request_message = to_message(&request_messageT);
  if(request_message == NULL){
    free_message(request_message);
    return NULL;
  }
  struct message_t * response_message = network_send_receive(rtable, request_message);
  if(response_message == NULL){
    response_message->opcode = MESSAGE_T__OPCODE__OP_ERROR;
    response_message->c_type = MESSAGE_T__C_TYPE__CT_NONE;
    free_message(response_message);
    return NULL;
  }
  if(response_message->data_size > 0){
    struct data_t * new_data = data_create2(response_message->data_size, response_message->data);
    struct data_t *dup = data_dup(new_data);
    printf("Dados retornados com sucesso, com valor e tamanho: %s e %d\n",
    response_message->data, response_message->data_size);
    //free(request_message);
    free_message(response_message);
    free(new_data);
    return dup;
  }else {
    printf("Dados não existentes!\n");
    free_message(response_message);
    return data_create2(1,"");
  }
}

int rtable_del(struct rtable_t *rtable, char *key){
  if(rtable == NULL || key == NULL) return -1;
  MessageT request_messageT;
  message_t__init(&request_messageT); //inicializar a mensagem
  request_messageT.opcode = MESSAGE_T__OPCODE__OP_DEL;  //tipo da operaçao
  request_messageT.c_type = MESSAGE_T__C_TYPE__CT_KEY;  //tipo de conteudo
  request_messageT.key = key;
  request_messageT.keys = NULL;
  request_messageT.n_keys = 0;
  request_messageT.data = NULL;
  request_messageT.data_size = 0;
  if(request_messageT.key == NULL){
    return -1;
  }
  struct message_t * request_message = to_message(&request_messageT);
  if(request_message == NULL){
    free_message(request_message);
    return -1;
  }
  struct message_t * response_message = network_send_receive(rtable, request_message);
  if(response_message == NULL){
    response_message->opcode = MESSAGE_T__OPCODE__OP_ERROR;
    response_message->c_type = MESSAGE_T__C_TYPE__CT_NONE;
    free_message(response_message);
    return -1;
  }else if(response_message->opcode != MESSAGE_T__OPCODE__OP_ERROR){
    printf("Operacao enviada com sucesso de numero: %d!\n", response_message->data_size);
  }else{
    printf("Nao existe essa chave!\n");
  }
  free_message(response_message);
  return 0;
}

int rtable_size(struct rtable_t *rtable){
  if(rtable == NULL) return -1;
  MessageT request_messageT;
  message_t__init(&request_messageT);
  request_messageT.opcode = MESSAGE_T__OPCODE__OP_SIZE;  //tipo da operaçao
  request_messageT.c_type = MESSAGE_T__C_TYPE__CT_NONE;  //tipo de conteudo
  request_messageT.key = NULL;
  request_messageT.keys = NULL;
  request_messageT.n_keys = 0;
  request_messageT.data = NULL;
  request_messageT.data_size = 0;
  struct message_t * request_message = to_message(&request_messageT);
  if(request_message == NULL){
    free_message(request_message);
    return -1;
  }
  struct message_t * response_message = network_send_receive(rtable, request_message);
  if(response_message == NULL){
    response_message->opcode = MESSAGE_T__OPCODE__OP_ERROR;
    response_message->c_type = MESSAGE_T__C_TYPE__CT_NONE;
    free_message(response_message);
    return -1;
  }else {
    printf("Tamanho da table: %d\n", response_message->data_size);
  }
  int size = response_message->data_size;
  free_message(response_message);
  return size;
}

char **rtable_get_keys(struct rtable_t *rtable){
  if(rtable == NULL) return NULL;
  MessageT request_messageT;
  message_t__init(&request_messageT); //inicializar a mensagem
  request_messageT.opcode = MESSAGE_T__OPCODE__OP_GETKEYS;  //tipo da operaçao
  request_messageT.c_type = MESSAGE_T__C_TYPE__CT_NONE;  //tipo de conteudo
  request_messageT.key = NULL;
  request_messageT.keys = NULL;
  request_messageT.n_keys = 0;
  request_messageT.data = NULL;
  request_messageT.data_size = 0;
  struct message_t * request_message = to_message(&request_messageT);
  if(request_message == NULL || request_messageT.n_keys < 0){
    return NULL;
  }
  struct message_t * response_message = network_send_receive(rtable, request_message);
  if(response_message == NULL){
    response_message->opcode = MESSAGE_T__OPCODE__OP_ERROR;
    response_message->c_type = MESSAGE_T__C_TYPE__CT_NONE;
    printf("\nNao existem keys na tabela!\n");
    free_message(response_message);
    return NULL;
  }
  if(response_message->n_keys == 0){
    printf("Nenhuma chave na tabela!\n");
    free_message(response_message);
    return NULL;
  } else {
    printf("Todas as keys foram geradas com sucesso!\n");
    char **keys_result = response_message->keys;
    free(response_message);
    return keys_result;
  }
}

void rtable_free_keys(char **keys){
  int count = 0;
  if (keys != NULL) {
    while (keys[count] != NULL) {
      free(keys[count]);
      count++;
    }
    free(keys);
  }
}

int rtable_verify(struct rtable_t *rtable, int op_n){
  if(rtable == NULL) return -1;
  MessageT request_messageT;
  message_t__init(&request_messageT); //inicializar a mensagem
  request_messageT.opcode = MESSAGE_T__OPCODE__OP_VERIFY;  //tipo da operaçao
  request_messageT.c_type = MESSAGE_T__C_TYPE__CT_NONE;  //tipo de conteudo
  request_messageT.key = NULL;
  request_messageT.keys = NULL;
  request_messageT.n_keys = 0;
  request_messageT.data = NULL;
  request_messageT.data_size = op_n;
  struct message_t * request_message = to_message(&request_messageT);
  if(request_messageT.data_size < 0 || request_message == NULL){
    free_message(request_message);
    message_t__free_unpacked(&request_messageT, NULL);
    return -1;
  }
  struct message_t *response_message;
  response_message = network_send_receive(rtable, request_message);
  if(response_message == NULL){
    response_message->opcode = MESSAGE_T__OPCODE__OP_ERROR;
    response_message->c_type = MESSAGE_T__C_TYPE__CT_NONE;
    free_message(response_message);
    return -1;
  }
  if(response_message->data_size == 0){
    printf("A operação %d já foi executada!\n", op_n);
    free_message(response_message);
    return 0;
  } else {
    printf("A operação %d não foi executada!\n", op_n);
    free_message(response_message);
    return -1;
  }
}
