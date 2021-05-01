/**
 * Grupo: SO-023
 * Francisco Martins nº 51073
 * Filipe Pedroso nº 51958
 * Tiago Lourenço nº 46670
*/

#ifndef CONFIGUTARION_H_GUARD
#define CONFIGURATION_H_GUARD


#include "main.h"

/*
* Reads from the file that receives the program arguments and writes them into the main_data structure
* Calls the functions to create the log and statistic files
*/
int open_begin_file(char* name_file, struct main_data* data);

#endif