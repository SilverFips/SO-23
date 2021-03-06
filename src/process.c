

#include "memory.h"
#include "main.h"

/* Função que inicia um processo através da função fork do SO. O novo
* processo pode ser um cliente, proxy, ou servidor, consoante a variável
* process_code tem valor 0, 1, ou 2 respectivamente, e irá executar a função
* execute_* respetiva, fazendo exit do retorno. O processo pai devolve o pid
* do processo criado.
*/
int launch_process(int process_id, int process_code, struct communication_buffers* buffers, struct main_data* data, struct semaphores* sems){

    int fork = fork();

    if(fork == 0){
        switch(process_code){
            case 0:
                int retur = execute_client(process_id, buffers, data, sems);
                exit(retur);
                break;
            case 1:
                int retur = execute_proxies(process_id, buffers, data, sems);
                exit(retur);
                break;
            case 2:
                int retur = execute_server(process_id, buffers, data, sems);
                exit(retur);
                break;
            default:
                break;
        }
        return getpid();
    }
}


/* Função que espera que um processo termine através da função waitpid. 
* Devolve o retorno do processo, se este tiver terminado normalmente.
*/
int wait_process(int process_id){
    int result;
    waitpid(process_id, &result);
    if(WIFEXITED(result)){
        return WEXITSTATUS(result);
    }else{
        return -1;                          //Para quando o retorno/exit nao correu bem
    }
}

