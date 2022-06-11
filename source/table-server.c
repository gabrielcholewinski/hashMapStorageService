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

#include "network_server.h"
#include "table_skel.h"
#include "zookeeper/zookeeper.h"

/*
   Programa que implementa um servidor de uma tabela hash com chainning.
   Uso: table-server <port> <n_lists>
   Exemplo de uso: ./table_server 54321 6
*/
int main(int argc, char **argv){
	int listening_socket;

	//Sao testados os argumentos de entrada
	if(argc != 4){
		printf("Uso: table-server <port> <n_lists> <IP>:<porta>\n");
		printf("Exemplo de uso: ./table_server 8080 1 127.0.0.1:2181\n");
		return -1;
	}

	//inicialização da camada de rede
	listening_socket = network_server_init(atoi(argv[1]));

	table_skel_init(argv[1], atoi(argv[2]), argv[3]);

	int result = network_main_loop(listening_socket);

	//if(result == 0)// exit(0); //servidor lança signal
		/*atexit(*/network_server_close()/*)*/; //assim que o network_server_close desligar, o terminal encerra

	return result;
}
