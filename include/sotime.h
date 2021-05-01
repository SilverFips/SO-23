/**
 * Grupo: SO-023
 * Francisco Martins nº 51073
 * Filipe Pedroso nº 51958
 * Tiago Lourenço nº 46670
*/

#ifndef SOTIME_H_GUARD
#define SOTIME_H_GUARD

/*
 * Retorna o tempo no preciso momento
*/
struct timespec getTime( struct timespec time);

/*
 * Passa a string (no formato data horas) do tempo dado
*/
char* toString(struct timespec time, char *string);




#endif