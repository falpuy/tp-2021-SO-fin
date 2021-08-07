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
    #include <math.h>

    enum comandos {
        SUCCESS=200,
        ENVIAR_CAMBIO_DE_ESTADO=501,
        ENVIAR_UBICACION_TRIPULANTE=502,
        ENVIAR_TAREA=520,
        EXPULSAR_TRIPULANTE=530,
        ERROR_CANTIDAD_TRIPULANTES=554,
        ERROR_POR_FALTA_DE_MEMORIA=555,
        ERROR_NO_HAY_TAREAS=560,
        INICIAR_PATOTA=500,
        ELIMINAR_PATOTA=599,
        ENVIAR_OBTENER_BITACORA=760,
        MOVER_TRIPULANTE=761,
        COMENZAR_EJECUCION_TAREA=762,
        FINALIZAR_EJECUCION_TAREA=763,
        ATIENDE_SABOTAJE=764,
        RESPUESTA_OBTENER_BITACORA=766,
        COMIENZA_SABOTAJE=767,
        INVOCAR_FSCK=771,
        RESOLUCION_SABOTAJE=772
    };

    //ESTRUCTURAS

    typedef struct{
        char* puertoDiscordiador;
        t_log* loggerDiscordiador;
    }parametrosServer;

    typedef struct{
        int conexion_RAM;
        int conexion_IMS;
        t_log* loggerDiscordiador;
    }parametrosConsola;

    typedef struct{
        int idSemaforo;
    }parametrosThread;

    typedef struct{
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
        int mensajeInicialIMS;
        int cicloCPUCumplido;
    }tcb;

    typedef struct{
        int pid; //Identificador de la Patota
        int todosLosTCBsTerminaron;
        char* rutaTareas; //Ruta del archivo de tareas
        t_list* listaTCB;
    }pcb;

    //-----------------------------------------------------------
    t_log* logger;
    t_config* config;

    // int conexion_RAM;
    // int conexion_IMS;
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
    char* buffer;

    int validador;
    int planificacion_viva;
    int sabotaje_activado;
    int ciclos_transcurridos_sabotaje;
    int sabotaje_terminado;
    int cantidadTCBEnExec;

    tcb *tripulanteFixer;
    
    char** parametros;

    //------------------HILOS - MUTEX - SEMAFOROS------------------- 

    pthread_t hNewaReady;
    pthread_t hReadyaExec;
    pthread_t hExecaReady;
    pthread_t* hiloTripulante;
    pthread_t hExecaBloqIO;
    pthread_t hExecaExit;
    pthread_t hBloqIO;
    pthread_t hExit;  
    pthread_t hEsperarSabotaje;
    pthread_t hExecReadyaBloqEmer;
    pthread_t hBloqEmeraReady;
    pthread_t hFixerdeEmeraReady;

    pthread_mutex_t mutexNew;
    pthread_mutex_t mutexReady;
    pthread_mutex_t mutexExec;
    pthread_mutex_t mutexBloqIO;
    pthread_mutex_t mutexBloqEmer;
    pthread_mutex_t mutexExit;
    pthread_mutex_t mutexBloqEmerSorted;
    pthread_mutex_t mutexValidador;
    pthread_mutex_t mutexPlanificacionViva;
    pthread_mutex_t mutexSabotajeActivado;
    pthread_mutex_t mutexCiclosTranscurridosSabotaje;
    pthread_mutex_t mutexListaPCB;
    pthread_mutex_t mutex_cantidadTCB;
    pthread_mutex_t mutex_cantidadVieja;
    pthread_mutex_t mutex_cantidadActual;
    pthread_mutex_t mutexBuffer;
    pthread_mutex_t mutexSemaforosTrip;
    pthread_mutex_t mutexValidacionPos;
    pthread_mutex_t mutex_contadorSemGlobal;
    pthread_mutex_t mutexContextoSabotaje;
    pthread_mutex_t mutexBuffersAEnviar;

    sem_t semNR;
    sem_t semRE;
    sem_t semER;
    sem_t semEaX;
    sem_t semBLOCKIO;
    sem_t semEXIT;
    sem_t semEBIO;
    sem_t* semTripulantes;
    sem_t semERM;
    sem_t semMR;
    sem_t pausar;

    int cantidadVieja;
    int cantidadActual;
    int contadorSemGlobal;
    
    int contadorPCBs;
    int cantidadTCBTotales;

    int posSabotajeX;
    int posSabotajeY;

    int contadorCicloCPU;
    int primerCicloSabotaje;

    t_queue* cola_new;
    t_queue* ready;
    t_queue* exec;
    t_queue* bloq_io;
    t_queue* bloq_emer;
    t_queue* bloq_emer_sorted;
    t_queue* cola_exit;
    t_queue* colaContSab;
    t_queue* buffersAEnviar;

    t_list* lista_parametros;
    t_list* listaPCB;
    t_list* listaSemaforos;

    void handler(int client, char* identificador, int comando, void* payload, t_log* logger);
    bool comparadorTid(void* tripulante1, void* tripulante2);
    
    bool ordenarMasCercano(void* tripulante1, void* tripulante2);
    bool compDistancias(void* tripulante1, void* tripulante2);


#endif