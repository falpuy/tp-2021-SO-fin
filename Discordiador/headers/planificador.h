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

    #include "patotas.h"

    void funcionPlanificador(t_log* logger);

    void funcionhNewaReady (t_log* logger);
    void funcionhReadyaExec (t_log* logger);

    void funcionCambioExecIO(void* nodo);
    void funcionhExecaBloqIO (t_log* logger);

    void funcionCambioExecReady(void* nodo);
    void funcionhExecaReady (t_log* logger);

    void funcionCambioExecExit(void* nodo);
    void funcionhExecaExit (t_log* logger);

    void funcionhBloqIO (t_log* logger);
    void funcionContadorEnBloqIO(void* nodo);

    void funcionhExecReadyaBloqEmer (t_log* logger);
    void funcionhBloqEmeraReady (t_log* logger);

    void funcionhExit (t_log* logger);

    /*------------------ADICIONALES--------------------*/
    void terminaUnCiclo(void* item);
    void eliminarPatotaEnRAM(void* item);
    void deletearTripulante(void* nodo);
    void eliminarPatotaEnRAM(void* item);
    void signalHilosTripulantes(void *nodo);
    int list_iterate_obtener_posicion(t_list* self, int tid);
    int list_iterate_todos_terminaron(t_list* self);
    
#endif