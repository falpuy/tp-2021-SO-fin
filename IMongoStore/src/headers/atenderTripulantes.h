#ifndef ATENDER_TRIPULANTES_H
#define ATENDER_TRIPULANTES_H

    #include <stdio.h>
    #include <stdlib.h>
    #include <commons/string.h>
    #include <string.h>
    #include <unnamed/serialization.h>
    #include <unnamed/select.h>
    #include <unnamed/socket.h>

    
    enum COMANDOS{
        MOVER_TRIPULANTE = 1,
        COMIENZA_EJECUCION_TAREA = 2,
        FINALIZA_TAREA = 3,
        ATENDER_SABOTAJE = 4,
        RESUELTO_SABOTAJE = 5
    };

    enum TAREAS_TRIPULANTE{
        GENERAR_OXIGENO,
        CONSUMIR_OXIGENO,
        GENERAR_COMIDA,
        GENERAR_BASURA,
        DESCARTAR_BASURA   
    };
    
    
    
    void handler(int client, char* identificador, int comando, void* payload, t_log* logger);


#endif