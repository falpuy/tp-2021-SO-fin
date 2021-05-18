#ifndef PREPARACION_FS_H
#define PREPARACION_FS_H

    #include <stdio.h>
    #include <stdlib.h>
    #include <commons/log.h>
    #include <commons/config.h>
    #include <string.h>
    #include <unnamed/validation.h>
    #include <unistd.h>
    #include <commons/string.h>
    #include <stdint.h>
    #include <commons/bitarray.h>
    #include <sys/mman.h>
    #include <sys/stat.h>
    #include <fcntl.h>
    #include <stdarg.h>

    typedef struct{
        char* puntoMontaje;
        char* puerto;
        int tiempoSincronizacion;
    }configIMS;

    configIMS* datosConfig;
    void* p_bitmap;

    uint32_t cantidadBloques;
    uint32_t tamanioBloque;
    int flag_existiaSB;
    
    FILE* arch_bloques;
    FILE* superBloque;


    void inicializacionFS(t_log* log);
    void generarBitmap(t_log* log);
    void guardarEspacioBitmap(t_log* log);
    void validarBlocks(t_log* log);
    void validarSuperBloque(t_log* log);


#endif