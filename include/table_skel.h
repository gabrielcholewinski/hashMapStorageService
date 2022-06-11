/*
Grupo 49
Guilherme Teixeira - 49021
Ruhan Azevedo - 51779
Gabriel Freitas - 51035
*/

#ifndef _TABLE_SKEL_H
#define _TABLE_SKEL_H

#include "message-private.h"
#include "sdmessage.pb-c.h"
#include "table.h"

/* Inicia o skeleton da tabela.
 * O main() do servidor deve chamar esta função antes de poder usar a
 * função invoke(). O parâmetro n_lists define o número de listas a
 * serem usadas pela tabela mantida no servidor.
 * Retorna 0 (OK) ou -1 (erro, por exemplo OUT OF MEMORY)
 */
int table_skel_init(const char* porta, int n_lists, const char *host_port);

/* Liberta toda a memória e recursos alocados pela função table_skel_init.
 */
void table_skel_destroy();

/* Executa uma operação na tabela (indicada pelo opcode contido em msg)
 * e utiliza a mesma estrutura message_t para devolver o resultado.
 * Retorna 0 (OK) ou -1 (erro, por exemplo, tabela nao incializada)
*/
int invoke(struct message_t *msg);

void * process_task (void *params);

int verify(int op_n);

#endif
