#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unnamed/socket.h>
#include <unnamed/serialization.h>
#include <commons/log.h>
#include <string.h>
#include <unistd.h>
#include <commons/string.h>
#include <stdint.h>
#include <commons/bitarray.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdarg.h>
#include <sys/types.h>
#include <dirent.h>
#include <commons/process.h>
#include <pthread.h>
#include <semaphore.h>
#include <commons/collections/queue.h>
#include <readline/readline.h>
#include <commons/temporal.h>

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
        t_log* logger;
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
    }tcb;

    typedef struct{
        int pid; //Identificador de la Patota
        char* rutaTareas; //Ruta del archivo de tareas
        t_list* listaTCB;
    }pcb;

    //-----------------------------------------------------------
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
    int planificacion_viva;
    int sabotaje_activado;

    //------------------HILOS - MUTEX - SEMAFOROS------------------- 

    pthread_t hiloConsola;
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

    pthread_mutex_t mutexNew;
    pthread_mutex_t mutexReady;
    pthread_mutex_t mutexExec;
    pthread_mutex_t mutexBloqIO;
    pthread_mutex_t mutexBloqEmer;
    pthread_mutex_t mutexExit;
    pthread_mutex_t mutexValidador;
    pthread_mutex_t mutexListaPCB;
    

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

    t_list* listaPCB;

enum tipo_mensaje_consola {
        C_INICIAR_PLANIFICACION,
        C_PAUSAR_PLANIFICACION,
        C_INICIAR_PATOTA,
        C_LISTAR_TRIPULANTES,
        C_EXPULSAR_TRIPULANTE,
        C_OBTENER_BITACORA,
        C_SALIR
};

void funcionConsola(t_log* logger);
int obtener_tipo_mensaje_consola(char *mensaje);
void funcionhNewaReady (t_log* logger);
void mostrarTripulante(void* elemento);
void mostrarListaTripulantes(void* elemento);
/*---------------------------------------MAIN---------------------------------------------*/

int main(){
    logger = log_create("test.log","TEST",1,LOG_LEVEL_INFO);
    setearConfiguraciones();
    funcionConsola(logger);

    return 0;
}


/*---------------------------------------FUNCIONES---------------------------------------------*/
void funcionhNewaReady (t_log* logger) {
    log_info(logger, "HOLAAA");
    pthread_mutex_lock(&mutexValidador);
    int temp_validador = validador;
    pthread_mutex_unlock(&mutexValidador);

    while (temp_validador) {
        sem_wait(&semNR);

        if(planificacion_viva){
            while(!queue_is_empty(cola_new)){   
                
                log_info(logger,"----------------------------------");
                log_info(logger, "Se ejecuta el hilo de New a Ready");
                tcb* aux_TCB = malloc (sizeof(tcb));

                pthread_mutex_lock(&mutexNew);
                aux_TCB = queue_pop(cola_new);
                pthread_mutex_unlock(&mutexNew);

                log_info(logger,"Tripulante encontrado. Moviendolo a Ready...");
                log_info(logger,"TID:%d", aux_TCB->tid);
                log_info(logger,"Status:%c",aux_TCB->status);
                log_info(logger,"Instruccion Actual:%s", aux_TCB->instruccion_actual);

                aux_TCB->status = 'R';
                pthread_mutex_lock(&mutexReady);
                queue_push(ready, (void*) aux_TCB);
                pthread_mutex_unlock(&mutexReady);

                log_info(logger, "Se paso nodo a Ready");

            }

            log_info(logger,"Se hizo una ejecución de CPU en NEW->READY");
            log_info(logger,"----------------------------------");
        }

        sem_post(&semConsola);
    }
}
void setearConfiguraciones (){

    //logger = log_create("consola.log", "consola", 1, LOG_LEVEL_INFO);
    config = config_create("archivo.config"); //Hay que ver cómo se llama el archivo.config cuando nos lo den
    
    ip_RAM = config_get_string_value(config, "IP_MI_RAM_HQ");
    puerto_RAM = config_get_string_value(config, "PUERTO_MI_RAM_HQ");
    ip_IMS = config_get_string_value(config, "IP_I_MONGO_STORE");
    puerto_IMS = config_get_string_value(config, "PUERTO_I_MONGO_STORE");
    grado_multitarea = config_get_int_value (config, "GRADO_MULTITAREA");
    algoritmo = config_get_string_value(config, "ALGORITMO");
    quantum_RR = config_get_int_value (config, "QUANTUM");
    duracion_sabotaje = config_get_int_value (config, "DURACION_SABOTAJE");
    ciclo_CPU = config_get_int_value (config, "RETARDO_CICLO_CPU");
    puerto_DIS = config_get_string_value(config, "PUERTO_DISCORDIADOR");


    log_info(logger, "%s, %s, %s, %s, %d, %s, %d, %d, %d", ip_RAM, puerto_RAM, ip_IMS, 
    puerto_IMS, grado_multitarea, algoritmo, quantum_RR, duracion_sabotaje, ciclo_CPU);

    cantidadVieja = 0;
    cantidadActual = 0;
    contadorPCBs = 0;
    cantidadTCBTotales = 0;
    validador = 1;
    planificacion_viva = 1;
    sabotaje_activado = 0;

    //conexion_RAM = _connect(ip_RAM, puerto_RAM, logger);
    //conexion_IMS = _connect(ip_IMS, puerto_IMS, logger);

    pthread_mutex_init(&mutexNew, NULL);
    pthread_mutex_init(&mutexReady, NULL);
    // pthread_mutex_init(&mutexExec, NULL);
    // pthread_mutex_init(&mutexBloqIO, NULL);
    // pthread_mutex_init(&mutexBloqEmer, NULL);
    // pthread_mutex_init(&mutexExit, NULL);
    pthread_mutex_init(&mutexValidador, NULL);
    // pthread_mutex_init(&mutexListaPCB, NULL);


    sem_init(&semConsola, 0, 1);
    sem_init(&semNR, 0, 0);
    // sem_init(&semRE, 0, 0);
    // sem_init(&semER, 0, 0);
    // sem_init(&semBLOCKIO, 0, 0);
    // sem_init(&semEXIT, 0, 0);
    // sem_init(&semEBIO, 0, 0);
    // sem_init(&semEaX, 0, 0);

    
    pthread_create(&hNewaReady, NULL, (void *) funcionhNewaReady, logger);
    // pthread_create(&hReadyaExec, NULL, (void *) funcionhReadyaExec, logger);
    // pthread_create(&hExecaReady, NULL, (void *) funcionhExecaReady, logger);
    // pthread_create(&hExecaBloqIO, NULL, (void *) funcionhExecaBloqIO, logger);
    // pthread_create(&hExecaExit, NULL, (void *) funcionhExecaExit, logger);
    // pthread_create(&hBloqIO, NULL, (void *) funcionhBloqIO, logger);
    // pthread_create(&hExit, NULL, (void *) funcionhExit, logger);
        // pthread_create(&hExecReadyaBloqEmer, NULL, (void *) funcionhExecReadyaBloqEmer, logger);
        // pthread_create(&hBloqEmeraReady, NULL, (void *) funcionhBloqEmeraReady, logger);

   //pthread_detach(hNewaReady);
    //pthread_detach(hReadyaExec);
    //pthread_detach(hExecaReady);
    //pthread_detach(hExecaBloqIO);
    //pthread_detach(hExecaExit);
    //pthread_detach(hBloqIO);
    //pthread_detach(hExit);
    //pthread_detach(hExecReadyaBloqEmer);
    //pthread_detach(hBloqEmeraReady);

    // char* bufferAEnviar = string_new();
    // string_append(&bufferAEnviar, "Aviso de inicio Discordiador");
    // _send_message(conexion_IMS, "DIS", INICIO_DISCORDIADOR, bufferAEnviar, strlen(bufferAEnviar), logger);
    // free(bufferAEnviar);
}


void funcionConsola(t_log* logger){
    char* leido;
    
    pthread_mutex_lock(&mutexValidador);
    while(validador) {
    pthread_mutex_unlock(&mutexValidador);

        sem_wait(&semConsola);
        while(strcmp(leido = readline("> "), "") != 0) { //mientras se ingrese algo por consola

            parametros = string_split(leido," "); //char**: vector de strings, cada elemento del vector es un parametro, menos el primero que es el mensaje!
            free(leido);

            t_mensaje *mensajeRecibido = malloc (sizeof(t_mensaje));
            int tamanioBuffer;
            void* buffer;
          
            switch (obtener_tipo_mensaje_consola(parametros[0])) {
                case C_INICIAR_PLANIFICACION: 
                    log_info(logger, "Entró comando: INICIAR_PLANIFICACION");
                    planificacion_viva = 1;  //activa flag para que se ejecuten los hilos
                    log_info(logger, "Se inició la planificación. Estado de la flag: %d", planificacion_viva);

                	free(mensajeRecibido);
                	free(parametros[0]); 
                	free(parametros);
                    break;

                case C_PAUSAR_PLANIFICACION: 
                	log_info(logger, "Entró comando: PAUSAR_PLANIFICACION");
                	planificacion_viva = 0;
                	log_info(logger, "Se pausó la planificación.");
                
                	free(mensajeRecibido);
                	free(parametros[0]); 
                	free(parametros);
                    break;

                case C_INICIAR_PATOTA: 
                    log_info(logger, "Entró comando: INICIAR_PATOTA");
                    pcb* nuevoPCB = crear_PCB (parametros, conexion_RAM, logger);
                		
                	if (nuevoPCB) {
                    	list_add (listaPCB, (void*) nuevoPCB);

                        if(cantidadVieja == 0){
                            semTripulantes = malloc(sizeof(sem_t)*cantidadActual); 
                            hiloTripulante = malloc(sizeof(pthread_t) * cantidadActual); 
                        }else{
                            semTripulantes = realloc(semTripulantes,(sizeof(sem_t)) * cantidadActual);
                            hiloTripulante = realloc(hiloTripulante , sizeof(pthread_t) * cantidadActual); 

                        } 

                        for(int i=cantidadVieja; i<cantidadActual; i++){
                            sem_init(&semTripulantes[i], 0, 0);

                            
                        }
                        create_tcb_by_list(nuevoPCB->listaTCB, iniciar_tcb, conexion_RAM, logger);//recorre la lista de TCBs, los agrega a new y crea el hilo de cada tripulante
                        cantidadVieja += cantidadActual;
                    } else {
                    	log_error(logger, "No se pudo crear el PCB por falta de memoria");
                        cantidadActual-=atoi(parametros[1]);
                    }
                		
                	free(parametros[0]); //iniciarPatota
                	int cantidadTripulantes = atoi(parametros[1]);
                	free(parametros[1]);//5 (en formato de char*)
                	free(parametros[2]);//listaTareas
                
                	for(int i = 3 ; parametros[i] != NULL ; i++){
                    	free(parametros[i]);
                    }
                	free(parametros);
                	free(mensajeRecibido);
                    break;

                case C_LISTAR_TRIPULANTES:
                    log_info(logger, "--------------------------------------------------------------------");
                    log_info(logger, "Entró comando: LISTAR_TRIPULANTES");
                    char* hora_y_fecha_actual;
                    hora_y_fecha_actual = temporal_get_string_time("%d/%m/%y %H:%M:%S");
                    log_info(logger, "Estado de la Nave: %s", hora_y_fecha_actual);
                    list_iterate(listaPCB, mostrarListaTripulantes);
                    log_info(logger, "--------------------------------------------------------------------");
                	free(hora_y_fecha_actual);
                	free(mensajeRecibido);
                	free(parametros[0]);
                	free(parametros);
                    break;

                case C_EXPULSAR_TRIPULANTE:
                    // log_info(logger, "Entró comando: EXPULSAR_TRIPULANTE");
                    // if (planificacion_viva == 0) {
                    //     loEncontro = 0;
                    //     tamanioBuffer = sizeof(int);
                    //     buffer = malloc(tamanioBuffer);
					// 	int idTripulante = atoi(parametros[1]);
                    //     buffer = _serialize(tamanioBuffer, "%d", idTripulante);
                    //     _send_message(conexion_RAM, "DIS", EXPULSAR_TRIPULANTE, buffer, tamanioBuffer, logger);
                    //     free(buffer);
                    //   	t_mensaje* mensajeRecibido = _receive_message(conexion_RAM, logger);
                        
                    //   	if(mensajeRecibido->command == SUCCESS) {
                    //     	log_info(logger, "Se expulsó correctamente el tripulante en memoria");
                    //         expulsarNodo(cola_new, "New", mutexNew);
                    //         expulsarNodo(ready, "Ready", mutexReady);
                    //         expulsarNodo(ready, "Exec", mutexExec);
                    //         expulsarNodo(bloq_io, "Bloqueado por IO", mutexBloqIO);
                    //         expulsarNodo(bloq_emer, "Bloqueado por emergencia", mutexBloqEmer);
                    //     }
                    //     else {
                    //       	log_info(logger, "No se pudo expulsar el tripulante en memoria");
                    //     }
                    // }
                    // else{
                    //     log_info(logger, "La planificación está pausada, no se puede expulsar a un tripulante");
                    // }

                    // free(mensajeRecibido->payload);
                	// free(mensajeRecibido->identifier);
                	// free(mensajeRecibido);

                    // free(parametros[0]);
                	// free(parametros[1]);
                	// free(parametros);
                    break;

                case C_OBTENER_BITACORA: 
                    // log_info(logger, "Entró comando: OBTENER_BITACORA");
                    // tamanioBuffer = sizeof(int);
                	// int idTripulante = atoi(parametros[1]);
                    // buffer = _serialize(tamanioBuffer, "%d", idTripulante);
                    // _send_message(conexion_RAM, "DIS", ENVIAR_OBTENER_BITACORA, buffer, tamanioBuffer, logger); //ENVIAR_OBTENER_BITACORA: 760
                    // free(buffer);
                    // t_mensaje* mensajeRecibido = _receive_message(conexion_IMS, logger);
                	// if(mensajeRecibido->command == RESPUESTA_OBTENER_BITACORA){ //RESPUESTA_OBTENER_BITACORA: 766
    				// 	log_info(logger,"La bitacora del tripulante es: %s", mensajeRecibido->payload);
                    //   	//Esperar a delfi para ver como hace el string.
                    // }else{
                    //   	log_error(logger, "No se encontró bitácora para el tripulante: %d", idTripulante);
                    // } 
                	// free(mensajeRecibido->payload);
                	// free(mensajeRecibido->identifier);
                	// free(mensajeRecibido);
                
                	// free(parametros[0]);
                	// free(parametros[1]);
                	// free(parametros);
                    break;
                
                case C_SALIR: 
                    log_info(logger, "Salimos de la consola");
                    pthread_mutex_lock(&mutexValidador);
                    validador = 0;
                    pthread_mutex_unlock(&mutexValidador);

                	free(mensajeRecibido);
                    free(parametros[0]);
                	free(parametros);
                    //liberarMemoria();
                    break;

                default:
                    log_info(logger, "El mensaje ingresado no corresponde a una acción propia del Discordiador");
                	free(mensajeRecibido);
                    free(parametros[0]);
                	free(parametros);
                    break;
                }

                pthread_mutex_lock(&mutexValidador);
                if (!validador) {
                    pthread_mutex_unlock(&mutexValidador);
                    break;
                 }
        }
        
        sem_post(&semNR);
    }

}


int obtener_tipo_mensaje_consola(char *mensaje) {
    if(!strcmp(mensaje, "INICIAR_PLANIFICACION")) {
        return C_INICIAR_PLANIFICACION;
    }else if(!strcmp(mensaje, "PAUSAR_PLANIFICACION")) {
        return C_PAUSAR_PLANIFICACION;
    }else if(!strcmp(mensaje, "INICIAR_PATOTA")) {
        return C_INICIAR_PATOTA;
    }else if(!strcmp(mensaje, "LISTAR_TRIPULANTES")) {
        return C_LISTAR_TRIPULANTES;
    }else if(!strcmp(mensaje, "EXPULSAR_TRIPULANTE")) {
        return C_EXPULSAR_TRIPULANTE;
    }else if(!strcmp(mensaje, "OBTENER_BITACORA")) {
        return C_OBTENER_BITACORA;
    }else if(!strcmp(mensaje, "SALIR")) {
        return C_SALIR;
    }return -1;
}


void mostrarTripulante(void* elemento){
    tcb* tripulante = (tcb*) elemento;
    log_info(logger, "Tripulante: %d\t", tripulante->tid);
    log_info(logger, "Patota: %d\t", tripulante->pid);
    log_info(logger,"Tarea actual: %s\n", tripulante->instruccion_actual);
    log_info(logger,"Posicion X e Y: %d - %d\n", tripulante->posicionX, tripulante->posicionY);

    switch(tripulante->status){
        case 'N':
            log_info(logger, "Status: NEW\n");
            break;
        case 'R':
            log_info(logger, "Status: READY\n");
            break;
        case 'E':
            log_info(logger, "Status: EXEC\n");
            break;
        case 'I':
            log_info(logger, "Status: BLOQ IO\n");
            break;
        case 'M':
            log_info(logger, "Status: BLOQ EMERGENCIA\n");
            break;
        case 'X':
            log_info(logger, "Status: EXIT\n");
            break;
    }
}

void mostrarListaTripulantes(void* elemento){
    pcb* patotas = (pcb*) elemento;
    list_iterate(patotas->listaTCB, mostrarTripulante);
}