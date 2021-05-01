/**
 * Grupo: SO-023
 * Francisco Martins nº 51073
 * Filipe Pedroso nº 51958
 * Tiago Lourenço nº 46670
*/

#include <time.h>
#include <stdlib.h>
#include <stdio.h>

#define NANO_TO_MILLI(N) ((N)/1000000)

struct timespec getTime( struct timespec time){
    if( clock_gettime( CLOCK_REALTIME, &time) == -1 ) {
        perror( "clock gettime" );
    }
    return time;
}


void toString(struct timespec time, char *string){
    
    struct tm* info = localtime(&time.tv_sec);
    time_t milli = NANO_TO_MILLI(time.tv_nsec);
    sprintf(string, "%d-%d-%d %d:%d:%d-%ld", 1900 + info->tm_year, info->tm_mon, info->tm_mday,info->tm_hour, info->tm_min, info->tm_sec,milli);

}