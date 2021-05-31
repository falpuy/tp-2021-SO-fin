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
    #include "syncFile.h"

    
    enum COMANDOS{
        OBTENER_BITACORA = 760 ,
        MOVER_TRIPULANTE ,
        COMIENZA_EJECUCION_TAREA,
        FINALIZA_TAREA,
        ATENDER_SABOTAJE,
        RESUELTO_SABOTAJE
    };

    enum TAREAS_TRIPULANTE{
        GENERAR_OXIGENO,
        CONSUMIR_OXIGENO,
        GENERAR_COMIDA,
        GENERAR_BASURA,
        DESCARTAR_BASURA   

    };

    char* str_para_blocks;
    
    char* pathCompleto(const char* str, ...);
    void handler(int client, char* identificador, int comando, void* payload, t_log* logger);
    int cantidad_bloques(char* string);
    char* funcionStrMover(int posX_v, int posY_v, int posX_n, int posY_n);
    void crearMetadataBitacora(char* path_tripulante);

#endif