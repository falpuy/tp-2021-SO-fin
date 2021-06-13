#ifndef SYNC_FILE
#define SYNC_FILE

    #include <stdio.h>
    #include <stdlib.h>
    #include <sys/mman.h>
    #include <pthread.h>
    #include <unistd.h>
    #include <commons/log.h>
    #include <stdint.h>
    #include <commons/bitarray.h>

    #include "preparacionFS.h"


    
    pthread_mutex_t mutexBlocks;
    pthread_mutex_t mutexSuperBloque;

    void actualizarArchivo(t_log* log);

#endif