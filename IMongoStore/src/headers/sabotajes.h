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
    #include <unnamed/socket.h>
    #include <commons/bitarray.h>
    
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
        INVOCAR_FSK = 771
    };

    typedef struct{
        char* puntoMontaje;
        char* puerto;
        int tiempoSincronizacion;  
    }configIMS;
    configIMS* datosConfig;

    pthread_t sync_blocks;

    uint32_t tamanioBloque;
    uint32_t cantidadBloques;

    int flagEnd;
    int socketDiscordiador;
    int testeoIDTripulante;
    int contadorListaSabotajes;

    char** posicionesSabotajes;

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
    pthread_mutex_t validador;

    void sabotaje();
    void protocolo_fsck();
    void validacionSuperBloque();
    void validarCantidadBloques();
    void validarBitmapSabotaje();
    void validacionFiles();
    char* pathCompleto(char* strConcatenar);
    void validacionBitmapRecurso(char* pathRecurso,t_bitarray* bitmapFalso);
    void validacionBitmapTripulantes(t_bitarray* bitmapFalso);
    char* crearStrTripulante(int idTripulante);

    
#endif