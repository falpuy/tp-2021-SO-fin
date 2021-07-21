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

    void inicializacionFS();
    void validarDirectorioFS();
    void validarBlocks();
    void validarSuperBloque();

#endif