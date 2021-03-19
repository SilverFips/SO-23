#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include "main.h"
#include "memory.h"
#include "memory-private.h"
#include "synchronization.h"
#include "process.h"


/* Função que reserva a memória dinâmica necessária para a execução
* do sovaccines, nomeadamente para os arrays *_pids e *_stats da estrutura 
* main_data. Para tal, pode ser usada a função create_dynamic_memory do memory.h.
*/
void create_dynamic_memory_buffers(struct main_data* data) {
	int clientes = data->n_clients;
	int proxies = data->n_proxies;
	int servers = data->n_servers;

	data->client_pids = create_dynamic_memory(clientes * sizeof(int));
	data->proxy_pids = create_dynamic_memory(proxies * sizeof(int));
	data->server_pids = create_dynamic_memory(servers * sizeof(int));

	data->client_stats = create_dynamic_memory(clientes * sizeof(int));
	data->proxy_stats = create_dynamic_memory(proxies * sizeof(int));
	data->server_stats = create_dynamic_memory(servers * sizeof(int));

}

/* Função que reserva a memória partilhada necessária para a execução do
* sovaccines. É necessário reservar memória partilhada para todos os buffers da
* estrutura communication_buffers, incluindo os buffers em si e respetivos
* pointers, assim como para o array data->results e variável data->terminate.
* Para tal, pode ser usada a função create_shared_memory do memory.h.
*/
void create_shared_memory_buffers(struct main_data* data, struct communication_buffers* buffers) {
	
	buffers->main_cli->ptr = create_shared_memory(STR_SHM_MAIN_CLI_PTR, (data->buffers_size)*sizeof(int));
	buffers->main_cli->op = create_shared_memory(STR_SHM_MAIN_CLI_BUFFER, (data->buffers_size)*sizeof(struct operation));
	
	buffers->cli_prx->ptr = create_shared_memory(STR_SHM_CLI_PRX_PTR , sizeof(struct pointer));
	buffers->cli_prx->op = create_shared_memory(STR_SHM_CLI_PRX_BUFFER, (data->buffers_size)*sizeof(struct operation));

	buffers->prx_srv->ptr = create_shared_memory(STR_SHM_PRX_SRV_PTR , (data->buffers_size)*sizeof(int));
	buffers->prx_srv->op = create_shared_memory(STR_SHM_PRX_SRV_BUFFER, (data->buffers_size)*sizeof(struct operation));

	buffers->srv_cli->ptr = create_shared_memory(STR_SHM_SRV_CLI_PTR , sizeof(struct pointer));
	buffers->srv_cli->op = create_shared_memory(STR_SHM_SRV_CLI_BUFFER, (data->buffers_size)*sizeof(struct operation));
		
	data->results = create_shared_memory( STR_SHM_RESULTS	, (data->max_ops)*sizeof(struct operation));
	
	data->terminate = create_shared_memory(STR_SHM_TERMINATE, sizeof(int));
	*(data->terminate) = 0;

}

/* Função que inicializa os semáforos da estrutura semaphores. Semáforos
* *_full devem ser inicializados com valor 0, semáforos *_empty com valor
* igual ao tamanho dos buffers de memória partilhada, e os *_mutex com valor
* igual a 1. Para tal pode ser usada a função semaphore_create.
*/
void create_semaphores(struct main_data* data, struct semaphores* sems){		//NAO ESTA ACABADO
	sems->main_cli->full = semaphore_create(STR_SEM_MAIN_CLI_FULL, 0);
	sems->main_cli->empty = semaphore_create(STR_SEM_MAIN_CLI_EMPTY , data->buffers_size);		//NAO SEI SE ESTA BEM O VALOR
	sems->main_cli->mutex = semaphore_create(STR_SEM_MAIN_CLI_MUTEX, 1);

	sems->cli_prx->full = semaphore_create(STR_SEM_CLI_PRX_FULL, 0);
	sems->cli_prx->empty = semaphore_create(STR_SEM_CLI_PRX_EMPTY , data->buffers_size);		//NAO SEI SE ESTA BEM O VALOR
	sems->cli_prx->mutex = semaphore_create(STR_SEM_CLI_PRX_MUTEX, 1);

	sems->prx_srv->full = semaphore_create(STR_SEM_PRX_SRV_FULL, 0);
	sems->prx_srv->empty = semaphore_create(STR_SEM_PRX_SRV_EMPTY , data->buffers_size);		//NAO SEI SE ESTA BEM O VALOR
	sems->prx_srv->mutex = semaphore_create(STR_SEM_PRX_SRV_MUTEX, 1);

	sems->srv_cli->full = semaphore_create(STR_SEM_SRV_CLI_FULL, 0);
	sems->srv_cli->empty = semaphore_create(STR_SEM_SRV_CLI_EMPTY  , data->buffers_size);		//NAO SEI SE ESTA BEM O VALOR
	sems->srv_cli->mutex = semaphore_create(STR_SEM_SRV_CLI_MUTEX , 1);

	sems->results_mutex = semaphore_create(STR_SEM_RESULTS_MUTEX , 1);
}

/* Função que inicia os processos dos clientes, proxies e
* servidores. Para tal, pode usar a função launch_process,
* guardando os pids resultantes nos arrays respetivos
* da estrutura data.
*/
void launch_processes(struct communication_buffers* buffers, struct main_data* data, struct semaphores* sems) {
	int clientes = data->n_clients;
	int proxies = data->n_proxies;
	int servers = data->n_servers;

	for(int i = 0; i < clientes; i++){
		data->client_pids[i] = launch_process(i, 0, buffers, data, sems);
	}
	for(int i = 0; i < proxies; i++){
		data->proxy_pids[i] = launch_process(i, 1, buffers, data, sems);
	}
	for(int i = 0; i < servers; i++){
		data->server_pids[i] = launch_process(i, 2, buffers, data, sems);
	}
	

}



/* Se o limite de operações ainda não tiver sido atingido, cria uma nova
* operação identificada pelo valor atual de op_counter, escrevendo a mesma
* no buffer de memória partilhada entre main e clientes e efetuando a 
* necessária sincronização antes e depois de escrever. Imprime o id da
* operação e incrementa o contador de operações op_counter.
*/
void create_request(int* op_counter, struct communication_buffers* buffers, struct main_data* data, struct semaphores* sems) { //NAO ESTA ACABDO

	struct operation* op = malloc(sizeof(struct operation));
	op-> id = (*op_counter);									//SERA QUE TENHO DE FAZER MALLOC?
	op->status = ' ';
	op->client = 0;
	op->proxy = 0;
	op->server = 0;
																//FALTA A SINCRONIZAÇÃO
	produce_begin(sems->main_cli);
	write_rnd_access_buffer(buffers->main_cli, data->buffers_size, op );
	produce_end(sems->main_cli);
	free(op);
	printf("A op %d foi realizada com sucesso.\n", *op_counter);
	(*op_counter)++;
}

/* Função que lê um id de operação do utilizador e verifica se a mesma
* é valida e se já foi respondida por um servidor. Em caso afirmativo,
* imprime informação da mesma, nomeadamente o seu estado, e os ids do 
* cliente, proxy e servidor que a processaram. O acesso à estrutura 
* data->results deve ser sincronizado com as funções e semáforos
* respetivos.
*/
void read_answer(struct main_data* data, struct semaphores* sems) {		//NAO ESTA ACABADO
			// O read vem na forma de "read_x" sendo x o op pretendido, entao temos que o obter
	
	char a[10];
	scanf("%s",a);
	int i = atoi(a);

	semaphore_mutex_lock(sems->results_mutex);
	char status = data->results[i].status;

	if(status != 'S'){
		printf("A op %d, ainda não esta realizada.\n", i);
		return;
	}
	 		//ISTO É SO UMA VARIAVEL DE TESTE PERGUNTAR SE NA FUNCAO NAO DEVAI RECEBER UM ARGUMENTO COM INT
	
	int client =  data->results[i].client;
	int proxy = data->results[i].proxy;
	int server = data->results[i].server;

	// NÃO ESQUECER QUE AINDA FALTA FAZER A SINCRONIZACAO
	semaphore_mutex_unlock(sems->results_mutex);
	printf("Op %d com estado %c foi recebida pelo cliente %d, encaminhada pelo proxu %d, e tratada pelo servido %d!\n", i, status, client, proxy, server);
}

/* Função que termina a execução do programa sovaccines. Deve começar por 
* afetar a flag data->terminate com o valor 1. De seguida, e por esta
* ordem, deve acordar processos adormecidos, esperar que terminem a sua 
* execução, escrever as estatisticas finais do programa, e por fim libertar
* os semáforos e zonas de memória partilhada e dinâmica previamente 
*reservadas. Para tal, pode usar as outras funções auxiliares do main.h.
*/
void stop_execution(struct main_data* data, struct communication_buffers* buffers, struct semaphores* sems){  //NAO ESTA ACABADO
	*(data->terminate) = 1;
	
	wakeup_processes(data, sems);
	
	wait_processes(data);
	write_statistics(data);
	
	destroy_semaphores(sems);
	destroy_shared_memory_buffers(data, buffers);
	destroy_dynamic_memory_buffers(data);
	
	destroy_dynamic_memory(data);
	destroy_dynamic_memory(buffers->main_cli);
	destroy_dynamic_memory(buffers->cli_prx);
	destroy_dynamic_memory(buffers->prx_srv);
	destroy_dynamic_memory(buffers->srv_cli);
	destroy_dynamic_memory(buffers);
	destroy_dynamic_memory(sems->main_cli);
	destroy_dynamic_memory(sems->cli_prx);
	destroy_dynamic_memory(sems->prx_srv);
	destroy_dynamic_memory(sems->srv_cli);
	destroy_dynamic_memory(sems);

}

/* Função que acorda todos os processos adormecidos em semáforos, para que
* estes percebam que foi dada ordem de terminação do programa. Para tal,
* pode ser usada a função produce_end sobre todos os conjuntos de semáforos
* onde possam estar processos adormecidos e um número de vezes igual ao 
* máximo de processos que possam lá estar.
*/
void wakeup_processes(struct main_data* data, struct semaphores* sems){			//NÃO ESTA ACABADO

	produce_end(sems->main_cli);
	produce_end(sems->cli_prx);
	produce_end(sems->prx_srv);
	produce_end(sems->srv_cli);

}

/* Função que espera que todos os processos previamente iniciados terminem,
* incluindo clientes, proxies e servidores. Para tal, pode usar a função 
* wait_process do process.h.
*/
void wait_processes(struct main_data* data){		//MUITO PROVAVEL ESTAR MAL

	//Chamar a funcao wait process
	// guardar no historico

	int clientes = data->n_clients;
	int proxies = data->n_proxies;
	int servers = data->n_servers;
	for(int i = 0; i < clientes; i++){
		data->client_stats[i] = wait_process(data->client_pids[i]);			
	}
	for(int i = 0; i < proxies; i++){
		data->proxy_stats[i] = wait_process(data->proxy_pids[i]);
		
	}
	for(int i = 0; i < servers; i++){
		data->server_stats[i] =wait_process(data->server_pids[i]);
		
	}
}

/* Função que imprime as estatisticas finais do sovaccines, nomeadamente quantas
* operações foram processadas por cada cliente, proxy e servidor.
*/
void write_statistics(struct main_data* data){
	printf("Terminando o sovaccines! Imprimindo estatísticas:\n");

	int clientes = data->n_clients;
	int proxies = data->n_proxies;
	int servers = data->n_servers;

	for(int i = 0; i < clientes; i++){					
		printf("Client %d recebeu %d pedidos!\n",i ,data->client_stats[i]);
	}
	for(int i = 0; i < proxies; i++){
		printf("Proxy %d recebeu %d pedidos!\n",i ,data->proxy_stats[i]);
	}
	for(int i = 0; i < servers; i++){
		printf("Server %d recebeu %d pedidos!\n",i ,data->server_stats[i]);
	}
}

/* Função que liberta todos os buffers de memória dinâmica previamente
* reservados na estrutura data.
*/
void destroy_dynamic_memory_buffers(struct main_data* data){
	destroy_dynamic_memory(data->client_pids);
	destroy_dynamic_memory(data->proxy_pids);
	destroy_dynamic_memory(data->server_pids);

	destroy_dynamic_memory(data->proxy_stats);
	destroy_dynamic_memory(data->client_stats);
	destroy_dynamic_memory(data->server_stats);
	
}


/* Função que liberta todos os buffers de memória partilhada previamente
* reservados nas estruturas data e buffers.
*/
void destroy_shared_memory_buffers(struct main_data* data, struct communication_buffers* buffers){
	
	destroy_shared_memory(STR_SHM_MAIN_CLI_PTR,buffers->main_cli->ptr, (data->buffers_size)*sizeof(int));
	destroy_shared_memory(STR_SHM_MAIN_CLI_BUFFER, buffers->main_cli->op, (data->buffers_size)*sizeof(struct operation));
	
	destroy_shared_memory(STR_SHM_CLI_PRX_PTR, buffers->cli_prx->ptr, (data->buffers_size)*sizeof(struct pointer));
	destroy_shared_memory(STR_SHM_CLI_PRX_BUFFER, buffers->cli_prx->op,(data->buffers_size)*sizeof(struct operation));

	destroy_shared_memory(STR_SHM_PRX_SRV_PTR, buffers->prx_srv->ptr, (data->buffers_size)*sizeof(int));
	destroy_shared_memory(STR_SHM_PRX_SRV_BUFFER,buffers->prx_srv->op, (data->buffers_size)*sizeof(struct operation));

	destroy_shared_memory(STR_SHM_SRV_CLI_PTR ,buffers->srv_cli->ptr, (data->buffers_size)*sizeof(struct pointer));
	destroy_shared_memory(STR_SHM_SRV_CLI_BUFFER,buffers->srv_cli->op, (data->buffers_size)*sizeof(struct operation));
		
	destroy_shared_memory( STR_SHM_RESULTS, data->results, (data->max_ops)*sizeof(struct operation));
	
	destroy_shared_memory(STR_SHM_TERMINATE, data->terminate, sizeof(int));
}

/* Função que liberta todos os semáforos da estrutura semaphores.
*/
void destroy_semaphores(struct semaphores* sems){							//NAO ESTA ACABADO
	semaphore_destroy(STR_SEM_MAIN_CLI_FULL, sems->main_cli->full);
	semaphore_destroy(STR_SEM_MAIN_CLI_EMPTY, sems->main_cli->empty);
	semaphore_destroy(STR_SEM_MAIN_CLI_MUTEX, sems->main_cli->mutex);

	semaphore_destroy(STR_SEM_CLI_PRX_FULL, sems->cli_prx->full);
	semaphore_destroy(STR_SEM_CLI_PRX_EMPTY, sems->cli_prx->empty);
	semaphore_destroy(STR_SEM_CLI_PRX_MUTEX, sems->cli_prx->mutex);

	semaphore_destroy(STR_SEM_PRX_SRV_FULL, sems->prx_srv->full);
	semaphore_destroy(STR_SEM_PRX_SRV_EMPTY, sems->prx_srv->empty);
	semaphore_destroy(STR_SEM_PRX_SRV_MUTEX, sems->prx_srv->mutex);

	semaphore_destroy(STR_SEM_SRV_CLI_FULL, sems->srv_cli->full);
	semaphore_destroy(STR_SEM_SRV_CLI_EMPTY, sems->srv_cli->empty);
	semaphore_destroy(STR_SEM_SRV_CLI_MUTEX, sems->srv_cli->mutex);

	semaphore_destroy(STR_SEM_RESULTS_MUTEX, sems->results_mutex);

}			

/* Função que faz interação do utilizador com o sistema, podendo receber 4 comandos:
* op - cria uma nova operação, através da função create_request
* read - verifica o estado de uma operação através da função read_answer
* stop - termina o execução do sovaccines através da função stop_execution
* help - imprime informação sobre os comandos disponiveis
*/
void user_interaction(struct communication_buffers* buffers, struct main_data* data, struct semaphores* sems) {
	
	//Primeira mensagem a ser impressa
	printf("Ações disponiveis: \n");
	printf("	op - criar um pedido de aquisição de vacinas\n");
	printf("	read x - consultar o estado do pedido x (valor máximo é %d)\n", data->max_ops);
	printf("	stop - termina a execução do sovaccines.\n");
	printf("	help - imprime informação sobre as ações disponiveis\n");

	int* p = malloc(sizeof(int));
	int end = 0;

	while(end != 1){
		char resp[10];
		printf("Introduzir ação:\n");
		scanf("%s", resp);

		if(strcmp(resp,"stop") == 0){
				stop_execution(data, buffers, sems);
				end = 1;
				free(p);
		}else if(strcmp(resp,"help") == 0){
				printf("Ações disponiveis: \n");
				printf("	op - criar um pedido de aquisição de vacinas\n");
				printf("	read x - consultar o estado do pedido x (valor máximo é %d)\n", data->max_ops);
				printf("	stop - termina a execução do sovaccines.\n");
				printf("	help - imprime informação sobre as ações disponiveis\n");
		}else if(strcmp(resp,"op") == 0){
			if(data->max_ops < (*p)){			
					printf("O número máximo de pedidos foi alcançado!\n");
					break;
				}else{
					create_request(p, buffers, data, sems); 
				}
		} else if(strcmp(resp,"read") == 0){
			
				read_answer(data, sems);
		}else{
			printf("Ação não reconhecida, insira 'help' para assistência.\n");
		}
	}

	
}
/* Função que lê os argumentos da aplicação, nomeadamente o número
* máximo de operações, o tamanho dos buffers de memória partilhada
* usados para comunicação, e o número de clientes, de proxies e de
* servidores. Guarda esta informação nos campos apropriados da
* estrutura main_data.
*/
void main_args(int argc, char* argv[], struct main_data* data) {
	for(int i = 1; i < argc; i++){
		switch (i) {
			case 1:
				data->max_ops = atoi(argv[i]);
				break;
			case 2:
				data->buffers_size = atoi(argv[i]);
				break;
			case 3:
				data->n_clients = atoi(argv[i]);
				break;
			case 4:
				data->n_proxies = atoi(argv[i]);
				break;
			case 5:
				data->n_servers = atoi(argv[i]);
				break;
		}
	}
}

int main(int argc, char *argv[]) {
//init data structures

	if(argc < 6) {
		perror("Nao foram dados parametros suficientes.");
        exit(1);
	}else{
		// shm_unlink("/SHM_MAIN_CLI_PTR_1000");
		// shm_unlink("/SHM_MAIN_CLI_BUFFER_1000");
		// shm_unlink("/SHM_CLI_PRX_PTR_1000");
		// shm_unlink("/SHM_CLI_PRX_BUFFER_1000");
		// shm_unlink("/SHM_PRX_SRV_PTR_1000");
		// shm_unlink("/SHM_PRX_SRV_BUFFER_1000");
		// shm_unlink("/SHM_SRV_CLI_PTR_1000");
		// shm_unlink("/SHM_SRV_CLI_BUFFER_1000");
		// shm_unlink("/SHM_RESULTS_1000");
		// shm_unlink("/SHM_TERMINATE_1000");

		// sem_unlink("sem_main_cli_full_1000");
		// sem_unlink("sem_main_cli_empty_1000");
		// sem_unlink("sem_main_cli_mutex_1000");
		// sem_unlink("sem_cli_prx_full_1000");
		// sem_unlink("sem_cli_prx_empty_1000");
		// sem_unlink("sem_cli_prx_mutex_1000");
		// sem_unlink("sem_prx_srv_full_1000");
		// sem_unlink("sem_prx_srv_empty_1000");
		// sem_unlink("sem_prx_srv_mutex_1000");
		// sem_unlink("sem_srv_cli_full_1000");
		// sem_unlink("sem_srv_cli_empty_1000");
		// sem_unlink("sem_srv_cli_mutex_1000");
		// sem_unlink("sem_results_mutex_1000");

		
		struct main_data* data = create_dynamic_memory(sizeof(struct main_data));
		struct communication_buffers* buffers = create_dynamic_memory(sizeof(struct communication_buffers));
		buffers->main_cli = create_dynamic_memory(sizeof(struct rnd_access_buffer));
		buffers->cli_prx = create_dynamic_memory(sizeof(struct circular_buffer));
		buffers->prx_srv = create_dynamic_memory(sizeof(struct rnd_access_buffer));
		buffers->srv_cli = create_dynamic_memory(sizeof(struct circular_buffer));
		struct semaphores* sems = create_dynamic_memory(sizeof(struct semaphores));
		sems->main_cli = create_dynamic_memory(sizeof(struct prodcons));
		sems->cli_prx = create_dynamic_memory(sizeof(struct prodcons));
		sems->prx_srv = create_dynamic_memory(sizeof(struct prodcons));
		sems->srv_cli = create_dynamic_memory(sizeof(struct prodcons));
		
		//execute main code
		main_args(argc, argv, data);
		
		create_dynamic_memory_buffers(data);
		create_shared_memory_buffers(data, buffers);
		create_semaphores(data, sems);
		
		launch_processes(buffers, data, sems);
		user_interaction(buffers, data, sems);

		
	}

	
}


