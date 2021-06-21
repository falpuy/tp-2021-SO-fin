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
    #include <commons/log.h>
    #include <commons/collections/list.h>
    #include "actualizarMetadata.h"

    void borrarEnBlocks(char* stringABorrar,char* path,int esRecurso);
    void guardarEnBlocks(char* stringGuardar,char* path,int esRecurso);
    void borrarPorBloque(char* stringBorrar,int posEnString, int tamStr, int cantidadBloquesABorrar,char* path,int esRecurso);
    void guardarPorBloque(char* stringGuardar,int posEnString, int cantidadBloquesAUsar,char* path,int esRecurso, int flagEsGuardar);
    
#endif