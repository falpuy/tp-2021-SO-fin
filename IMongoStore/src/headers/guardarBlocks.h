#ifndef GUARDAR_BLOCKS_H
#define GUARDAR_BLOCKS_H

    #include <stdio.h>
    #include <stdlib.h>
    #include <commons/string.h>
    #include <string.h>
    #include <unnamed/serialization.h>
    #include <unnamed/select.h>
    #include <unnamed/socket.h>
    #include <math.h>
    #include <commons/config.h>
    #include "syncFile.h"
    #include <commons/log.h>
    #include "actualizarMetadata.h"
    #include <commons/collections/list.h>

    pthread_mutex_t m_blocks;
    pthread_mutex_t m_bitmap;
    pthread_mutex_t m_superBloque;
    pthread_mutex_t m_metadata;
    
    typedef struct{
        int bloque;
    }nodoBloque;

    void guardarEnBlocks(char* stringGuardar,char* path_fileTripulante,int flag, t_log* logger);
    t_list* crearListaDeBloques(char* listaBloques,int cantidadBloques,t_log* log);
    
#endif