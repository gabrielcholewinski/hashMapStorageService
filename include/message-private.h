/*
Grupo 49
Guilherme Teixeira - 49021
Ruhan Azevedo - 51779
Gabriel Freitas - 51035
*/

#ifndef _MESSAGE_PRIVATE_H
#define _MESSAGE_PRIVATE_H

#include "data.h"
#include "entry.h"

#include "sdmessage.pb-c.h"

struct message_t {
  ProtobufCMessage base;
  MessageT__Opcode opcode;
  MessageT__CType c_type;
  int32_t data_size;
  char *data;
  char *key;
  size_t n_keys;
  char **keys;
};

void print_message(struct message_t *msg);

int write_all(int sock, char *buf, int len);

int read_all(int sock, char *buf, int len);

struct _MessageT to_messageT(struct message_t *message);

struct message_t *to_message(struct _MessageT *messageT);

void free_message(struct message_t *message);

#endif
