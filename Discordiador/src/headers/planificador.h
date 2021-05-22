#ifndef PLANIFICADOR_DISCORDIADOR_H
#define PLANIFICADOR_DISCORDIADOR_H

    #include<stdio.h>
    #include<stdlib.h>
    #include<ctype.h>
    #include<commons/log.h>
    #include<commons/string.h>
    #include<commons/config.h>
    #include<commons/collections/queue.h>
    #include<commons/collections/list.h>
    #include<readline/readline.h>
    #include<sys/socket.h>
    #include<unnamed/socket.h>

    t_queue* NEW;
    t_queue* READY;
    t_queue* EXEC;
    t_queue* BLOQ_IO;
    t_queue* BLOQ_EMER;
    t_queue* EXIT;
    typedef struct
        {
        int TID = 0; //Identificador del Tripulante
        int PID = 0; //Identificador de la Patota
        char status; //Estado del Tripulante
        int posicionX = 0; // Posición del tripulante en el Eje X
        int posicionY = 0; // Posición del tripulante en el Eje Y
        char* instruccion_actual; // Nombre de la tarea que estamos ejecutando
        }TCB;
    typedef struct
        {
        int PID = 0; //Identificador de la Patota
        char* rutaTareas; //Ruta del archivo de tareas
        t_list* listaTCB;
        }PCB;

    t_list* listaPCB;

    int contadorPCBs = 0;
    void funcionPlanificador(t_log* logger);
    void send_tareas(int id_pcb, char *ruta_archivo, int conexion_RAM, t_log* logger);
    TCB* crear_TCB(int idP, int posX, int posY, int idT, char* tarea);
    //PCB* crear_PCB(char** parametros, int conexion_RAM, int conexion_IMS);

#endif