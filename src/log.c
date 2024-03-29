/**
 * Grupo: SO-023
 * Francisco Martins nº 51073
 * Filipe Pedroso nº 51958
 * Tiago Lourenço nº 46670
*/

#include "log.h"
#include <stdio.h>
#include <stdlib.h>

FILE *log_file;

void file_log_begin(char* name){
    if ((log_file = fopen(name, "w")) == NULL){
        perror(name);
        exit(1);
    }
}

void write_file_log(char* data, char* instruction, int argument){
    char arr[50];
    if(argument == -1){
        sprintf(arr, "%s %s\n", data, instruction);
    }else {
        sprintf(arr, "%s %s %d\n", data, instruction, argument);
    }

    fputs(arr, log_file);
}


void file_destroy_log(){
    fclose(log_file);
}
