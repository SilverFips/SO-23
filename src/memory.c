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
	ret = ftruncate(fd, size);
	if (ret == -1){ 
		perror("shm"); 
		exit(2); 
	}
	int* ptr = mmap(0, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (ptr == MAP_FAILED){ 
		perror("shm-mmap"); 
		exit(3); 
	}
	return ptr;

	//Preencher com o valor 0 
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
	ret = munmap(ptr, sizeof(int)); 
	if (ret == -1){ 
		perror("/shm"); 
		exit(7); 
	} 
	ret = shm_unlink("/shm"); 
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
			buffer->ptr[i] = 1;
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
	int pos_out = buffer->ptr->out;

	while(((pos_in + 1) % buffer_size) == pos_out);

	buffer->op[pos_in].id = op->id;
	buffer->op[pos_in].status = op->status;
	buffer->op[pos_in].client = op->client;
	buffer->op[pos_in].proxy = op->proxy;
	buffer->op[pos_in].server = op->server;
	buffer->ptr->in = (pos_in +1) % buffer_size;
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
			op->status = buffer->op[i].status;
			op-> client = buffer->op[i].client;
			op->proxy = buffer->op[i].proxy;
			op->server = buffer->op[i].server;
			buffer->ptr[i] = 0;
			break;
		}
	}
	buffer->op->id = -1;
}


/* Função que lê uma operação de um buffer circular, se houver alguma 
* disponível para ler. A leitura deve ser feita segundo as regras de
* leitura em buffers circular. Se não houver nenhuma operação disponível,
* afeta op->id com o valor -1.
*/
void read_circular_buffer(struct circular_buffer* buffer, int buffer_size, struct operation* op){

	int pos_in = buffer->ptr->in;
	int pos_out = buffer->ptr->out;

	while(pos_in == pos_out);

	op->id = buffer->op[pos_in].id;
	op->status = buffer->op[pos_in].status;
	op->client = buffer->op[pos_in].client;
	op->proxy = buffer->op[pos_in].proxy;
	op->server = buffer->op[pos_in].server;
	buffer->ptr->out = (pos_out +1) % buffer_size;

}



