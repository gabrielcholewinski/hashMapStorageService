/*
Grupo 49
Guilherme Teixeira - 49021
Ruhan Azevedo - 51779
Gabriel Freitas - 51035
*/
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include "errno.h"

#include "message-private.h"
#include "sdmessage.pb-c.h"

void print_message(struct message_t *msg){
  printf("\n=========MESSAGE=========\n");
  printf("opcode: %d, c_type: %d\n", msg->opcode, msg->c_type);
  if(msg->c_type == MESSAGE_T__C_TYPE__CT_KEY){
    printf("key: %s\n", msg->key);
  } else if(msg->opcode ==  MESSAGE_T__OPCODE__OP_VERIFY+1 || msg->opcode ==  MESSAGE_T__OPCODE__OP_VERIFY){
    printf("%d\n", (int)msg->data_size);
  } else if(msg->c_type == MESSAGE_T__C_TYPE__CT_VALUE){
    printf("datasize: %d, value: %s\n", msg->data_size, msg->data);
  } else if(msg->c_type == MESSAGE_T__C_TYPE__CT_ENTRY){
    printf("key: %s\n", msg->key);
    printf("datasize: %d, value: %s\n", msg->data_size, msg->data);
  } else if(msg->c_type == MESSAGE_T__C_TYPE__CT_KEYS){
    if(msg->keys != NULL){
      for(int i = 0; i < msg->n_keys; i++){
        printf("key%d: %s\n", i, msg->keys[i]);
      }
    }
  } else if(msg->c_type == MESSAGE_T__C_TYPE__CT_RESULT){
    printf("size: %d\n", (int)msg->data_size);
  } else {
    printf("opNum: %d\n", (int)msg->data_size);
  }
  printf("=========================\n");
}

int write_all(int sock, char *buf, int len) {
  int bufsize = len;
  while(len > 0) {
  int res = write(sock, buf, len);
    if(res<0) {
      if(errno==EINTR) continue;
        perror("write failed:");
        return res;
      }
      buf += res;
      len -= res;
  }
  return bufsize;
}

int read_all(int sock, char *buf, int len) {
  int bufsize = len;
  while(len > 0) {
    int res = read(sock, buf, len);
    if(res<0) {
      if(errno==EINTR) continue;
        perror("write failed:");
        return res;
      }
      buf += res;
      len -= res;
      if(res == 0){
        buf = 0;
      }
  }
  return bufsize;
}

struct _MessageT to_messageT(struct message_t *message){
  struct _MessageT messageT;
  message_t__init(&messageT);
  messageT.base = message->base;
  messageT.opcode = message->opcode;
  messageT.c_type = message->c_type;
  messageT.data_size = message->data_size;
  messageT.data = message->data;
  messageT.key = message->key;
  messageT.n_keys = message->n_keys;
  messageT.keys = message->keys;
  if(message->keys != NULL){
    for(int i = 0; i < message->n_keys; i++){
      messageT.keys[i] = strdup(message->keys[i]);
    }
  }

  return messageT;
}

struct message_t *to_message(struct _MessageT *messageT){
  if(messageT == NULL) return NULL;
  struct message_t * message = (struct message_t*)malloc(sizeof(struct message_t));
  if(message == NULL) return NULL;
  message->base = messageT->base;
  message->opcode = messageT->opcode;
  message->c_type = messageT->c_type;
  message->data_size = messageT->data_size;
  message->data = NULL;
  if(messageT->data != NULL && strcmp(messageT->data, "") != 0) message->data = strdup(messageT->data);
  message->key = NULL;
  if(messageT->key != NULL && strcmp(messageT->key, "") != 0) message->key = strdup(messageT->key);
  message->n_keys = messageT->n_keys;
  if(message->n_keys == 0) message->keys = NULL;
  if(messageT->keys != NULL){
    message->keys = (char **) malloc((message->n_keys+1) * sizeof(char*));
    for(int i = 0; i < message->n_keys; i++){
      message->keys[i] = strdup(messageT->keys[i]);
    }
    message->keys[message->n_keys] = NULL;
  }
  return message;
}

void free_message(struct message_t *message){
  if(message == NULL) return;
  if(message->data != NULL){
    free(message->data);
  }
  if(message->key != NULL){
    free(message->key);
  }
  if(message->n_keys != 0){
    if(message->keys != NULL){
      for(int i = 0; i < message->n_keys; i++) {
        free(message->keys[i]);
      }
      free(message->keys);
    }
  }
  free(message);
}
