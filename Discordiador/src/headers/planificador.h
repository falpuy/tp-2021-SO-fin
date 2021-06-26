#ifndef PLANIFICADOR_DISCORDIADOR_H
#define PLANIFICADOR_DISCORDIADOR_H

    #include<stdio.h>
    #include<stdlib.h>
    #include<stdbool.h>
    #include<ctype.h>
    #include<pthread.h>
    #include<semaphore.h>
    #include<commons/log.h>
    #include<commons/string.h>
    #include<commons/config.h>
    #include<commons/collections/queue.h>
    #include<commons/collections/list.h>
    #include<readline/readline.h>
    #include<sys/socket.h>
    #include<unnamed/socket.h>
    #include<unnamed/serialization.h>

    #include "sabotaje.h"

    enum comandos {
        SUCCESS=200,
        RECIBIR_UBICACION_TRIPULANTE=510,
        ENVIAR_TAREA=520,
        EXPULSAR_TRIPULANTE=530,
        ERROR_CANTIDAD_TRIPULANTES=554,
        ERROR_POR_FALTA_DE_MEMORIA=555,
        ERROR_NO_HAY_TAREAS=560,
        INICIAR_PATOTA=610,
        ENVIAR_OBTENER_BITACORA=760,
        MOVER_TRIPULANTE=761,
        COMENZAR_EJECUCION_TAREA=762,
        FINALIZAR_EJECUCION_TAREA=763,
        RESPUESTA_OBTENER_BITACORA=766,
        INICIO_DISCORDIADOR=770,
        INVOCAR_FSCK=771
    };

    

    void funcionTripulante (void* item);
    void funcionhNewaReady (t_log* logger);
    void funcionhReadyaExec (t_log* logger);
    void funcionCambioExecIO(void* nodo, int posicion);
    void list_iterate_position(t_list *self, void(*closure)());
    void funcionhExecaBloqIO (t_log* logger);
    void funcionhBloqEmeraReady (t_log* logger);
    void funcionhBloqIOaReady (t_log* logger);
    void funcionhExecaExit (t_log* logger);
    void funcionPlanificador(t_log* logger);
    void send_tareas(int id_pcb, char *ruta_archivo, int conexion_RAM, t_log* logger);
    tcb* crear_TCB(int idP, int posX, int posY, int idT, t_log* logger);
    pcb* crear_PCB(char** parametros, int conexion_RAM, t_log* logger);

#endif