/**
 * Grupo: SO-023
 * Francisco Martins nº 51073
 * Filipe Pedroso nº 51958
 * Tiago Lourenço nº 46670
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "main.h"
#include "log.h"
#include <ctype.h> // for isspace on trimwhitespace

/*
* Reads from the file that receives the program arguments and writes them into the main_data structure
* Calls the functions to create the log and statistic files
*/

int open_begin_file(char* name_file, struct main_data* data){
    FILE *file;
    
    //FILE *statistic_file;
    file = fopen(name_file, "r");
    if(file == NULL){
        perror("abrir ficheiro");
        exit(1);
    }
    char line[42];
    int i = 0;
    while(fgets(line, sizeof(line), file) != NULL){
        if(i == 0){
            data->max_ops = atoi(line);
        } else if(i == 1){
            data->buffers_size = atoi(line);
        } else if(i == 2){
            data->n_clients = atoi(line);
        } else if(i == 3){
            data->n_proxies = atoi(line);
        } else if(i == 4){
            data->n_servers = atoi(line);
        } else if(i == 5){
            char filename[256];
            char file[256]; 
            sscanf(line, "%s", file);
            sprintf(filename, "./bin/%s", file);
            file_log_begin(filename);
        //} else if(i == 6){
            // char filename[50];
            // char file[50]; 
            // sscanf(line, "%s", file);
            // sprintf(filename, "./bin/%s", file);
            // file_stats_begin(filename);
        //     }
        } else if( i == 7){
            data->alarm = atoi(line);
            break;
        }
        i++;
    }
    fclose(file);
    

   
    return 0;
}



    



