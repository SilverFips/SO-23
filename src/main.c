#ifndef MAIN_H_GUARD
#define MAIN_H_GUARD

#include <stdio.h>
#include "memory.h"
#include "synchronization.h"


//Estrutura que agrega a informação necessária pela main do sovaccines.
struct main_data {
	int max_ops;		//número máximo de operações
	int buffers_size;	//tamanho máximo dos buffers de mem. partilhada
	
	int n_clients;		//número de clientes
	int n_proxies;		//número de proxies
	int n_servers;		//número de servidores
	
	int *client_pids;	//process ids de clientes
	int *proxy_pids;	//process ids de proxies
	int *server_pids;	//process ids de servidores
	
	int* client_stats;	//nº de operações recebidas por cada cliente
	int* proxy_stats;	//nº de operações encaminhadas por cada proxy
	int* server_stats;	//nº de operações respondidas por cada servidor
	
	struct operation* results;	//array com histórico de ops executadas
	
	int* terminate; //flag booleana, valor 1 indica que sovaccines deve terminar a sua execução
};


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
				data->max_ops = *argv[i];
				break;
			case 2:
				data->buffers_size = *argv[i];
				break;
			case 3:
				data->n_clients = *argv[i];
				break;
			case 4:
				data->n_proxies = *argv[i];
				break;
			case 5:
				data->n_servers = *argv[i];
				break;
		}
	}
}

/* Função que reserva a memória dinâmica necessária para a execução
* do sovaccines, nomeadamente para os arrays *_pids e *_stats da estrutura 
* main_data. Para tal, pode ser usada a função create_dynamic_memory do memory.h.
*/
void create_dynamic_memory_buffers(struct main_data* data) {

}

/* Função que reserva a memória partilhada necessária para a execução do
* sovaccines. É necessário reservar memória partilhada para todos os buffers da
* estrutura communication_buffers, incluindo os buffers em si e respetivos
* pointers, assim como para o array data->results e variável data->terminate.
* Para tal, pode ser usada a função create_shared_memory do memory.h.
*/
void create_shared_memory_buffers(struct main_data* data, struct communication_buffers* buffers) {

}

/* Função que inicializa os semáforos da estrutura semaphores. Semáforos
* *_full devem ser inicializados com valor 0, semáforos *_empty com valor
* igual ao tamanho dos buffers de memória partilhada, e os *_mutex com valor
* igual a 1. Para tal pode ser usada a função semaphore_create.
*/
void create_semaphores(struct main_data* data, struct semaphores* sems){

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
		data->client_stats[i] = launch_process(i, 0, buffers, data, sems);
	}
	for(int i = 0; i < proxies; i++){
		data->proxy_pids[i] = launch_process(i, 1, buffers, data, sems);
	}
	for(int i = 0; i < servers; i++){
		data->server_pids[i] = launch_process(i, 2, buffers, data, sems);
	}
	

}

/* Função que faz interação do utilizador com o sistema, podendo receber 4 comandos:
* op - cria uma nova operação, através da função create_request
* read - verifica o estado de uma operação através da função read_answer
* stop - termina o execução do sovaccines através da função stop_execution
* help - imprime informação sobre os comandos disponiveis
*/
void user_interaction(struct communication_buffers* buffers, struct main_data* data, struct semaphores* sems) {

}

/* Se o limite de operações ainda não tiver sido atingido, cria uma nova
* operação identificada pelo valor atual de op_counter, escrevendo a mesma
* no buffer de memória partilhada entre main e clientes e efetuando a 
* necessária sincronização antes e depois de escrever. Imprime o id da
* operação e incrementa o contador de operações op_counter.
*/
void create_request(int* op_counter, struct communication_buffers* buffers, struct main_data* data, struct semaphores* sems) {

}

/* Função que lê um id de operação do utilizador e verifica se a mesma
* é valida e se já foi respondida por um servidor. Em caso afirmativo,
* imprime informação da mesma, nomeadamente o seu estado, e os ids do 
* cliente, proxy e servidor que a processaram. O acesso à estrutura 
* data->results deve ser sincronizado com as funções e semáforos
* respetivos.
*/
void read_answer(struct main_data* data, struct semaphores* sems) {

}

/* Função que termina a execução do programa sovaccines. Deve começar por 
* afetar a flag data->terminate com o valor 1. De seguida, e por esta
* ordem, deve acordar processos adormecidos, esperar que terminem a sua 
* execução, escrever as estatisticas finais do programa, e por fim libertar
* os semáforos e zonas de memória partilhada e dinâmica previamente 
*reservadas. Para tal, pode usar as outras funções auxiliares do main.h.
*/
void stop_execution(struct main_data* data, struct communication_buffers* buffers, struct semaphores* sems){

}

/* Função que acorda todos os processos adormecidos em semáforos, para que
* estes percebam que foi dada ordem de terminação do programa. Para tal,
* pode ser usada a função produce_end sobre todos os conjuntos de semáforos
* onde possam estar processos adormecidos e um número de vezes igual ao 
* máximo de processos que possam lá estar.
*/
void wakeup_processes(struct main_data* data, struct semaphores* sems){}

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
		data->results = wait_process(data->client_pids[i]);
		data->client_stats[i] += 1;
	}
	for(int i = 0; i < proxies; i++){
		data->results = wait_process(data->proxy_pids[i]);
		data->proxy_stats[i] += 1;
	}
	for(int i = 0; i < servers; i++){
		data->results = wai_process(data->server_pids[i]);
		data->server_stats[i] += 1;
	}
}


/* Função que imprime as estatisticas finais do sovaccines, nomeadamente quantas
* operações foram processadas por cada cliente, proxy e servidor.
*/
void write_statistics(struct main_data* data){
	printf("Terminando o sovaccines! Imprimindo estatisticas:\n");
	
	int clientes = data->n_clients;
	int proxies = data->n_proxies;
	int servers = data->n_servers;

	for(int i = 0; i < clientes; i++){
		printf("CLIENTE %d recebeu %d pedidos!\n", i, data->client_stats[i]);
	}
	for(int i = 0; i < proxies; i++){
		printf("PROXY %d recebeu %d pedidos!\n", i, data->proxy_stats[i]);
	}
	for(int i = 0; i < servers; i++){
		printf("SERVER %d recebeu %d pedidos!\n", i, data->server_stats[i]);
	}

}

/* Função que liberta todos os buffers de memória dinâmica previamente
* reservados na estrutura data.
*/
void destroy_dynamic_memory_buffers(struct main_data* data){}


/* Função que liberta todos os buffers de memória partilhada previamente
* reservados nas estruturas data e buffers.
*/
void destroy_shared_memory_buffers(struct main_data* data, struct communication_buffers* buffers){}

/* Função que liberta todos os semáforos da estrutura semaphores.
*/
void destroy_semaphores(struct semaphores* sems){}

int main(int argc, char *argv[]) {
//init data structures

	if(argc < 6) {
		perror("Nao foram dados parametros suficientes.");
        exit(1);
	}else{
		/*
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
		*/
		//execute main code
		main_args(argc, argv, data);
		create_dynamic_memory_buffers(data);
		create_shared_memory_buffers(data, buffers);
		create_semaphores(data, sems);
		launch_processes(buffers, data, sems);
		user_interaction(buffers, data, sems);

		/*
		//release final memory
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
		*/
	}

	
}


#endif
