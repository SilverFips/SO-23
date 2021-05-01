/**
 * Grupo: SO-023
 * Francisco Martins nº 51073
 * Filipe Pedroso nº 51958
 * Tiago Lourenço nº 46670
*/

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include "sosignal.h"
#include "memory.h"
#include "main.h"
#include "client.h"
#include "sotime.h"

#include <errno.h>
#include <unistd.h>


/* Função principal de um Cliente. Deve executar um ciclo infinito
* onde cada iteração do ciclo tem dois passos: primeiro, lê uma operação
* da main e se a mesma lida tiver id diferente de -1 e se data->terminate
* ainda for igual a 0, processa-a e escreve-a para os proxies; segundo,
* lê uma resposta dos servidores e se a mesma tiver id diferente de -1 e se
* data->terminate ainda for igual a 0, processa-a. Operações com id igual a
* -1 são ignoradas (op inválida) e se data->terminate for igual a 1 é porque
* foi dada ordem de terminação do programa, portanto deve-se fazer return do
* o número de operações processadas. Para efetuar estes passos, pode usar os
* outros métodos auxiliares definidos em client.h. 
*/

int execute_client(int client_id, struct communication_buffers* buffers, struct main_data* data, struct semaphores* sems){
    int* count = calloc(0,sizeof(int));
    struct operation* op = malloc(sizeof(struct operation));
    struct sigaction sa;
	sa.sa_handler = ctrlC_other;
	sa.sa_flags = SA_RESTART;
	sigemptyset(&sa.sa_mask);
    sigaddset(&sa.sa_mask, SIGALRM);
	
	if (sigaction(SIGINT, &sa, NULL) == -1) {
		perror("main:");
		exit(-1);
	}
    while(1){
        client_get_operation(op, buffers, data, sems);

        if((op->id != -1) && (*(data->terminate) == 0)){
            client_process_operation(op, client_id, count);
            client_send_operation(op, buffers, data, sems);
            
        }
       
        client_receive_answer(op, buffers, data,sems);

        if((op->id != -1) && (*(data->terminate) == 0)){
            client_process_answer(op, data, sems);

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


/* Função que lê uma operação do buffer de memória partilhada entre a 
* main e clientes, efetuando a necessária sincronização antes e depois
* de ler. Quando o processo acorda da sincronização, e antes de tentar ler
* a operação, deve verificar se data->terminate tem valor 1. Em caso 
* afirmativo, retorna imediatamente da função.
*/
void client_get_operation(struct operation* op, struct communication_buffers* buffers, struct main_data* data, struct semaphores* sems){
    consume_begin(sems->main_cli);
    if((*data->terminate) == 1){

        
        return;
    }
    
    semaphore_mutex_lock(sems->main_cli->mutex);
    read_rnd_access_buffer(buffers->main_cli, data->buffers_size, op);
    semaphore_mutex_unlock(sems->main_cli->mutex);
    consume_end(sems->main_cli);
    

}


/* Função que processa uma operação, alterando o seu campo cliente para o id
* passado como argumento, alterando o estado da mesma para 'C' (client), e 
* incrementando o contador de operações.
*/
void client_process_operation(struct operation* op, int cient_id, int* counter){
    
    sleep(3.3);
    op->status = 'C';
    op->client = cient_id;
    (*counter)++;
    op->client_time = getTime(op->client_time);
    
    
}


/* Função que escreve uma operação no buffer de memória partilhada entre
* clientes e proxies, efetuando a necessária sincronização antes e depois
* de escrever.
*/
void client_send_operation(struct operation* op, struct communication_buffers* buffers, struct main_data* data, struct semaphores* sems){
    produce_begin(sems->cli_prx);
    semaphore_mutex_lock(sems->cli_prx->mutex);
    write_circular_buffer(buffers->cli_prx, data->buffers_size, op);
    semaphore_mutex_unlock(sems->cli_prx->mutex);
    produce_end(sems->cli_prx);

}


/* Função que lê uma operação do buffer de memória partilhada entre 
* servidores e clientes, efetuando a necessária sincronização antes e
* depois de ler. Quando o processo acorda da sincronização, e antes de
* tentar ler a operação, deve verificar se data->terminate tem valor 1.
* Em caso afirmativo, retorna imediatamente da função.
*/
void client_receive_answer(struct operation* op, struct communication_buffers* buffers, struct main_data* data, struct semaphores* sems){
    consume_begin(sems->srv_cli);
     if((*data->terminate) == 1){
        return;
    }
    semaphore_mutex_lock(sems->srv_cli->mutex);
    read_circular_buffer(buffers->srv_cli, data->buffers_size, op);
    semaphore_mutex_unlock(sems->srv_cli->mutex);
    consume_end(sems->srv_cli);
}


/* Função que guarda uma operação no array de operações finalizadas da
* estrutura data, usando o id da mesma para indexar o array. O acesso à
* estrutura deve ser sincronizada através do semáforo sems->results_mutex.
* Imprime também uma mensagem para o terminal a avisar que a operação 
* terminou.
*/
void client_process_answer(struct operation* op, struct main_data* data, struct semaphores* sems){
    int id = op->id;
    
    semaphore_mutex_lock(sems->results_mutex);
    data->results[id].id = op->id;
    data->results[id].status = op->status;
    data->results[id].client = op->client;
    data->results[id].proxy = op->proxy;
    data->results[id].server = op->server;
    data->results[id].start_time = op->start_time;
    data->results[id].client_time = op->client_time;
    data->results[id].proxy_time = op->proxy_time;
    data->results[id].server_time = op->server_time;
    data->results[id].end_time = getTime(data->results[id].end_time);
    semaphore_mutex_unlock(sems->results_mutex);
    
    printf("-> A op #%d foi realizada com sucesso.\n", data->results[id].id);

}