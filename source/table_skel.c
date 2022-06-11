/*
Grupo 49
Guilherme Teixeira - 49021
Ruhan Azevedo - 51779
Gabriel Freitas - 51035
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include "table_skel.h"
#include "fifo.h"

#include "zookeeper/zookeeper.h"

struct table_t *table;
struct fifo *queue;
int last_assigned, op_count;
pthread_mutex_t queue_lock, table_lock;
pthread_cond_t cond;
pthread_t thread;
int flag;

#define ZDATALEN 1024 * 1024
static char *host_port;
static char *root_path = "/chain";
static zhandle_t *zh;
static int is_connected;

typedef struct String_vector zoo_string;
zoo_string* children_list;
char * maxID;
char * minID;
const char* zoo_root = "/";
int retval;
char* pathNode;

struct rtable_server{
  zhandle_t *zh;
  char* minID;
  char* maxID;
  char* next_server;
  int sockfd;
};

struct rtable_server* server;
struct message_t* messageServer;

/*funcao watcher(provavelmente vamos ter que usa la mais tarde)*/
//void my_watcher_func(zhandle_t *zzh, int type, int state, const char *path, void *watcherCtx) {}
void connection_watcher(zhandle_t *zzh, int type, int state, const char *path, void* context) {
  if (type == ZOO_SESSION_EVENT) {
    if (state == ZOO_CONNECTED_STATE) {
      is_connected = 1;
    } else {
      is_connected = 0;
    }
  }
}

int table_skel_init(const char* porta, int n_lists, const char* host_port){
  server = (struct rtable_server*) malloc(sizeof(struct rtable_server));
  /*Conectar ao servidor zookeeper*/
  server->zh = zookeeper_init(host_port, connection_watcher,	2000, 0, NULL, 0);
  if (server->zh == NULL)	{
    fprintf(stderr, "Error connecting to ZooKeeper server!\n");
    exit(EXIT_FAILURE);
  }
  if (server->zh == NULL) {
		fprintf(stderr,"Error connecting to ZooKeeper server[%d]!\n", errno);
		exit(EXIT_FAILURE);
	}
	sleep(3); /* Sleep a little for connection to complete */
  if (is_connected) {
		if (ZNONODE == zoo_exists(server->zh, root_path, 0, NULL)) {
      if (ZOK != zoo_create(server->zh, root_path, NULL, -1, & ZOO_OPEN_ACL_UNSAFE, 0, NULL, 0)) {
        fprintf(stderr, "Error creating znode from path %s!\n", root_path);
      	exit(EXIT_FAILURE);
      }
		}
    char node_path[120] = "";
    strcat(node_path,root_path);
    strcat(node_path,"/node");
    int new_path_len = 1024;
    char* new_path = malloc (new_path_len);

    char * hostname = (char *) malloc(20);
    char delim[] = ":";
    char *ptr = strtok((char*)host_port, delim);
    strcpy(hostname, ptr);
    char address[50];
    strcpy(address,hostname);
    strcat(address,":");
    strcat(address, porta);

    if (ZOK != zoo_create(server->zh, node_path, address, strlen(address), & ZOO_OPEN_ACL_UNSAFE, ZOO_EPHEMERAL | ZOO_SEQUENCE, new_path, new_path_len)) {
      fprintf(stderr, "Error creating znode from path %s!\n", node_path);
    	exit(EXIT_FAILURE);
    }
    pathNode = new_path;
    fprintf(stderr, "Ephemeral Sequencial ZNode created! ZNode path: %s\n", new_path);

    server->maxID = malloc(new_path_len);
    strcpy(server->maxID, new_path);

    children_list = (zoo_string *) malloc(sizeof(zoo_string));

    /* Get the list of children synchronously */
    retval = zoo_get_children(server->zh, root_path, 0, children_list);
    if (retval != ZOK)	{
      fprintf(stderr, "Error retrieving znode from path %s!\n", zoo_root);
      exit(EXIT_FAILURE);
    }
    fprintf(stderr, "\n=== znode listing === [ %s ]\n", root_path);

    char *id;
    char *pathMin;
    char *pathMax;
    int idMin = 99999999;
    int idMax = -1;
    for(int i = 0; i < children_list->count; i++){
      id = strtok(children_list->data[i], "node");
      fprintf(stderr, "(%d) %s\n", i+1 ,children_list->data[i]);
      if(idMin >= atoi(id)){
        idMin = atoi(id);
        pathMin = children_list->data[i];
      }
      if(idMax <= atoi(id)){
        idMax = atoi(id);
        pathMax = children_list->data[i];
      }
    }
    printf("=============================\n");
    printf("MAX PATH: %s\n", pathMax);
    printf("MIN PATH: %s\n", pathMin);

    char **pathsOrder = malloc(children_list->count * 15);
    char *prov;
    for(int i = 0; i < children_list->count; i++){
      pathsOrder[i] = children_list->data[i];
    }
    for(int i = 0; i < children_list->count; i++){
      for(int j = i+1; j < children_list->count; j++){
        if(atoi(strtok(pathsOrder[i], "node")) > atoi(strtok(pathsOrder[j], "node"))){
          prov = pathsOrder[i];
          pathsOrder[i] = pathsOrder[j];
          pathsOrder[j] = prov;
        }
      }
    }
    printf("Children nodes ordered:\n");
    for(int i = 0; i < children_list->count; i++){
      printf("%s\n", pathsOrder[i]);
    }
    server->next_server = NULL;

    int lenServer = ZDATALEN;
    char *serverIPPorto = (char *)malloc(lenServer * sizeof(char));
    if (ZOK != zoo_get(server->zh, pathNode, 0, serverIPPorto, &lenServer, NULL)){
      fprintf(stderr, "Error getting server!\n");
    }
    char * serverIP = (char *) malloc(20);
    char delimi[] = ":";
    char *ptrr = strtok((char*)serverIPPorto, delimi);
    strcpy(serverIP, ptrr);

    struct sockaddr_in sockett;

    if((server->sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){ // Cria socket TCP
      fprintf(stderr, "\nErro ao criar socket TCP!\n");
    }
    sockett.sin_family = AF_INET; // família de endereços
    sockett.sin_port = htons(atoi(porta)); // Porta TCP
    if (inet_pton(AF_INET, serverIP, &sockett.sin_addr) <= 0) { // Endereço IP
      fprintf(stderr, "\nInvalid address\n");
    }
    if(connect(server->sockfd, (struct sockaddr *)&sockett, sizeof(sockett)) < 0){
      fprintf(stderr, "\nConnection failed!\n");
    }
  }

  if(n_lists < 1){
    return -1;
  }
  table = table_create(n_lists);
  last_assigned = 0;
  op_count = 0;
  flag = 1;
  queue = fifo_create();

  if(pthread_mutex_init(&queue_lock, NULL) != 0){
    perror("Criacao da thread queue_lock");
    return -1;
  }
  if(pthread_mutex_init(&table_lock, NULL) != 0){
    perror("Criacao da thread table_lock");
    return -1;
  }
  if(pthread_cond_init(&cond, NULL) != 0){
    perror("Criacao da thread cond");
    return -1;
  }
  if(pthread_create(&thread, NULL, &process_task, NULL) != 0){
    perror("Criacao da thread");
    exit(EXIT_FAILURE);
    return -1;
  }
/*
  free(children_list->data);
  free(children_list);
  zookeeper_close(zh);
*/
  return 0;
}

void table_skel_destroy(){
  if(table != NULL){
    table_destroy(table);
  }
  flag = 0;
  pthread_cond_signal(&cond);
  if(pthread_join(thread, NULL) != 0){
    perror("\nadadasd");
    exit(EXIT_FAILURE);
  }
  fifo_delete(queue);
}

int verify(int op_n){
  struct task_t *atual = queue->head;
  while(atual != NULL){
    if(atual->op_n == op_n){
      return -1;
    }
    atual = atual->proximo;
  }
  if(op_n >= op_count || op_n < 0){
    return -1;
  }
  return 0;
}

int invoke(struct message_t *msg){
  //ERROR HANDELING//
  if(table == NULL || msg == NULL){
    return -1;
  }
  if(msg->opcode < 10 || msg->opcode > 60 || msg->c_type < 10){
    return -1;
  }
  messageServer = msg;
  //SIZE//
  if(msg->opcode == MESSAGE_T__OPCODE__OP_SIZE){
    msg->opcode = MESSAGE_T__OPCODE__OP_SIZE + 1;
    msg->c_type = MESSAGE_T__C_TYPE__CT_RESULT;
    msg->data_size = table_size(table);
    return 0;
  }
  //DEL//
  else if(msg->opcode == MESSAGE_T__OPCODE__OP_DEL){
    pthread_mutex_lock(&queue_lock);
    msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
    msg->opcode = MESSAGE_T__OPCODE__OP_DEL + 1;
    struct task_t *task = (struct task_t *) malloc(sizeof(struct task_t));
    task->op_n = last_assigned;
    task->op = 0;
    task->key = /*strdup*/msg->key;
    fifo_add(queue, task);
    msg->data_size = last_assigned;
    last_assigned++;
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&queue_lock);
    return 0;
  }
  //GET//
  else if(msg->opcode == MESSAGE_T__OPCODE__OP_GET){
    struct data_t *d = table_get(table, msg->key);
    msg->opcode = MESSAGE_T__OPCODE__OP_GET + 1;
    msg->c_type = MESSAGE_T__C_TYPE__CT_VALUE;
    if(d == NULL){
      msg->data_size = 0;
      msg->data = NULL;
    } else {
      msg->data_size = d->datasize;
      msg->data = d->data;
    }
    free(d);
    return 0;
  }
  //PUT//
  else if(msg->opcode == MESSAGE_T__OPCODE__OP_PUT){
    pthread_mutex_lock(&queue_lock);
    msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
    msg->opcode = MESSAGE_T__OPCODE__OP_PUT + 1;
    struct task_t *task = (struct task_t *) malloc(sizeof(struct task_t));
    task->op_n = last_assigned;
    task->op = 1;
    task->key = /*strdup*/msg->key;
    task->data = /*strdup*/msg->data;
    task->datasize = msg->data_size;
    fifo_add(queue, task);
    msg->data_size = last_assigned;
    last_assigned++;
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&queue_lock);
    return 0;
  }
  //GET KEYS//
  else if(msg->opcode == MESSAGE_T__OPCODE__OP_GETKEYS){
    msg->opcode =  MESSAGE_T__OPCODE__OP_GETKEYS + 1;
    msg->c_type = MESSAGE_T__C_TYPE__CT_KEYS;
    msg->n_keys = table_size(table);
    if(msg->n_keys == 0){
      msg->keys = NULL;
      return 0;
    }
    char** chaves = table_get_keys(table);
    msg->keys = malloc(table_size(table)*sizeof(char*));
    for(int i = 0; chaves[i] != NULL; i++){
      msg->keys[i] = strdup(chaves[i]);
    }
    table_free_keys(chaves);
    return 0;
  }
  //VERIFY//
  else if(msg->opcode == MESSAGE_T__OPCODE__OP_VERIFY){
    int verificado = verify(msg->data_size);
    msg->opcode = MESSAGE_T__OPCODE__OP_VERIFY + 1;
    msg->c_type = MESSAGE_T__C_TYPE__CT_RESULT;
    msg->data_size = verificado;
    return 0;
  }
  msg->opcode =  MESSAGE_T__OPCODE__OP_ERROR;
  msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
  return -1;
}

void sendMessage(){
  MessageT messageT = to_messageT(messageServer);
  int len = message_t__get_packed_size(&messageT);
  uint8_t *buf = malloc(len);
  if (buf == NULL){
    free_message(messageServer);
    free(buf);
    fprintf(stdout, "malloc error\n");
    return -1;
  }
  message_t__pack(&messageT, buf);

  int nbytes;

  if((nbytes = write_all(server->sockfd,(char*)&len,sizeof(len))) != sizeof(len)){
    free_message(messageServer);
    free(buf);
    perror("Erro ao enviar dados do servidor");
    close(server->sockfd);
    return -1;
  }

  if((nbytes = write_all(server->sockfd,(char*)buf,len)) != len){
    free_message(messageServer);
    free(buf);
    perror("Erro ao enviar dados do servidor");
    close(server->sockfd);
    return -1;
  }
  free(buf);
}

void * process_task (void *params){
  while(flag){
    pthread_mutex_lock(&queue_lock);

    if(queue->size == 0){
      pthread_cond_wait(&cond, &queue_lock);
    }
    if(queue->size > 0 && queue->head != NULL){
      struct task_t *task = pop(queue);
      if(task->op == 0){
        pthread_mutex_lock(&table_lock);
        table_del(table, task->key);
        pthread_mutex_unlock(&table_lock);
      }else {
        pthread_mutex_lock(&table_lock);
        struct data_t *d = data_create2(task->datasize, task->data);
        table_put(table, task->key, d);
        free(d);
        pthread_mutex_unlock(&table_lock);
      }
      op_count++;
      free(task);
    }
    pthread_mutex_unlock(&queue_lock);
  }
  sendMessage();
  return 0;
}
