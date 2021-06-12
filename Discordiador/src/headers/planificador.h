#ifndef PLANIFICADOR_DISCORDIADOR_H
#define PLANIFICADOR_DISCORDIADOR_H

    #include<stdio.h>
    #include<stdlib.h>
    #include<ctype.h>
    #include<pthread.h>
    #include<commons/log.h>
    #include<commons/string.h>
    #include<commons/config.h>
    #include<commons/collections/queue.h>
    #include<commons/collections/list.h>
    #include<readline/readline.h>
    #include<sys/socket.h>
    #include<unnamed/socket.h>
    #include<unnamed/serialization.h>

    int conexion_RAM;
    int conexion_IMS;
    char* ip_RAM;
    char* puerto_RAM;
    char* ip_IMS;
    char* puerto_IMS;
    int grado_multitarea;
    char* algoritmo;
    int quantum_RR;
    int duracion_sabotaje;
    int ciclo_CPU;

    int validador;
    int planificacion_pausada;

    pthread_t hNewaReady;
    pthread_t hReadyaExec;
    /*pthread_t hExecaBloqEmer;
    pthread_t hExecaBloqIO;
    pthread_t hBloqEmeraReady;
    pthread_t hBloqIOaReady;
    pthread_t hExecaExit;*/

    pthread_mutex_t mutexNew;
    pthread_mutex_t mutexReady;
    pthread_mutex_t mutexExec;
    pthread_mutex_t mutexBloqIO;
    pthread_mutex_t mutexBloqEmer;
    pthread_mutex_t mutexExit;

    char** parametros;

    int contadorPCBs;

    #define CANTTAREAS 6    

    t_queue* cola_new;
    t_queue* ready;
    t_queue* exec;
    t_queue* bloq_io;
    t_queue* bloq_emer;
    t_queue* cola_exit;
    typedef struct
    {
        int tid; //Identificador del Tripulante
        int pid; //Identificador de la Patota
        char status; //Estado del Tripulante
        int posicionX; // Posición del tripulante en el Eje X
        int posicionY; // Posición del tripulante en el Eje Y
        char* instruccion_actual; // Nombre de la tarea que estamos ejecutando
    }tcb;
    typedef struct
    {
        int pid; //Identificador de la Patota
        char* rutaTareas; //Ruta del archivo de tareas
        t_list* listaTCB;
    }pcb;

    t_list* listaPCB;

    void funcionTripulante (t_log* logger, tcb* tcbTripulante);
    void funcionhNewaReady (t_log* logger);
    void funcionhReadyaExec (t_log* logger);
    void funcionPlanificador(t_log* logger);
    void send_tareas(int id_pcb, char *ruta_archivo, int conexion_RAM, t_log* logger);
    tcb* crear_TCB(int idP, int posX, int posY, int idT, char* tarea, t_log* logger);
    pcb* crear_PCB(char** parametros, int conexion_RAM, t_log* logger);

#endif