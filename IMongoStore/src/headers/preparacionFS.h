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
    #include <sys/types.h>
    #include <dirent.h>
    #include "atenderTripulantes.h"

    int superBloque;
    int blocks;
    int arch_bitmap;

    void* sb_memoria;
    void* blocks_memory;
    void* bitmap_memory;

    t_bitarray* bitmap;

    int err;

    void inicializacionFS(t_log* log);
    void validarDirectorioFS(t_log* log);
    void generarBitmap(t_log* log);
    void validarBlocks(t_log* log);
    void validarSuperBloque(t_log* log);
    void mapearBlocks(t_log* log);



#endif