#ifndef ATENDER_TRIPULANTES_H
#define ATENDER_TRIPULANTES_H

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
    #include <commons/bitarray.h>
    #include "tareas.h"


    void handler(int client, char* identificador, int comando, void* payload, t_log* logger);
    

#endif