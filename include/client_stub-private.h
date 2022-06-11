/*
Grupo 49
Guilherme Teixeira - 49021
Ruhan Azevedo - 51779
Gabriel Freitas - 51035
*/

#ifndef _CLIENT_STUB_PRIVATE_H
#define _CLIENT_STUB_PRIVATE_H

#include "data.h"
#include "entry.h"
#include <sys/socket.h>
#include <netinet/in.h>

struct rtable_t{
  struct sockaddr_in server;
  int sockfd;
};

#endif
