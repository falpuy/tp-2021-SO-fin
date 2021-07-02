#ifndef SABOTAJES_H
#define SABOTAJES_H

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
    #include <signal.h>
    #include <unnamed/serialization.h>
    
     enum COMANDOS{
        OBTENER_BITACORA = 760 ,
        MOVER_TRIPULANTE ,
        COMIENZA_EJECUCION_TAREA,
        FINALIZA_TAREA,
        ATENDER_SABOTAJE,
        RESUELTO_SABOTAJE,
        RESPUESTA_OBTENER_BITACORA,
        ESPERANDO_SABOTAJE,
        COMIENZA_SABOTAJE,
        ATIENDE_SABOTAJE,
        INVOCAR_FSK = 771

    };

    typedef struct{
        char* puntoMontaje;
        char* puerto;
        int tiempoSincronizacion;  
        char** posicionesSabotaje; 
    }configIMS;
    configIMS* datosConfig;

    pthread_t sync_blocks;

    uint32_t tamanioBloque;
    uint32_t cantidadBloques;

    int flagEnd;
    int contadorListaSabotajes;
    int socketDiscordiador;
    int testeoIDTripulante;

    void* copiaBlocks;
    void* copiaSB;
    void* memBitmap;
    
    t_bitarray* bitmap;
    t_config* config;
    t_log* logger;

    pthread_mutex_t blocks_bitmap;
    pthread_mutex_t logMutex;
    pthread_mutex_t m_superBloque;
    pthread_mutex_t m_metadata;
    pthread_mutex_t discordiador;

    void sabotaje();
    void protocolo_fsck();
    
#endif