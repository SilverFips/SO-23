/**
 * Grupo: SO-023
 * Francisco Martins nº 51073
 * Filipe Pedroso nº 51958
 * Tiago Lourenço nº 46670
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <signal.h>
#include <errno.h>


#include "main.h"
#include "memory.h"
#include "memory-private.h"
#include "synchronization.h"
#include "process.h"

#include "configuration.h"

#include "sosignal.h"
#include "sotime.h"
#include "log.h"


struct main_data* data;
struct communication_buffers* buffers;
struct semaphores* sems;
int end;



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
	

}

/* Função que inicializa os semáforos da estrutura semaphores. Semáforos
* *_full devem ser inicializados com valor 0, semáforos *_empty com valor
* igual ao tamanho dos buffers de memória partilhada, e os *_mutex com valor
* igual a 1. Para tal pode ser usada a função semaphore_create.
*/
void create_semaphores(struct main_data* data, struct semaphores* sems){		
	sems->main_cli->full = semaphore_create(STR_SEM_MAIN_CLI_FULL, 0);
	sems->main_cli->empty = semaphore_create(STR_SEM_MAIN_CLI_EMPTY , data->buffers_size);		
	sems->main_cli->mutex = semaphore_create(STR_SEM_MAIN_CLI_MUTEX, 1);

	sems->cli_prx->full = semaphore_create(STR_SEM_CLI_PRX_FULL, 0);
	sems->cli_prx->empty = semaphore_create(STR_SEM_CLI_PRX_EMPTY , data->buffers_size);		
	sems->cli_prx->mutex = semaphore_create(STR_SEM_CLI_PRX_MUTEX, 1);

	sems->prx_srv->full = semaphore_create(STR_SEM_PRX_SRV_FULL, 0);
	sems->prx_srv->empty = semaphore_create(STR_SEM_PRX_SRV_EMPTY , data->buffers_size);		
	sems->prx_srv->mutex = semaphore_create(STR_SEM_PRX_SRV_MUTEX, 1);

	sems->srv_cli->full = semaphore_create(STR_SEM_SRV_CLI_FULL, 0);
	sems->srv_cli->empty = semaphore_create(STR_SEM_SRV_CLI_EMPTY  , data->buffers_size);		
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
void create_request(int* op_counter, struct communication_buffers* buffers, struct main_data* data, struct semaphores* sems) { 

	struct operation* op = malloc(sizeof(struct operation));
	op-> id = (*op_counter);									
	op->status = ' ';
	op->client = -1;
	op->proxy = -1;
	op->server = -1;
	op->start_time = getTime(op->start_time);

																
	produce_begin(sems->main_cli);
	write_rnd_access_buffer(buffers->main_cli, data->buffers_size, op );
	produce_end(sems->main_cli);
	free(op);
	sleep(1);
	//consume_begin(sems->main_cli);
	printf("-> A op #%d pode ser consultada.\n", *op_counter);
	//consume_end(sems->main_cli);
	(*op_counter)++;
}

/* Função que lê um id de operação do utilizador e verifica se a mesma
* é valida e se já foi respondida por um servidor. Em caso afirmativo,
* imprime informação da mesma, nomeadamente o seu estado, e os ids do 
* cliente, proxy e servidor que a processaram. O acesso à estrutura 
* data->results deve ser sincronizado com as funções e semáforos
* respetivos.
*/
void read_answer(struct main_data* data, struct semaphores* sems) {		
				
	alarm(0);
	int i;
	printf("-> Qual a operação pretendida? (valor máximo é %d)\n", (data->max_ops-1));
	scanf("%d", &i);

	struct timespec op; 
	op = getTime(op);
	char log_data[50];
	toString(op, log_data);

	semaphore_mutex_lock(sems->results_mutex);
	char status = data->results[i].status;

	if(status != 'S'){
		write_file_log(log_data, "read", i);
		printf("-> A op #%d, ainda não esta realizada.\n", i);
		semaphore_mutex_unlock(sems->results_mutex);
		return;
	}
	 		
	
	int client =  data->results[i].client;
	int proxy = data->results[i].proxy;
	int server = data->results[i].server;

	
	semaphore_mutex_unlock(sems->results_mutex);
	write_file_log(log_data, "read", i);
	printf("-> Op #%d com estado %c foi recebida pelo cliente %d, encaminhada pelo proxy %d, e tratada pelo servido %d!\n", i, status, client, proxy, server);
	
}

/* Função que termina a execução do programa sovaccines. Deve começar por 
* afetar a flag data->terminate com o valor 1. De seguida, e por esta
* ordem, deve acordar processos adormecidos, esperar que terminem a sua 
* execução, escrever as estatisticas finais do programa, e por fim libertar
* os semáforos e zonas de memória partilhada e dinâmica previamente 
*reservadas. Para tal, pode usar as outras funções auxiliares do main.h.
*/
void stop_execution(struct main_data* data, struct communication_buffers* buffers, struct semaphores* sems){  
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
	
	//SITIO PARA CHAMADAS DE DESTRUIÇÃO DOS FILES
	file_destroy_log();
	
	end = 1;

}

/* Função que acorda todos os processos adormecidos em semáforos, para que
* estes percebam que foi dada ordem de terminação do programa. Para tal,
* pode ser usada a função produce_end sobre todos os conjuntos de semáforos
* onde possam estar processos adormecidos e um número de vezes igual ao 
* máximo de processos que possam lá estar.
*/
void wakeup_processes(struct main_data* data, struct semaphores* sems){			

	int clientes = data->n_clients;
	int proxies = data->n_proxies;
	int servers = data->n_servers;
	for(int i = 0; i < clientes; i++){
		produce_end(sems->main_cli);
		produce_end(sems->srv_cli);			
	}
	
	for(int i = 0; i < proxies; i++){
		produce_end(sems->cli_prx);			
	}
	for(int i = 0; i < servers; i++){
		produce_end(sems->prx_srv);		
	}
		
}

/* Função que espera que todos os processos previamente iniciados terminem,
* incluindo clientes, proxies e servidores. Para tal, pode usar a função 
* wait_process do process.h.
*/
void wait_processes(struct main_data* data){		

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
	printf("\n-> Terminando o sovaccines! Imprimindo estatísticas:\n");

	int clientes = data->n_clients;
	int proxies = data->n_proxies;
	int servers = data->n_servers;
	printf("\n");
	for(int i = 0; i < clientes; i++){					
		printf("Client %d recebeu %d pedidos!\n",i ,data->client_stats[i]);
	}
	printf("\n");
	for(int i = 0; i < proxies; i++){
		printf("Proxy %d encaminhou %d pedidos!\n",i ,data->proxy_stats[i]);
	}
	printf("\n");
	for(int i = 0; i < servers; i++){
		printf("Server %d respondeu %d pedidos!\n",i ,data->server_stats[i]);
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
void destroy_semaphores(struct semaphores* sems){						
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
	printf("	read - consultar o estado de um pedido (valor máximo é %d)\n", data->max_ops);
	printf("	stop - termina a execução do sovaccines.\n");
	printf("	help - imprime informação sobre as ações disponiveis\n");

	int p = 0;
	struct timespec op;
	
	//REFERENCIA A PARTE DO SIGNAL 
	struct sigaction sa;
	sa.sa_handler = sig_handler;
	sa.sa_flags = 0;
	sigemptyset(&sa.sa_mask);
	

	if (sigaction(SIGINT, &sa, NULL) == -1) {
		perror("main:");
		exit(-1);
	}

	//ALARME
	struct sigaction s;
	s.sa_handler = sig_handler;
	s.sa_flags = SA_RESTART;
	sigemptyset(&s.sa_mask);	
	if (sigaction(SIGALRM, &s, NULL) == -1) {
		perror("main:");
		exit(-1);
	}
	alarm(data->alarm);

	//WHILE PRINCIPAL
	while(end != 1){
		// OPERACOES NECESSARIAS PARA O LOG FILE
		op = getTime(op);
		char log_data[50];
		toString(op, log_data);
		//
		
		char resp[10];
		printf("-> Introduzir ação:\n");
		scanf("%s", resp);

		if(end == 1){
			printf("-------------------------------------------------------------------------\n");
			return;
		}
			
		if(strcmp(resp,"stop") == 0){
				write_file_log(log_data, resp, -1);
				stop_execution(data, buffers, sems);
		}else if(strcmp(resp,"help") == 0){
				printf("Ações disponiveis: \n");
				printf("	op - criar um pedido de aquisição de vacinas\n");
				printf("	read - consultar o estado de um pedido (valor máximo é %d)\n", data->max_ops);
				printf("	stop - termina a execução do sovaccines.\n");
				printf("	help - imprime informação sobre as ações disponiveis\n");
		}else if(strcmp(resp,"op") == 0){
			write_file_log(log_data, resp, -1);
			if(data->max_ops < ((p)+1)){			
					printf("-> O número máximo de pedidos foi alcançado!\n");
				}else{
					alarm(2);
					create_request(&p, buffers, data, sems); 
					alarm(data->alarm);
				}
		} else if(strcmp(resp,"read") == 0){
			
				read_answer(data, sems);
				alarm(data->alarm);
		}else{
			printf("-> Ação não reconhecida, insira 'help' para assistência.\n");
		}
		printf("-------------------------------------------------------------------------\n");
	}		
}
/* Função que lê os argumentos da aplicação, nomeadamente o número
* máximo de operações, o tamanho dos buffers de memória partilhada
* usados para comunicação, e o número de clientes, de proxies e de
* servidores. Guarda esta informação nos campos apropriados da
* estrutura main_data.
*/
void main_args(int argc, char* argv[], struct main_data* data) {
	char *config_file = argv[1];
	open_begin_file(config_file, data);

}

int main(int argc, char *argv[]) {
//init data structures
	if(argc != 2){
		printf("Numero de parametros incorretos.\n");
		printf("Estrutura: sovaccines bin/<nome_fich>\n");
		printf("Exemplo: ./bin/sovaccines bin/<nome_fich>\n");
		exit(1);
	// if(argc != 6) {
	// 	perror("Nao foram dados o numero de parametros corretos.\n");
	// 	printf("Estrutura: sovaccines max_ops buffers_size n_clients n_proxies n_servers\n");
	// 	printf("Exemplo: ./bin/sovaccines 10 10 1 1 1\n");
    //     exit(1);
	}else{

				
		data = create_dynamic_memory(sizeof(struct main_data));
		buffers = create_dynamic_memory(sizeof(struct communication_buffers));
		buffers->main_cli = create_dynamic_memory(sizeof(struct rnd_access_buffer));
		buffers->cli_prx = create_dynamic_memory(sizeof(struct circular_buffer));
		buffers->prx_srv = create_dynamic_memory(sizeof(struct rnd_access_buffer));
		buffers->srv_cli = create_dynamic_memory(sizeof(struct circular_buffer));
		sems = create_dynamic_memory(sizeof(struct semaphores));
		sems->main_cli = create_dynamic_memory(sizeof(struct prodcons));
		sems->cli_prx = create_dynamic_memory(sizeof(struct prodcons));
		sems->prx_srv = create_dynamic_memory(sizeof(struct prodcons));
		sems->srv_cli = create_dynamic_memory(sizeof(struct prodcons));
		
	
		//execute main code
		main_args(argc, argv, data);

		

		dados(buffers, data, sems);
		
		create_dynamic_memory_buffers(data);
		create_shared_memory_buffers(data, buffers);
		create_semaphores(data, sems);
		end = 0;
		//VARIAVEL SO PARA TESTE
		// (data->alarm) = 2;
		// file_log_begin("./bin/log");
		//--------------------------		
		launch_processes(buffers, data, sems);
		
		user_interaction(buffers, data, sems);

		
	}


}


