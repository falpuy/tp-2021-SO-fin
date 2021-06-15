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
    #include <sys/stat.h>
    #include <fcntl.h>
    #include <string.h>
    #include <commons/config.h>

    
    typedef struct{
        char* puntoMontaje;
        char* puerto;
        int tiempoSincronizacion;   
    }configIMS;
    configIMS* datosConfig;

    uint32_t tamanioBloque;
    uint32_t cantidadBloques;

    int flagEnd;

    void* copiaBlocks;
    void* copiaSB;
    void* memBitmap;
    
    t_bitarray* bitmap;
    t_config* config;
    t_log* log;

    pthread_mutex_t m_blocks;
    pthread_mutex_t m_bitmap;
    pthread_mutex_t m_superBloque;
    pthread_mutex_t m_metadata;
    

    void actualizarArchivoBlocks();

#endif