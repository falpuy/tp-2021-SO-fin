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
    #include <commons/string.h>
    #include <sys/stat.h>
    #include <fcntl.h>
    #include <string.h>
    #include <commons/config.h> 
    #include <signal.h>
    #include <unnamed/serialization.h>
    #include <unnamed/socket.h>
    #include <string.h>
    #include <commons/bitarray.h>
    #include <sys/stat.h>
    #include <math.h>
    
    enum COMANDOS{
        OBTENER_BITACORA = 760 ,
        MOVER_TRIPULANTE ,
        COMIENZA_EJECUCION_TAREA,
        FINALIZA_TAREA,
        ATENDER_SABOTAJE,
        RESUELTO_SABOTAJE,
        RESPUESTA_OBTENER_BITACORA,
        COMIENZA_SABOTAJE,
        ESPERANDO_SABOTAJE = 770,
        INVOCAR_FSK = 771,
        ERROR_NO_EXISTE_BITACORA = 773
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
    int contadorSabotajeLeido;

    char** posicionesSabotajes;
    char* ipDiscordiador;
    char* puertoDiscordiador;

    void* copiaBlocks;
    void* sb_memoria;
    // void* copiaSB;
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
    void validacionFiles();
    void validarCantidadBloques();
    void validarBitmapSabotaje();

    void corregirBitmap(int encontroVacio,t_bitarray* bitmapFalso, void* copiaSuperBloque);
    void corregirBitmapTripulantes(int encontroVacio,t_bitarray* bitmapFalso, void* copiaSuperBloque);
    void corregirBitmapRecursos(int encontroVacio,t_bitarray* bitmapFalso, void* copiaSuperBloque);
    void validarSizeFile();
    void validarSizeRecurso(char* path);
    void validarBlocksBlockCount();
    void validarBlockCountRecurso(char* path);
    void validacionBlocks();
    void validarBlocksRecursos(char* path);
    char* crearNuevaListaBloques(char* listaVieja,int bloqueAgregar, int flagEsGuardar,char* path);
    int cantidad_bloques(char* string);    
    int validarBitsLibre(int cantidadBloquesAUsar);
    void validacionEsValidaLista();
    void guardarPorBloque(char* stringGuardar,int posEnString, int cantidadBloquesAUsar,char* path,int esRecurso, int flagEsGuardar);
    void validacionEsValidaListaRecursos(char* path);
    
    void actualizarBlocks(int bloque,int flagEsGuardar,char* path);
    void setearMD5(char* pathMetadata);
    void actualizarBlockCount(t_config* metadataBitacora,int flagEsGuardar);
    void actualizarSize(t_config* metadataBitacora,int tamanio, int flagEsGuardar);

    char* pathCompleto(char* strConcatenar);
    char* crearStrTripulante(int idTripulante);
    char* queRecurso(char* path);
    
#endif