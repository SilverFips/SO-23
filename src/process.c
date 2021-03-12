
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "main.h"
#include "client.h"
#include "server.h"
#include "proxy.h"


/* Função que inicia um processo através da função fork do SO. O novo
* processo pode ser um cliente, proxy, ou servidor, consoante a variável
* process_code tem valor 0, 1, ou 2 respectivamente, e irá executar a função
* execute_* respetiva, fazendo exit do retorno. O processo pai devolve o pid
* do processo criado.
*/
int launch_process(int process_id, int process_code, struct communication_buffers* buffers, struct main_data* data, struct semaphores* sems){

    int pid = fork();
    if(pid == 0){
        switch(process_code){
            case 0:
                exit(execute_client(process_id, buffers, data, sems));
                break;
            case 1:
                exit(execute_proxy(process_id, buffers, data, sems));
                break;
            case 2:
                exit(execute_server(process_id, buffers, data, sems));
                break;
            default:
                break;
        }
    }else{
        return pid;
    }
    return -1;
}


/* Função que espera que um processo termine através da função waitpid. 
* Devolve o retorno do processo, se este tiver terminado normalmente.
*/
int wait_process(int process_id){
    int result;
    waitpid(process_id, &result, WUNTRACED);
    if(WIFEXITED(result)){
        return WEXITSTATUS(result);
    }else{
        return -1;                          //Para quando o retorno/exit nao correu bem
    }
}

