#ifndef MEMORY_PRIVATE_H_GUARD
#define MEMORY_PRIVATE_H_GUARD

struct pointer {
    int in;
    int out;
};

//estrutura que representa um buffer circular, completar pelos alunos
struct circular_buffer { 	
    struct pointer* ptr;
    struct operation* op;
};

//estrutura que representa um buffer de acesso aleatório, completar pelos alunos
struct rnd_access_buffer { 		
    int* ptr;
    struct operation* op;
};

#endif
