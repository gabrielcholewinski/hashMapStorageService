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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>

#include "client_stub.h"
#include "client_stub-private.h"
#include "sdmessage.pb-c.h"
#include "message-private.h"
#include "network_client.h"

#define MAX_MSG 2048

int network_connect(struct rtable_t *rtable){
  if(rtable == NULL) return -1;
  int sockfd = rtable->sockfd;
  printf("Conecta-se ao socket!!!\n");
  if(connect(sockfd,(struct sockaddr *)&rtable->server, sizeof(rtable->server)) < 0){
    perror("Erro ao conectar-se ao servidor!");
    close(sockfd);
    return -1;
  }
  return 0;
}

struct message_t *network_send_receive(struct rtable_t * rtable, struct message_t *msg){
  if(msg == NULL || rtable == NULL){
    free_message(msg);
    free(rtable);
    return NULL;
  }
  int sockfd = rtable->sockfd;
  unsigned len;
  MessageT messageT = to_messageT(msg);
  len = message_t__get_packed_size(&messageT);
  uint8_t *buf = malloc(len);
  if (buf == NULL){
    free_message(msg);
    free(buf);
    fprintf(stdout, "malloc error\n");
    return NULL;
  }
  message_t__pack(&messageT, buf);

  int nbytes;
  if((nbytes = write_all(sockfd, (char*)&len,sizeof(len))) != sizeof(len)){
    free_message(msg);
    free(buf);
    perror("Erro ao enviar dados ao servidor");
    close(sockfd);
    return NULL;
  }

  if((nbytes = write_all(sockfd,(char*)buf,len)) != len){
    free_message(msg);
    free(buf);
    perror("Erro ao enviar dados ao servidor");
    close(sockfd);
    return NULL;
  }

  printf("'A espera de resposta do servidor ...\n");

  int read_len;
  uint8_t *buffer;

  if((nbytes = read_all(sockfd, (char*)&read_len,sizeof(read_len))) != sizeof(read_len)){
    free_message(msg);
    free(buf);
    perror("Erro ao receber dados do servidor");
    close(sockfd);
  }

  buffer = malloc(read_len);

  if((nbytes = read_all(sockfd,(char*)buffer,read_len)) != read_len){
    free_message(msg);
    free(buf);
    free(buffer);
    perror("Erro ao receber dados do servidor");
    close(sockfd);
  }

  MessageT *recv_msg;

  recv_msg = message_t__unpack(NULL, read_len, buffer);
  if (recv_msg == NULL) {
    free_message(msg);
    free(buf);
    free(buffer);
    free(recv_msg);
    fprintf(stdout, "error unpacking message\n");
    return NULL;
  }
  struct message_t *m;
  m = to_message(recv_msg);
  message_t__free_unpacked(recv_msg, NULL);
  if(m == NULL){
    free_message(msg);
    free(buf);
    free(buffer);
    free_message(m);
    return NULL;
  }


  free_message(msg);
  free(buf);
  free(buffer);
  print_message(m);
  return m;
}

int network_close(struct rtable_t * rtable){
  if(rtable == NULL) return -1;
  close(rtable->sockfd);
  return 0;
}
