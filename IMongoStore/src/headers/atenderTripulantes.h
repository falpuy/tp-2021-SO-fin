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

    typedef struct{
        char* puntoMontaje;
        char* puerto;
        int tiempoSincronizacion;
        uint32_t cantidadBloques;
        uint32_t tamanioBloque;    
    }configIMS;

    configIMS* datosConfig;
    
    char* path(char* str);
    void handler(int client, char* identificador, int comando, void* payload, t_log* logger);
    int cantidad_bloques(char* string);

#endif