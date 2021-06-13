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
    #include "tareas.h"
    
    enum COMANDOS{
        OBTENER_BITACORA = 760 ,
        MOVER_TRIPULANTE ,
        COMIENZA_EJECUCION_TAREA,
        FINALIZA_TAREA,
        ATENDER_SABOTAJE,
        RESUELTO_SABOTAJE
    };
    
    char* str_para_blocks;
    
    void handler(int client, char* identificador, int comando, void* payload, t_log* logger);
    

#endif