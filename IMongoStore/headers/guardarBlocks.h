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


    void guardarEnBlocks(char* stringGuardar,char* path,int esRecurso);
    void borrarEnBlocks(char* stringABorrar,char* path,int esRecurso,char recurso);
    
#endif