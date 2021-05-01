/**
 * Grupo: SO-023
 * Francisco Martins nº 51073
 * Filipe Pedroso nº 51958
 * Tiago Lourenço nº 46670
*/

#ifndef SOSIGNAL_H_GUARD
#define SOSIGNAL_H_GUARD

#include <main.h>

/**
 * Função que realiza a paragem do programa quando é selecionado a flag SIGINT. 
 * Nesta será chamada a função stop_execution do main
 * 
*/
void ctrlC();

/**
 * Passagem dos apontadores buffers,data e sems para variavel globar no sosignal
*/
void dados(struct communication_buffers* buffers, struct main_data* data, struct semaphores* sems);

/*
 * Realiza a mensagem pretendida a quando o SIGALARM é ativado
*/
void alarm_message();

/*
 * Função principal que coordena os dois signals utilizados (SINGINT e SINGALARM)
*/
void sig_handler(int signum);

/*
 * Chamada de função para os client, proxy e server aquando da realização de um SIGINT
*/
void ctrlC_other();
#endif