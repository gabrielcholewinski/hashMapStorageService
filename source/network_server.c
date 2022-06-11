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

#include <signal.h>

#include <poll.h>
#include <fcntl.h>

#include "network_server.h"
#include "message-private.h"

#define NFDESC 20
//Numero de sockets (uma para listening)
#define TIMEOUT 50
//em milisegundos
#define MAX_MSG 2048


struct pollfd connections[NFDESC];
int wel_sockfd; // file descriptor para a welcoming socket
int nfds, kfds, i;

int sockfd, connsockfd;
struct sockaddr_in server, client;
int nbytes, count, listensock, option;
socklen_t size_client;

int network_server_init(short port){
  if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("Erro ao criar socket\n");
    return -1;
  }

  server.sin_family = AF_INET;
  server.sin_port = htons(port);
  server.sin_addr.s_addr = htonl(INADDR_ANY);

  if(setsockopt(sockfd, SOL_SOCKET,(SO_REUSEPORT | SO_REUSEADDR), (char*)&option, sizeof(option)) < 0){
    printf("setsockopt failed\n");
    close(sockfd);
    return -1;
  }

  if(bind(sockfd, (struct sockaddr *) &server, sizeof(server)) < 0){
    perror("Erro ao fazer bind\n");
    close(sockfd);
    return -1;
  }

  if(listen(sockfd, 0) < 0){
    perror("Erro ao executar listen\n");
    close(sockfd);
    return -1;
  }

  printf("Servidor 'a espera de dados\n");
  return sockfd;
}

static volatile int interrupt = 1;

void INTHandler(int sig){
  interrupt = 0;
}

int network_main_loop(int listening_socket){
  signal(SIGPIPE, SIG_IGN);
  signal(SIGQUIT, INTHandler);
  signal(SIGINT, INTHandler);
  signal(SIGTSTP, INTHandler);

  size_client = sizeof(struct sockaddr);
  struct message_t *msg;

  for(i = 0; i < NFDESC; i++){
    connections[i].fd = -1; //poll ignora estruturas com fd < 0
  }
  connections[0].fd = listening_socket;  //Vamos detetar eventos na welcoming socket
  connections[0].events = POLLIN; //Vamos esperar ligacoes nesta socket

  nfds = 1; // numero de file descriptors

  while(interrupt){
    // Retorna assim que exista um evento ou que TIMEOUT expire. * FUNCAO POLL *.
    while((kfds = poll(connections, nfds, 10)) >= 0){ // kfds == 0 significa timeout sem eventos
      if(kfds > 0){ // kfds eh o numero de descritores com evento ou erro
        if((connections[0].revents & POLLIN) && (nfds < NFDESC))  // Pedido na listening socket ?
        if((connections[nfds].fd = accept(connections[0].fd, (struct sockaddr *) &client, &size_client)) > 0){ // Liga��o feita ?
          connections[nfds].events = POLLIN; // Vamos esperar dados nesta socket
          nfds++;
        }
      }
      for(i = 1; i < nfds; i++){
        if(connections[i].revents & POLLIN){
          printf("O servidor esta à escuta!!!\n");
          msg = network_receive(connections[i].fd);
          if(msg == NULL || msg->opcode == MESSAGE_T__OPCODE__OP_BAD || msg->c_type == MESSAGE_T__C_TYPE__CT_BAD){
            free_message(msg);
            close(connections[i].fd);
            connections[i].fd = -1;
            for(int j = i; j < nfds; j++){
              connections[j] = connections[j+1];
            }
            nfds--;
            i--;
            continue;
          } else {
            printf("Menssagem recebida!\n");
            print_message(msg);
            if(invoke(msg) != 0){
              free_message(msg);
              perror("Erro no invoke");
              close(connections[i].fd);
              connections[i].fd = -1;
              continue;
            }
            print_message(msg);
            network_send(connections[i].fd, msg);
            printf("Mensagem enviada!\n");
            free_message(msg); //adicionei este free_message
          }
        }
        if(connections[i].revents & POLLHUP){
          close(connections[i].fd);
          connections[i].fd = -1;
          for(int j = i; j < nfds; j++){
            connections[j] = connections[j+1];
          }
          nfds--;
        }
      }
    }
  }
  printf("\nDesligando..\n");
  close(listening_socket);
  return 0;
}

int network_send(int client_socket, struct message_t *message){
  if(message == NULL || client_socket < 0) return -1;
  MessageT messageT = to_messageT(message);
  int len = message_t__get_packed_size(&messageT);
  uint8_t *buf = malloc(len);
  if (buf == NULL){
    free_message(message);
    free(buf);
    fprintf(stdout, "malloc error\n");
    return -1;
  }
  message_t__pack(&messageT, buf);

  if((nbytes = write_all(client_socket,(char*)&len,sizeof(len))) != sizeof(len)){
    free_message(message);
    free(buf);
    perror("Erro ao enviar dados do servidor");
    close(client_socket);
    return -1;
  }

  if((nbytes = write_all(client_socket,(char*)buf,len)) != len){
    free_message(message);
    free(buf);
    perror("Erro ao enviar dados do servidor");
    close(client_socket);
    return -1;
  }
  free(buf);
  return nbytes;
}

struct message_t *network_receive(int client_socket){
  if(client_socket < 0 ) return NULL;
  int read_len;
  if((nbytes = read_all(client_socket,(char*)&read_len,sizeof(read_len))) < 0){
    perror("Erro ao receber dados do cliente");
    close(client_socket);
    return NULL;
  }

  uint8_t *buf = malloc(read_len);

  if((nbytes = read_all(client_socket,(char*)buf,read_len)) < 0){
    perror("Erro ao receber dados do cliente");
    close(client_socket);
    return NULL;
  }
  MessageT *recv_msg;
  recv_msg = message_t__unpack(NULL, nbytes, buf);
  if (recv_msg == NULL) {
    fprintf(stdout, "error unpacking message\n");
    return NULL;
  }
  struct message_t *m = to_message(recv_msg);
  if(m == NULL){
    free(buf);
    free_message(m);
    return NULL;
  }
  message_t__free_unpacked(recv_msg, NULL);
  free(buf);
  return m;
}

int network_server_close(){
  table_skel_destroy();
  return close(listensock);
}
