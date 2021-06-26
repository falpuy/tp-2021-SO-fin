#ifndef SABOTAJE_DISCORDIADOR_H
#define SABOTAJE_DISCORDIADOR_H

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
    #include<sys/socket.h>
    #include<unnamed/socket.h>
    #include<unnamed/serialization.h>

    typedef struct{
        char* puertoDiscordiador;
        t_log* loggerDiscordiador;
    }parametrosServer;

    t_log* logger;
    t_config* config;

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
    char* puerto_DIS;

    int validador;
    int planificacion_pausada;
    int sabotaje_activado;

    pthread_t hNewaReady;
    pthread_t hReadyaExec;
    pthread_t hExecaReady;
    pthread_t hExecaBloqIO;
    pthread_t hExecaExit;
    pthread_t hBloqIO;
    pthread_t hExit;  
    pthread_t hEsperarSabotaje;
    pthread_t hExecReadyaBloqEmer;
    pthread_t hBloqEmeraReady;

    pthread_mutex_t mutexNew;
    pthread_mutex_t mutexReady;
    pthread_mutex_t mutexExec;
    pthread_mutex_t mutexBloqIO;
    pthread_mutex_t mutexBloqEmer;
    pthread_mutex_t mutexExit;

    sem_t semNR;
    sem_t semRE;
    sem_t semER;
    sem_t semEaX;
    sem_t semBLOCKIO;
    sem_t semEXIT;
    sem_t semEBIO;
    sem_t* semTripulantes;

    int cantidadActual;
    int cantidadVieja;
    int contadorSemGlobal;
    
    typedef struct{
        t_log* logger;
        int idSemaforo;
    }parametrosThread;

    char** parametros;

    int contadorPCBs;
    int cantidadTCBTotales;

    int posSabotajeX;
    int posSabotajeY;

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
        int estaVivoElHilo; // 1= está vivo, 0= está muerto
        int tiempoEnExec;
        int tiempoEnBloqIO;
        int ciclosCumplidos;
    }tcb;
    typedef struct
    {
        int pid; //Identificador de la Patota
        char* rutaTareas; //Ruta del archivo de tareas
        t_list* listaTCB;
    }pcb;

    t_list* listaPCB;

#endif
