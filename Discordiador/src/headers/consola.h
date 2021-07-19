#ifndef CONSOLA_DISCORDIADOR_H
#define CONSOLA_DISCORDIADOR_H

    #include<stdio.h>
    #include<stdlib.h>
    #include<ctype.h>
    #include<commons/log.h>
    #include<commons/string.h>
    #include<commons/config.h>
    #include<commons/temporal.h>
    #include<readline/readline.h>
    #include<sys/socket.h>
    #include<unnamed/socket.h>
    #include<unnamed/serialization.h>
    #include <readline/history.h>

    #include"planificador.h"

    enum tipo_mensaje_consola {
        C_INICIAR_PLANIFICACION,
        C_PAUSAR_PLANIFICACION,
        C_INICIAR_PATOTA,
        C_LISTAR_TRIPULANTES,
        C_EXPULSAR_TRIPULANTE,
        C_OBTENER_BITACORA,
        C_SALIR
    };


    void funcionConsola();
    
    int hayTripulantesNave();
    int obtener_tipo_mensaje_consola(char *mensaje);

    void mostrarTripulante(void* elemento);
    void mostrarListaTripulantes(void* elemento);
    bool buscarTripulante (void* elemento);
    
    void iterar_en_lista(t_list* self, void(*closure)(void*, t_list*, pthread_mutex_t, int), pthread_mutex_t mutexCola);
    
    void extraerTripulante (void* nodo, t_list* cola, pthread_mutex_t mutexCola, int posicion);
    void expulsarNodo (t_queue* cola, char* nombre_cola, pthread_mutex_t mutexCola);

    tcb* obtener_tcb_en_listaPCB(t_list* self);
    
    void liberarMemoria();


#endif