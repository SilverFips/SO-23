
#include <stdio.h>
#include <stdlib.h>
#include "memory.h"
#include "main.h"
#include "server.h"

/* Função principal de um Servidor. Deve executar um ciclo infinito onde em 
* cada iteração do ciclo lê uma operação dos proxies e se a mesma tiver id 
* diferente de -1 e se data->terminate ainda for igual a 0, processa-a e
* escreve a resposta para os clientes. Operações com id igual a -1 são 
* ignoradas (op inválida) e se data->terminate for igual a 1 é porque foi 
* dada ordem de terminação do programa, portanto deve-se fazer return do
* número de operações processadas. Para efetuar estes passos, pode usar os
* outros métodos auxiliares definidos em server.h.
*/
int execute_server(int server_id, struct communication_buffers* buffers, struct main_data* data, struct semaphores* sems){
    int* count = malloc(sizeof(int));
    struct operation* op = malloc(sizeof(struct operation));
    while(1){
        server_receive_operation(op, buffers, data, sems);

        if((op->id != -1) && (*(data->terminate) == 0)){
            server_process_operation(op, server_id, count);
            server_send_answer(op, buffers,data,sems);
            
        }
         if(*(data->terminate) == 1){
            break;
        }

    }
    free(op);
    int i = (*count);
    free(count);

    return i;
}


/* Função que lê uma operação do buffer de memória partilhada entre
* proxies e servidores, efetuando a necessária sincronização antes e
* depois de ler. Quando o processo acorda da sincronização, e antes de
* tentar ler a operação, deve verificar se data->terminate tem valor 1.
* Em caso afirmativo, retorna imediatamente da função.
*/
void server_receive_operation(struct operation* op, struct communication_buffers* buffers, struct main_data* data, struct semaphores* sems){
    consume_begin(sems->srv_cli);
    if((*data->terminate) == 1){
        return;
    }
    semaphore_mutex_lock(sems->srv_cli->mutex);
    read_circular_buffer(buffers->srv_cli, data->buffers_size, op);
    semaphore_mutex_unlock(sems->srv_cli->mutex);
    consume_end(sems->srv_cli);
}

/* Função que processa uma operação, alterando o seu campo server para o id
* passado como argumento, alterando o estado da mesma para 'S' (served), e 
* incrementando o contador de operações.
*/
void server_process_operation(struct operation* op, int proxy_id, int* counter){
    op->server = proxy_id;
    op->status= 'S';
    (*counter)++;
}


/* Função que escreve uma operação no buffer de memória partilhada entre
* servidores e clientes, efetuando a necessária sincronização antes e
* depois de escrever.
*/
void server_send_answer(struct operation* op, struct communication_buffers* buffers, struct main_data* data, struct semaphores* sems){
    produce_begin(sems->srv_cli);
    semaphore_mutex_lock(sems->srv_cli->mutex);
    write_circular_buffer(buffers->srv_cli, data->buffers_size, op);
    semaphore_mutex_unlock(sems->srv_cli->mutex);
    produce_end(sems->srv_cli);
}