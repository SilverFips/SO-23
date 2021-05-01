/**
 * Grupo: SO-023
 * Francisco Martins nº 51073
 * Filipe Pedroso nº 51958
 * Tiago Lourenço nº 46670
*/

#include "stats.h"
#include <stdio.h>
#include <stdlib.h>
#include "memory.h"
#include "sotime.h"

FILE *stats_file;

void file_stats_begin(char* name){
    if ((stats_file = fopen(name, "w")) == NULL){ // porque não modo w?
        perror(name);
        exit(1);
    }
}

void file_destroy_stats(){
    fclose(stats_file);
}

void write_stats_file(struct main_data* data, struct semaphores* sems){
    int clientes = data->n_clients;
	int proxies = data->n_proxies;
	int servers = data->n_servers;
    char arr[50];
    fputs("Process Statistics:\n", stats_file);
	for(int i = 0; i < clientes; i++){					
		sprintf(arr, "      Client %d recebeu %d pedidos!\n",i ,data->client_stats[i]);
        fputs(arr, stats_file);
	}

	for(int i = 0; i < proxies; i++){
		sprintf(arr, "      Proxy %d encaminhou %d pedidos!\n",i ,data->proxy_stats[i]);
        fputs(arr, stats_file);
	}

	for(int i = 0; i < servers; i++){
		sprintf(arr, "      Server %d respondeu %d pedidos!\n",i ,data->server_stats[i]);
        fputs(arr, stats_file);
	}

    semaphore_mutex_lock(sems->results_mutex);

    fputs("\nOperation Statistics:\n", stats_file);
    struct operation* ops = data->results;
    int count = 0;
    int max_ops = data->max_ops;
    for(int i = 0; i < max_ops; i++){
        struct operation op = ops[i];
        if(op.status == 'S'){
            count++;
            sprintf(arr, "OP: %d\n", op.id);
            fputs(arr, stats_file);
            sprintf(arr, "Status: %c\n", op.status);
            fputs(arr, stats_file);
            sprintf(arr, "Client_id: %d\n", op.client);
            fputs(arr, stats_file);
            sprintf(arr, "Proxy_id: %d\n", op.proxy);
            fputs(arr, stats_file);
            sprintf(arr, "Server_id: %d\n", op.server);
            fputs(arr, stats_file);

            char times[50];
            toString(op.start_time, times);
            sprintf(arr, "Created: %s\n", times);
            fputs(arr, stats_file);
        
            toString(op.client_time, times);
            sprintf(arr, "Client_time: %s\n", times);
            fputs(arr, stats_file);
        
            toString(op.proxy_time, times);
            sprintf(arr, "Proxy_time: %s\n", times);
            fputs(arr, stats_file);
        
            toString(op.server_time, times);
            sprintf(arr, "Server_time: %s\n", times);
            fputs(arr, stats_file);
        
            toString(op.end_time, times);
            sprintf(arr, "Ended: %s\n", times);
            fputs(arr, stats_file);

            double total_time = (op.end_time.tv_sec - op.start_time.tv_sec) + (op.end_time.tv_nsec - op.start_time.tv_nsec) / 1000000000L;
            sprintf(arr, "Total Time: %lf\n", total_time);
            fputs(arr, stats_file);

            fputs("\n", stats_file);
        }
        
    }
    if(count == 0){
        fputs("Não foi feita nenhuma operação", stats_file);
    }

    semaphore_mutex_unlock(sems->results_mutex);

}