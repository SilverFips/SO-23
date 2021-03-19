
#include <stdio.h>
#include "memory.h"
#include "main.h"

/* Função principal de um Proxy. Deve executar um ciclo infinito onde em 
* cada iteração do ciclo lê uma operação dos clientes e se a mesma tiver id 
* diferente de -1 e se data->terminate ainda for igual a 0, processa-a e
* escreve-a para os servidores. Operações com id igual a -1 são ignoradas
* (op inválida) e se data->terminate for igual a 1 é porque foi dada ordem
* de terminação do programa, portanto deve-se fazer return do número de 
* operações processadas. Para efetuar estes passos, pode usar os outros
* métodos auxiliares definidos em proxy.h.
*/
int execute_proxy(int proxy_id, struct communication_buffers* buffers, struct main_data* data, struct semaphores* sems){
    while(true){
        //read_circular_buffer(buffers->cli_prx, data->buffers_size, buffers->cli_prx->op);
        //le op cliente
        //op->id !=-1 && data->terminate == 0
        //processa-a e escreve para servidor
        //else op-> id == -1 ignore
        // data->terminate == 1 -> terminar programa-> return numero de ops 
    }


    return 0;
}


/* Função que lê uma operação do buffer de memória partilhada entre
* clientes e proxies, efetuando a necessária sincronização antes e
* depois de ler. Quando o processo acorda da sincronização, e antes de
* tentar ler a operação, deve verificar se data->terminate tem valor 1.
* Em caso afirmativo, retorna imediatamente da função.
*/
void proxy_receive_operation(struct operation* op, struct communication_buffers* buffers, struct main_data* data, struct semaphores* sems){
    consume_begin(sems->cli_prx);
    semaphore_mutex_lock(sems->cli_prx->mutex);
    read_circular_buffer(buffers->cli_prx, data->buffers_size, op);
    semaphore_mutex_unlock(sems->cli_prx->mutex);
    consume_end(sems->cli_prx);

}
    

/* Função que processa uma operação, alterando o seu campo proxy para o id
* passado como argumento, alterando o estado da mesma para 'P' (proxied), e 
* incrementando o contador de operações.
*/
void proxy_process_operation(struct operation* op, int server_id, int* counter){
    op->proxy = server_id;
    op->status = 'P';
    counter++;
}
    

/* Função que escreve uma operação no buffer de memória partilhada entre
* proxies e servidores, efetuando a necessária sincronização antes e
* depois de escrever.
*/
void proxy_forward_operation(struct operation* op, struct communication_buffers* buffers, struct main_data* data, struct semaphores* sems){
    produce_begin(sems->prx_srv);
    semaphore_mutex_lock(sems->prx_srv->mutex);
    write_rnd_access_buffer(buffers->prx_srv, data->buffers_size, op);
    semaphore_mutex_unlock(sems->prx_srv->mutex);
    produce_end(sems->prx_srv);
}

