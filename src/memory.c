/**
 * Grupo: SO-023
 * Francisco Martins nº 51073
 * Filipe Pedroso nº 51958
 * Tiago Lourenço nº 46670
*/

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include "memory-private.h"
#include "memory.h"
#include "main.h"


/* Função que reserva uma zona de memória partilhada com tamanho indicado
* por size e nome name, preenche essa zona de memória com o valor 0, e 
* retorna um apontador para a mesma. Pode concatenar o resultado da função
* getuid() a name, para tornar o nome único para o processo.
*/
void* create_shared_memory(char* name, int size){
	char name_fd[50];
	int ret;
	sprintf(name_fd,"/%s_%d", name, getuid());
	int fd = shm_open(name_fd, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
	if (fd == -1){ 
		perror("shm"); 
		exit(1); 
	}
	ret = ftruncate(fd, size);							//DEFINICAO DO TAMANHO DO FICHEIRO
	if (ret == -1){ 
		perror("shm"); 
		exit(2); 
	}
	int* ptr = mmap(0, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);		//PROJECAO DA MEMORIA PARTILHADA
	if (ptr == MAP_FAILED){ 
		perror("shm-mmap"); 
		exit(3); 
	}
	
	return memset(ptr, 0, size);
}


/* Função que reserva uma zona de memória dinâmica com tamanho indicado
* por size, preenche essa zona de memória com o valor 0, e retorna um 
* apontador para a mesma.
*/
void* create_dynamic_memory(int size){
	void *s = malloc(size);
	return memset(s, 0, size);
}


/* Função que liberta uma zona de memória dinâmica previamente reservada.
*/
void destroy_shared_memory(char* name, void* ptr, int size){
	int ret;
	char name_fd[50];
	sprintf(name_fd,"/%s_%d", name, getuid());
	ret = munmap(ptr, size); 
	if (ret == -1){ 
		perror("exit7"); 
		exit(7); 
	} 
	ret = shm_unlink(name_fd); 
	if (ret == -1){ 
		perror("/shm"); 
		exit(8); 
	}
}


/* Função que liberta uma zona de memória partilhada previamente reservada.
*/
void destroy_dynamic_memory(void* ptr){
	free(ptr);
}


/* Função que escreve uma operação num buffer de acesso aleatório. A
* operação deve ser escrita numa posição livre do buffer, segundo as
* regras de escrita em buffers de acesso aleatório. Se não houver nenhuma
* posição livre, não escreve nada.
*/
void write_rnd_access_buffer(struct rnd_access_buffer* buffer, int buffer_size, struct operation* op){
	for(int i = 0; i < buffer_size; i++){
		if(buffer->ptr[i] == 0){
			buffer->op[i].id = op->id;
			buffer->op[i].status = op->status;
			buffer->op[i].client = op->client;
			buffer->op[i].proxy = op->proxy;
			buffer->op[i].server = op->server;
			buffer->op[i].start_time = op->start_time;
			buffer->op[i].client_time = op->client_time;
			buffer->op[i].proxy_time = op->proxy_time;
			buffer->op[i].server_time = op->server_time;
			buffer->op[i].end_time = op->end_time;
			buffer->ptr[i] = 1;
			//printf("memory op id: %d, memory status: %c, memory client: %d, memory proxy: %d, memory server: %d   .\n", buffer->op[i].id, buffer->op[i].status, buffer->op[i].client, buffer->op[i].proxy, buffer->op[i].server );
			break;
		}
	}	
}


/* Função que escreve uma operação num buffer circular. A operação deve 
* ser escrita numa posição livre do buffer, segundo as regras de escrita
* em buffers circulares. Se não houver nenhuma posição livre, não escreve
* nada.
*/
void write_circular_buffer(struct circular_buffer* buffer, int buffer_size, struct operation* op){
	int pos_in = buffer->ptr->in;
	//int pos_out = buffer->ptr->out;

	//while(((pos_in + 1) % buffer_size) == pos_out);
	buffer->op[pos_in].id = op->id;
	
	buffer->op[pos_in].status = op->status;
	buffer->op[pos_in].client = op->client;
	
	buffer->op[pos_in].proxy = op->proxy;
	buffer->op[pos_in].server = op->server;
	buffer->op[pos_in].start_time = op->start_time;
	buffer->op[pos_in].client_time = op->client_time;
	buffer->op[pos_in].proxy_time = op->proxy_time;
	buffer->op[pos_in].server_time = op->server_time;
	buffer->op[pos_in].end_time = op->end_time;
	buffer->ptr->in = (pos_in +1) % buffer_size;

	//printf("write_circular : op: %d, st: %c, cli: %d, pro: %d, srv: %d\n", buffer->op[pos_in].id, buffer->op[pos_in].status, buffer->op[pos_in].client, buffer->op[pos_in].proxy, buffer->op[pos_in].server);
}


/* Função que lê uma operação de um buffer de acesso aleatório, se houver
* alguma disponível para ler. A leitura deve ser feita segundo as regras
* de leitura em buffers acesso aleatório. Se não houver nenhuma operação
* disponível, afeta op com o valor -1.
*/
void read_rnd_access_buffer(struct rnd_access_buffer* buffer, int buffer_size, struct operation* op){
	for(int i = 0; i < buffer_size; i++){
		if(buffer->ptr[i] == 1){
			op->id = buffer->op[i].id;
			buffer->op[i].id = -1;
			op->status = buffer->op[i].status;
			buffer->op[i].status = ' ';
			op-> client = buffer->op[i].client;
			buffer->op[i].client = -1;
			op->proxy = buffer->op[i].proxy;
			buffer->op[i].proxy = -1;
			op->server = buffer->op[i].server;
			buffer->op[i].server = -1;
			op->start_time = buffer->op[i].start_time;
			op->client_time = buffer->op[i].client_time;
			op->server_time = buffer->op[i].server_time;
			op->proxy_time = buffer->op[i].proxy_time;
			op->end_time = buffer->op[i].end_time;


			buffer->ptr[i] = 0;
			
			return;
		}
	}
	//printf("read_circular : op: %d, st: %c, cli: %d, pro: %d, srv: %d\n", op->id, op->status, op->client, op->proxy, op->server);
	op->id = -1;
}


/* Função que lê uma operação de um buffer circular, se houver alguma 
* disponível para ler. A leitura deve ser feita segundo as regras de
* leitura em buffers circular. Se não houver nenhuma operação disponível,
* afeta op->id com o valor -1.
*/
void read_circular_buffer(struct circular_buffer* buffer, int buffer_size, struct operation* op){

	//int pos_in = buffer->ptr->in;
	int pos_out = buffer->ptr->out;

	//while(pos_in == pos_out);

	op->id = buffer->op[pos_out].id;
    buffer->op[pos_out].id = -1;
    op->status = buffer->op[pos_out].status;
    buffer->op[pos_out].status = ' ';
    op->client = buffer->op[pos_out].client;
    buffer->op[pos_out].client = -1;
    op->proxy = buffer->op[pos_out].proxy;
    buffer->op[pos_out].proxy = -1;
    op->server = buffer->op[pos_out].server;
    buffer->op[pos_out].server = -1;
	op->start_time = buffer->op[pos_out].start_time;
	op->client_time = buffer->op[pos_out].client_time;
	op->server_time = buffer->op[pos_out].server_time;
	op->proxy_time = buffer->op[pos_out].proxy_time;
	op->end_time = buffer->op[pos_out].end_time;
    buffer->ptr->out = (pos_out +1) % buffer_size;

	//printf("read_circular : op: %d, st: %c, cli: %d, pro: %d, srv: %d\n", op->id, op->status, op->client, op->proxy, op->server);
}



