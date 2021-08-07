#ifndef ACTUALIZAR_METADATA_H
#define ACTUALIZAR_METADATA_H

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
    #include "syncFile.h"

    #include <commons/bitarray.h>

    
    void crearMetadataBitacora(char* path_tripulante);
    void crearMetadataFiles(char* path,char* charLlenado);
    
    
    char* strMoverTripultante(int idTripulante,int posX_v,int posY_v,int posX_n,int posY_n);
    
    char* obtenerBitacora(int tripulante);
    int cantidadBloquesUsados(char** listaBloques);
    void liberarArray (char** array, int posiciones);

    
#endif