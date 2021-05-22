#ifndef SYNC_FILE
#define SYNC_FILE

    #include <stdio.h>
    #include <stdlib.h>
    #include <sys/mman.h>
    #include <pthread.h>
    #include <unistd.h>
    #include <commons/log.h>
    #include <stdint.h>

    typedef struct{
        char* puntoMontaje;
        char* puerto;
        int tiempoSincronizacion;
        uint32_t cantidadBloques;
        uint32_t tamanioBloque;    
    }configIMS;

    uint32_t cantidad;
    uint32_t tamanio; 


    int flagEnd;
    configIMS* datosConfig;
    char* mapArchivo;

    pthread_mutex_t mutexBlocks;
    pthread_mutex_t mutexSuperBloque;

    void actualizarArchivo(t_log* log);

#endif