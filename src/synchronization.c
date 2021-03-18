
#include <fcntl.h>
#include <semaphore.h>
#include <stdio.h>
#include "synchronization.h"

/* Função que cria um novo semáforo com nome name e valor inicial igual a
* value. Pode concatenar o resultado da função getuid() a name, para tornar
* o nome único para o processo.
*/
sem_t * semaphore_create(char* name, int value){
	
	char name_fd[50];
	
	sprintf(name_fd,"%s_%d", name, getuid());
	printf( "\n nome semaforo_create: %s\n", name_fd);
	sem_t* sem = sem_open(name_fd, O_CREAT, 0xFFFFFFFF, value);
	if (sem == SEM_FAILED)
 		perror(name);
	return sem;
}

/* Função que destroi o semáforo passado em argumento.
*/
void semaphore_destroy(char* name, sem_t* semaphore){
	char name_fd[50];
	
	sprintf(name_fd,"%s_%d", name, getuid());
	
	if (sem_close(semaphore) == -1){
 		perror(name);
	}
	if (sem_unlink(name_fd) == -1){
 		perror(name);
	}
}
	
/* Função que inicia o processo de produzir, fazendo sem_wait nos semáforos
* corretos da estrutura passada em argumento.
*/
void produce_begin(struct prodcons* pc){
	if(sem_wait(pc->empty) == -1){
		perror("wait empty");
	}
}

/* Função que termina o processo de produzir, fazendo sem_post nos semáforos
* corretos da estrutura passada em argumento.
*/
void produce_end(struct prodcons* pc){
	if(sem_post(pc->full) == -1){
		perror("post full");
		
	}
}

/* Função que inicia o processo de consumir, fazendo sem_wait nos semáforos
* corretos da estrutura passada em argumento.
*/
void consume_begin(struct prodcons* pc){
	if(sem_wait(pc->full) == -1){
		perror("wait full");
	}
}

/* Função que termina o processo de consumir, fazendo sem_post nos semáforos
* corretos da estrutura passada em argumento.
*/
void consume_end(struct prodcons* pc){
	if(sem_post(pc->empty) == -1){
		perror("post empty");
	}
}

/* Função que faz wait a um semáforo.
*/
void semaphore_mutex_lock(sem_t* sem){
	if(sem_wait(sem) == -1){
		perror("wait mutex");
	}
}

/* Função que faz post a um semáforo.
*/
void semaphore_mutex_unlock(sem_t* sem){
	if(sem_post(sem) == -1){
		perror("post mutex");
	}
}


