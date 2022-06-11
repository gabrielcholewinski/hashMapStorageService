/*
	Programa cliente para manipular tabela de hash remota.

	Uso: table-client <ip servidor>:<porta servidor>
	Exemplo de uso: ./table_client 10.101.148.144:54321
*/


/*
Grupo 49
Guilherme Teixeira - 49021
Ruhan Azevedo - 51779
Gabriel Freitas - 51035
*/
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "sdmessage.pb-c.h"
#include "client_stub.h"
#include "zookeeper/zookeeper.h"

#define ZDATALEN 1024 * 1024
static zhandle_t *zh;
static int is_connected;
static char *root_path = "/chain";
typedef struct String_vector zoo_string;
zoo_string* children_list;
struct rtable_t *rtableHead;
struct rtable_t *rtableTail;
int idMin = 99999999;
int idMax = -1;
char *pathMax;
char *pathMin;
char *bufferMax;
char *bufferMin;
char *id;
char *watcher_client = "ZooKeeper Data Watcher";

void connection_watcher(zhandle_t *zzh, int type, int state, const char *path, void* context) {
	if (type == ZOO_SESSION_EVENT) {
		if (state == ZOO_CONNECTED_STATE) {
			is_connected = 1;
		} else {
			is_connected = 0;
		}
	}
}


static void child_watcher(zhandle_t *wzh, int type, int state, const char *zpath, void *watcher_ctx) {
	zoo_string* children_list =	(zoo_string *) malloc(sizeof(zoo_string));
	int zoo_data_len = ZDATALEN;
	if (state == ZOO_CONNECTED_STATE)	 {
		if (type == ZOO_CHILD_EVENT) {
			if (ZOK != zoo_wget_children(zh, "/chain", &child_watcher, watcher_client, children_list)) {
 				fprintf(stderr, "Error setting watch!\n");
 			}
			//PEGAR O MAIOR E MENOR ID E SEUS PATHS
			for(int i = 0; i < children_list->count; i++){
				memcpy(id, children_list->data[i][4], 3);
				if(idMin >= atoi(id)){
					idMin = atoi(id);
					pathMin = children_list->data[i];
				}
				if(idMax <= atoi(id)){
					idMax = atoi(id);
					pathMax = children_list->data[i];
				}
			}
			 //FAZER GETS DOS SERVIDORES
			if (ZOK != zoo_get(zh, pathMin, 0, bufferMin, sizeof(bufferMin), NULL)){
				fprintf(stderr, "Error getting min server!\n");
			}
			if (ZOK != zoo_get(zh, pathMax, 0, bufferMax, sizeof(bufferMax), NULL)){
				fprintf(stderr, "Error getting max server!\n");
			}
			//FAZER CONNECT DOS SERVERS TAIL E HEAD
			if((rtableHead = rtable_connect(bufferMin)) == NULL){
		    //return -1;
		  }
			if((rtableTail = rtable_connect(bufferMax)) == NULL){
		    //return -1;
			}
		 }
	 }
	 free(children_list);
}


int main(int argc, char **argv){
	char *s;
	char *token;
	bufferMax = (char *)malloc(ZDATALEN * sizeof(char));
	bufferMin = (char *)malloc(ZDATALEN * sizeof(char));

	/* Testar os argumentos de entrada */
	if(argc != 2){
		printf("Uso: table-client <ip servidor>:<porta servidor>\n");
		printf("Exemplo de uso: ./binary/table_client 127.0.0.1:54321\n");
		return -1;
	}
	//CONECTRA AO ZOOKEEPER
	zh = zookeeper_init(argv[1], connection_watcher,	2000, 0, NULL, 0);
	if (zh == NULL)	{
		fprintf(stderr, "Error connecting to ZooKeeper server!\n");
		exit(EXIT_FAILURE);
	}
	//PEGAR OS CHILDRENS
	children_list =	(zoo_string *) malloc(sizeof(zoo_string));
	if (ZOK != zoo_wget_children(zh, "/chain", &child_watcher, watcher_client, children_list)) {
		fprintf(stderr, "Error setting watch!\n");
	}

	/* Iniciar instância do stub e Usar rtable_connect para estabelcer ligação ao servidor*/
	/*
	if((rtable = rtable_connect(argv[1])) == NULL){
    return -1;
  }
	*/
	token = "";
	s = (char*) malloc(2048);
	/* Fazer ciclo até que o utilizador resolva fazer "quit" */
 	while (strcmp(token, "quit") != 0){
		printf(">>> "); // Mostrar a prompt para inserção de comando

		/* Receber o comando introduzido pelo utilizador
		   Sugestão: usar fgets de stdio.h
		   Quando pressionamos enter para finalizar a entrada no
		   comando fgets, o carater \n é incluido antes do \0.
		   Convém retirar o \n substituindo-o por \0.
		*/

		fgets(s, 2048, stdin);
        s[strlen(s) - 1] = '\0';

		if (strcmp(s, "") == 0){
      printf("Escreva um comando valido:\n\nsize\ndel <key>\nget <key>\nput <key> <data>\ngetkeys\nquit\n");
			continue;
		}

		/* Verificar se o comando foi "quit". Em caso afirmativo
		   não há mais nada a fazer a não ser terminar decentemente.
		    Caso contrário:
			Verificar qual o comando;
			chamar função do stub para processar o comando e receber msg_resposta
		*/
		token = strtok(s, " ");

		if(strcmp(token, "quit") == 0)
		  continue;

    if(strcmp(token, "size") == 0){
	     printf("\nNumero de elementos na tabela: %d\n",rtable_size(rtableTail));
		   continue;
		}

    if(strcmp(token, "getkeys") == 0){
	    char** keys = rtable_get_keys(rtableTail);
			if(keys != NULL){
				for(int i = 0; keys[i] != NULL; i++){
					printf("Chave %d = %s\n", i, keys[i]);
				}
				rtable_free_keys(keys);
			}
		  continue;
		}

	  if(strcmp(token, "get") == 0){
	    token = strtok(NULL, " ");
			if(token == NULL){
				printf("Escreva um comando valido:\nget <key>\n");
				token = "";
				continue;
			}
			//copia-se o valor de token para key
	    char *key = strdup(token);
			struct data_t * data = rtable_get(rtableTail, key);
			//liberta-se a key
			free(key);
			if(strcmp(data->data, "") == 0){
				free(data);
			}else{
				data_destroy(data);
			}
			token = "";
			continue;
		}

		if(strcmp(token, "del") == 0){
	    token = strtok(NULL, " ");
			if(token == NULL){
				printf("Escreva um comando valido:\ndel <key>\n");
				token = "";
				continue;
			}
	    char *key = strdup(token);
		  rtable_del(rtableHead, key);
			free(key);
			continue;
		}

		if(strcmp(token, "put") == 0){
		  token = strtok(NULL, " ");
			if(token == NULL){
				printf("Escreva um comando valido:\nput <key> <data>\n");
				token = "";
				continue;
			}
			char *key = strdup(token);
		  token = strtok(NULL, " ");
			//Caso o prompt nao venha com o numero de argumentos correto
			if(token == NULL){
				printf("Escreva um comando valido:\nput <key> <data>\n");
				token = "";
			  free(key);
				continue;
			} else {
				//cria-se uma nova data
				struct data_t *data = data_create2(strlen(token),token);
				//cria-se uma nova entry
				struct entry_t *entry = entry_create(key,data);
				//faz-se put da entry
				rtable_put(rtableHead, entry);
				//libertam-se os campos alocados
				free(data);
			  free(entry);
			  free(key);
				continue;
			}
		}

		if(strcmp(token, "verify") == 0){
			token = strtok(NULL, " ");
			if(token == NULL){
				printf("Escreva um comando valido:\nverify <op>\n");
				token = "";
				continue;
			}
			int op = atoi(token);
			rtable_verify(rtableTail, op);
			continue;
		}

		printf("Escreva um comando valido:\n\nsize\ndel <key>\nget <key>\nput <key> <data>\ngetkeys\nverify <op>\nquit\n");
	}
	//QUIT
	free(s);
	printf("A desligar...\n");
	return rtable_disconnect(rtableHead) && rtable_disconnect(rtableTail);
	//avisa o server que fez quit
}
