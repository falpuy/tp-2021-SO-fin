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

    uint32_t tamanioBloque;
    uint32_t cantidadBloques;

    char* crearNuevaListaBloques(char* listaBloques, int bloqueAgregar);
    void actualizarBlocks(t_config* metadataBitacora,int bloque,t_log*log);
    void actualizarBlockCount(t_config* metadataBitacora,t_log*log);
    void actualizarSize(t_config* metadataBitacora,t_log*log);
    int setearMD5(char* pathMetadata, t_log* log);
    
#endif