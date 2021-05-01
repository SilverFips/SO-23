/**
 * Grupo: SO-023
 * Francisco Martins nº 51073
 * Filipe Pedroso nº 51958
 * Tiago Lourenço nº 46670
*/

#ifndef LOG_H_GUARD
#define LOG_H_GUARD


/**
 * Inicializa um ficheiro novo com o nome dado
*/
void file_log_begin(char* name);

/*
 * Destroi o ficheiro em questao
*/
void file_destroy_log();

/**
 * Escrita no ficheiro de log com a formatação data-instrução-argumento
*/
void write_file_log(char* data, char* instruction, int argument);

#endif