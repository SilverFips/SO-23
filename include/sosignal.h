#ifndef SOSIGNAL_H_GUARD
#define SOSIGNAL_H_GUARD

#include <main.h>

/**
 * Função que realiza a paragem do programa quando é selecionado a flag SIGINT. 
 * Nesta será chamada a função stop_execution do main
 * 
*/
void ctrlC();

void dados(struct communication_buffers* buffers, struct main_data* data, struct semaphores* sems);

void alarm_message();
void sig_handler(int signum);
void ctrlC_other();
#endif