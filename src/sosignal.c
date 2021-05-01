/**
 * Grupo: SO-023
 * Francisco Martins nº 51073
 * Filipe Pedroso nº 51958
 * Tiago Lourenço nº 46670
*/

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>


#include "sosignal.h"
#include "main.h"
#include "synchronization.h"

struct main_data* data;
struct communication_buffers* buffers;
struct semaphores* sems;
int pass;

void dados(struct communication_buffers* buffers, struct main_data* data, struct semaphores* sems){
    data = data;
    buffers = buffers;
    sems = sems;
    pass = 0;
}

void sig_handler(int signum){
    if(signum == SIGINT){                       // signal handler for SIGINT 
        ctrlC();
        pass = 1;
    }
    if(signum == SIGALRM && pass != 1){         //signal handler for SIGALRM
        alarm_message(signum);
    }
    
}


void ctrlC(){
    stop_execution(data,buffers, sems);
}

void ctrlC_other(){}


void alarm_message(int signum) {
    int max = data->max_ops;
    semaphore_mutex_lock(sems->results_mutex);

    printf("\n/////////////////// ALARME /////////////////////\n");
    for(int i = 0; i < max; i++){
        struct operation op = data->results[i];
        char status = op.status;
        
        if(status == 'S'){
            printf("(-) op: %d status:%c start:%ld client:%d client_time:%ld proxy:%d proxy_time:%ld server:%d server_time:%ld end:%ld\n", 
            op.id, op.status, op.start_time.tv_sec, op.client, op.client_time.tv_sec, op.proxy, op.proxy_time.tv_sec, op.server, op.server_time.tv_sec, op.end_time.tv_sec);
        }else{
            printf("(-) op:%d status: %d\n", i, 0);
        }
    }
    printf("////////////////////////////////////////////////\n");
    semaphore_mutex_unlock(sems->results_mutex);
    alarm(data->alarm);
}

		


