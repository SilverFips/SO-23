/**
 * Grupo: SO-023
 * Francisco Martins nº 51073
 * Filipe Pedroso nº 51958
 * Tiago Lourenço nº 46670
*/

#ifndef STATS_H_GUARD
#define STATS_H_GUARD

#include "main.h"

void file_stats_begin(char* name);

void file_destroy_stats();

void write_stats_file(struct main_data* data, struct semaphores* sems);

#endif