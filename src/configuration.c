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




char *trimwhitespace(char *str)
{
  char *end;

  // Trim leading space
  while(isspace((unsigned char)*str)) str++;

  if(*str == 0)  // All spaces?
    return str;

  // Trim trailing space
  end = str + strlen(str) - 1;
  while(end > str && isspace((unsigned char)*end)) end--;

  // Write new null terminator character
  end[1] = '\0';

  return str;
}

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
            char *token = strtok(line, ".");
            sprintf(filename, "./bin/%s\n", token);
            char *trimmedFileName = trimwhitespace(filename);

            file_log_begin(trimmedFileName);
        //} else if(i == 6){
            // char buffer[50];
            // sprintf(buffer, "./bin/%s", line);
            // printf("Nome ficheiro statistics: %s\n", buffer);
            // statistic_file = fopen(buffer, "a");
            // if(statistic_file == NULL){
            //     perror("creating statistic_file");
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



    



