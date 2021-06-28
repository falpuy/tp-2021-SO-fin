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

    void funcionCambioExecIO(void* nodo, int posicion);
    void funcionhExecaBloqIO (t_log* logger);
    
    void funcionhBloqIO (t_log* logger);
    void funcionContadorEnBloqIO(void* nodo, int posicion);

    void funcionCambioExecReady(void* nodo, int posicion);
    void funcionhExecaReady (t_log* logger);

    void funcionCambioExecExit(void* nodo, int posicion);
    void funcionhExecaExit (t_log* logger);

    void funcionhExit (t_log* logger);

    /*------------------ADICIONALES--------------------*/
    void signalHilosTripulantes(void *nodo);
    void list_iterate_position(t_list *self, void(*closure)());
#endif