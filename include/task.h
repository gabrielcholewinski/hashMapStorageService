/*
Grupo 49
Guilherme Teixeira - 49021
Ruhan Azevedo - 51779
Gabriel Freitas - 51035
*/

struct task_t {
  int op_n; //o número da operação
  int op; //a operação a executar. op=0 se for um delete, op=1 se for um put
  char* key; //a chave a remover ou adicionar
  char* data; // os dados a adicionar em caso de put, ou NULL em caso de delete
  struct task_t* proximo;
  int datasize;
};
