#ifndef PATOTAS_H
#define PATOTAS_H

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

    #include "sabotaje.h"
    
    tcb* crear_TCB(int idP, int posX, int posY, int idT, t_log* logger);
    pcb* crear_PCB(char** parametros, int conexion_RAM, t_log* logger);

    void destruirTCB(void* nodo);
    void destruirPCB(void* nodo);

    void funcionTripulante (void* item);

    bool llegoAPosicion(int tripulante_posX,int tripulante_posY,int posX, int posY);
    void pedirProximaTarea(tcb* tcbTripulante);
    int esTareaIO(char *tarea);
    void moverTripulanteUno(tcb* tcbTrip, int posXfinal, int posYfinal);


    char *get_tareas(char *ruta_archivo, t_log* logger);

    void create_tcb_by_list(t_list* self, void(*closure)(void*, int, int, t_log*), int conexion_RAM, int cantidad_inicial, t_log *logger);
    void iniciar_tcb(void *elemento, int conexion_RAM, int indice_tcb_temporal, t_log *logger);

    void * get_by_id(t_list * self, int id);
    void _signal(int incremento, int valorMax, sem_t semaforo);
#endif