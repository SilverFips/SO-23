/**
 * Grupo: SO-023
 * Francisco Martins nº 51073
 * Filipe Pedroso nº 51958
 * Tiago Lourenço nº 46670
*/


#include <stdio.h>
#include <stdlib.h>
#include "main.h"
#include "log.h"
/*
* Reads from the file that receives the arguments into the main_data struct
* Creates the logfile, statisticfile
*/
int open_begin_file(char* name_file, struct main_data* data){
    FILE *file;
    
    FILE *statistic_file;
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
            printf("\nmax ops: %d\n", data->max_ops);
        } else if(i == 1){
            data->buffers_size = atoi(line);
            printf("\nbuffer_size: %d\n", data->buffers_size);
        } else if(i == 2){
            data->n_clients = atoi(line);
            printf("\nn_clients: %d \n", data->n_clients);
        } else if(i == 3){
            data->n_proxies = atoi(line);
        } else if(i == 4){
            data->n_servers = atoi(line);
        } else if(i == 5){
            char buffer[50];
            sprintf(buffer, "./bin/%s", line);
            printf("Nome ficheiro log: %s\n", buffer);
            file_log_begin(buffer);
        } else if(i == 6){
            char buffer[50];
            sprintf(buffer, "./bin/%s", line);
            printf("Nome ficheiro statistics: %s\n", buffer);
            statistic_file = fopen(buffer, "a");
            if(statistic_file == NULL){
                perror("creating statistic_file");
            }
        } else if( i == 7){
            data->alarm = atoi(line);
            break;
        }
        i++;
    }
    fclose(file);
    printf("%d\n", i);

    printf("max %d, size %d, clients %d, proxies  %d, servers %d\n", data->max_ops, data->buffers_size, data->n_clients, data->n_proxies, data->n_servers); 
    return 0;
}
    



