#include "headers/consola.h"

// crear hilo

void mostrarTripulante(void* elemento){
    tcb* tripulante = (tcb*) elemento;
    log_info(logger, "Tripulante: %d\t", tripulante->tid);
    log_info(logger, "Patota: %d\t", tripulante->pid);
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

bool buscarTripulante (void* elemento){
    tcb* tripulante = (tcb*) elemento;
    return tripulante->tid == atoi(parametros[1]);
}

void expulsarNodo (t_queue* cola, char* nombre_cola){
    if (loEncontro==0) {
        tcb* tripulanteAExpulsar = malloc(sizeof(tcb));
        tripulanteAExpulsar = list_find (cola->elements, buscarTripulante);
    	tcb* aux_TCB = malloc (sizeof(tcb));	
      	if (tripulanteAExpulsar != NULL) {
            log_info(logger, "Se encontró tripulante: %d a expulsar en la cola: %s", tripulanteAExpulsar->tid, nombre_cola);
            loEncontro=1;
          	switch(tripulanteAExpulsar->status){
                case 'N':
                    pthread_mutex_lock(&mutexNew);
                    aux_TCB = queue_pop(cola_new);
                    pthread_mutex_unlock(&mutexNew);
                    aux_TCB->status = 'X';
                    pthread_mutex_lock(&mutexExit);
                    queue_push(cola_exit, (void*) aux_TCB);
                    pthread_mutex_unlock(&mutexExit);
                    break;

                case 'R':
					pthread_mutex_lock(&mutexReady);
                    aux_TCB = queue_pop(ready);
                    pthread_mutex_unlock(&mutexReady);
                    aux_TCB->status = 'X';
                    pthread_mutex_lock(&mutexExit);
                    queue_push(cola_exit, (void*) aux_TCB);
                    pthread_mutex_unlock(&mutexExit);
                    break;

                case 'E':
					pthread_mutex_lock(&mutexExec);
                    aux_TCB = queue_pop(exec);
                    pthread_mutex_unlock(&mutexExec);
                    aux_TCB->status = 'X';
                    pthread_mutex_lock(&mutexExit);
                    queue_push(cola_exit, (void*) aux_TCB);
                    pthread_mutex_unlock(&mutexExit);
                    break;

                case 'I':
					pthread_mutex_lock(&mutexBloqIO);
                    aux_TCB = queue_pop(bloq_io);
                    pthread_mutex_unlock(&mutexBloqIO);
                    aux_TCB->status = 'X';
                    pthread_mutex_lock(&mutexExit);
                    queue_push(cola_exit, (void*) aux_TCB);
                    pthread_mutex_unlock(&mutexExit);
                    break;

                case 'M':
                    pthread_mutex_lock(&mutexBloqEmer);
                    aux_TCB = queue_pop(bloq_emer);
                    pthread_mutex_unlock(&mutexBloqEmer);
                    aux_TCB->status = 'X';
                    pthread_mutex_lock(&mutexExit);
                    queue_push(cola_exit, (void*) aux_TCB);
                    pthread_mutex_unlock(&mutexExit);
                    break;

                case 'X':
                    log_info(logger,"Ya fue expulsado el tripulante");
                    break;
            }
				log_info(logger, "Se movió tripulante:%d de la cola:%s a la cola: EXIT", tripulanteAExpulsar->tid, nombre_cola);	         
        }
        else{
            log_info(logger, "No se encontro el tripulante en la cola %s", nombre_cola);
            free(aux_TCB);
          	free(tripulanteAExpulsar);
        }
    }
}

void create_tcb_by_list(t_list* self, void(*closure)(void*, int, t_log*), int conexion_RAM, t_log *logger) {
	t_link_element *element = self->head;
	t_link_element *aux = NULL;
	while (element != NULL) {
		aux = element->next;
		closure(element->data, conexion_RAM, logger);
		element = aux;
	}
}

void iniciar_tcb(void *elemento, int conexion_RAM, t_log *logger) {
	tcb *aux = (tcb *) elemento;
  	int tamanioBuffer = sizeof(int) * 2;
  	void *buffer = _serialize(tamanioBuffer, "%d%d", aux->pid, aux->tid);
  	_send_message(conexion_RAM, "DIS", 520, buffer, tamanioBuffer, logger);
    free(buffer);
  	t_mensaje *mensaje = _receive_message(conexion_RAM, logger);
  	if (mensaje->command == 200) { // Recibi la primer tarea
        log_info(logger, "entra al if");
        int tamanioTarea;
        memcpy(&tamanioTarea, mensaje->payload, sizeof(int));
        log_info(logger, "tamanio tarea: %d", tamanioTarea);
        aux->instruccion_actual = malloc(tamanioTarea + 1);
        memcpy(aux->instruccion_actual, mensaje->payload + sizeof(int), tamanioTarea);
        aux->instruccion_actual[tamanioTarea] = '\0';
        log_info(logger, "tarea actual: %s", aux->instruccion_actual);
        aux->estaVivoElHilo = 1;
        queue_push (cola_new, (void*) aux);
        pthread_t hiloTripulante;
        pthread_create(&hiloTripulante, NULL, (void *) funcionTripulante, aux);
        pthread_detach(hiloTripulante);
    } else {
    	log_error(logger, "No hay tareas disponibles");
    }
}

enum tipo_mensaje_consola {
INICIAR_PLANIFICACION,
PAUSAR_PLANIFICACION,
INICIAR_PATOTA,
LISTAR_TRIPULANTES,
EXPULSAR_TRIPULANTE,
OBTENER_BITACORA,
SALIR};

int obtener_tipo_mensaje_consola(char *mensaje) {
    if(!strcmp(mensaje, "INICIAR_PLANIFICACION")) {
            return INICIAR_PLANIFICACION;
    };
    if(!strcmp(mensaje, "PAUSAR_PLANIFICACION")) {
            return PAUSAR_PLANIFICACION;
    };
    if(!strcmp(mensaje, "INICIAR_PATOTA")) {
            return INICIAR_PATOTA;
    };
    if(!strcmp(mensaje, "LISTAR_TRIPULANTES")) {
            return LISTAR_TRIPULANTES;
    };
    if(!strcmp(mensaje, "EXPULSAR_TRIPULANTE")) {
            return EXPULSAR_TRIPULANTE;
    };
    if(!strcmp(mensaje, "OBTENER_BITACORA")) {
            return OBTENER_BITACORA;
    };
    if(!strcmp(mensaje, "SALIR")) {
            return SALIR;
    };
    return -1;
}

void destruirTCB(void* nodo){
    tcb* tcbADestruir = (tcb*) nodo;
    free(tcbADestruir->instruccion_actual);
    free(tcbADestruir);
}

void destruirPCB(void* nodo){
    pcb* pcbADestruir = (pcb*) nodo;
    free(pcbADestruir->rutaTareas);
    list_destroy_and_destroy_elements(pcbADestruir->listaTCB, destruirTCB);
    free(pcbADestruir);
}

void liberarMemoria(){
  	log_destroy(logger);
  	config_destroy(config);
  	
  	queue_destroy_and_destroy_elements(cola_new, destruirTCB);
  	queue_destroy_and_destroy_elements(ready, destruirTCB);
  	queue_destroy_and_destroy_elements(exec, destruirTCB);
  	queue_destroy_and_destroy_elements(bloq_io, destruirTCB);
  	queue_destroy_and_destroy_elements(bloq_emer, destruirTCB);
  	queue_destroy_and_destroy_elements(cola_exit, destruirTCB);
		
  	list_destroy_and_destroy_elements(listaPCB, destruirPCB);
    free(ip_RAM);
    free(puerto_RAM);
    free(ip_IMS);
    free(puerto_IMS);
    free(algoritmo);
		
  	pthread_mutex_destroy(&mutexNew);
  	pthread_mutex_destroy(&mutexReady);
  	pthread_mutex_destroy(&mutexExec);
  	pthread_mutex_destroy(&mutexBloqIO);
  	pthread_mutex_destroy(&mutexBloqEmer);
  	pthread_mutex_destroy(&mutexExit);
}

void funcionConsola(t_log* logger, int conexion_RAM, int conexion_IMS) {
    contadorPCBs = 0;
    validador = 1;
    planificacion_pausada = 1;
    char* leido;

    while(validador) {
        while(strcmp(leido = readline("> "), "") != 0) { //mientras se ingrese algo por consola
            parametros = string_split(leido," "); //char**: vector de strings, cada elemento del vector es un parametro, menos el primero que es el mensaje!
            free(leido);

            t_mensaje *mensajeRecibido = malloc (sizeof(t_mensaje));
            int tamanioBuffer;
            void* buffer;
          
            switch (obtener_tipo_mensaje_consola(parametros[0])) {
                case INICIAR_PLANIFICACION: 
                    log_info(logger, "Entró comando: INICIAR_PLANIFICACION");
                    planificacion_pausada = 0;  //activa flag para que se ejecuten los hilos
                    log_info(logger, "Se inició la planificación. Estado de la flag: %d", planificacion_pausada);

                	free(mensajeRecibido);
                	free(parametros[0]); 
                	free(parametros);
                    break;

                case PAUSAR_PLANIFICACION: 
                	log_info(logger, "Entró comando: PAUSAR_PLANIFICACION");
                	planificacion_pausada = 1;
                	log_info(logger, "Se pausó la planificación.");
                
                	free(mensajeRecibido);
                	free(parametros[0]); 
                	free(parametros);
                    break;

                case INICIAR_PATOTA: 
                    log_info(logger, "Entró comando: INICIAR_PATOTA");
                    pcb* nuevoPCB = crear_PCB (parametros, conexion_RAM, logger);
                		
                	if (nuevoPCB) {
                    	list_add (listaPCB, (void*) nuevoPCB);
                        create_tcb_by_list(nuevoPCB->listaTCB, iniciar_tcb, conexion_RAM, logger);//recorre la lista de TCBs, los agrega a new y crea el hilo de cada tripulante
                    } else {
                    	log_error(logger, "No se pudo crear el PCB por falta de memoria");
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

                case LISTAR_TRIPULANTES:
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

                case EXPULSAR_TRIPULANTE:
                    log_info(logger, "Entró comando: EXPULSAR_TRIPULANTE");
                    if (planificacion_pausada == 0) {
                        loEncontro = 0;
                        tamanioBuffer = sizeof(int);
                        buffer = malloc(tamanioBuffer);
						int idTripulante = atoi(parametros[1]);
                        buffer = _serialize(tamanioBuffer, "%d", idTripulante);
                        _send_message(conexion_RAM, "DIS", 530, buffer, tamanioBuffer, logger); // EXPULSAR_TRIPULANTE= 530 
                        free(buffer);
                      	t_mensaje* mensajeRecibido = _receive_message(conexion_RAM, logger);
                        
                      	if(mensajeRecibido->command == 200) { // SUCCESS= 200
                        		log_info(logger, "Se expulsó correctamente el tripulante en memoria");
                        }
                        else {
                          	log_info(logger, "No se pudo expulsar el tripulante en memoria");
                        }

                        expulsarNodo(cola_new, "New");
                        expulsarNodo(ready, "Ready");
                        expulsarNodo(exec, "Exec");
                        expulsarNodo(bloq_io, "Bloqueado por IO");
                        expulsarNodo(bloq_emer, "Bloqueado por emergencia");
                      
                        if (loEncontro==0){
                            log_info(logger, "El tripulante que se quiere expulsar ya esta en estado EXIT o el ID ingresado es incorrecto");
                        }
                    }
                    else{log_info(logger, "La planificación está pausada, no se puede expulsar a un tripulante");}

                    free(mensajeRecibido->payload);
                	free(mensajeRecibido->identifier);
                	free(mensajeRecibido);

                    free(parametros[0]);
                	free(parametros[1]);
                	free(parametros);
                    
                    break;

                case OBTENER_BITACORA: 
                    log_info(logger, "Entró comando: OBTENER_BITACORA");
                    tamanioBuffer = sizeof(int);
                	int idTripulante = atoi(parametros[1]);
                    buffer = _serialize(tamanioBuffer, "%d", idTripulante);
                    _send_message(conexion_RAM, "DIS", 560, buffer, tamanioBuffer, logger); //ENVIAR_OBTENER_BITACORA: 560
                    free(buffer);
                    t_mensaje* mensajeRecibido = _receive_message(conexion_IMS, logger);
                	if(mensajeRecibido->command == 766){ //RESPUESTA_OBTENER_BITACORA: 766
    					log_info(logger,"La bitacora del tripulante es: %s", mensajeRecibido->payload);
                      	//Esperar a delfi para ver como hace el string.
                    }else{
                      	log_error(logger, "No se encontró bitácora para el tripulante: %d", idTripulante);
                    } 
                	free(mensajeRecibido->payload);
                	free(mensajeRecibido->identifier);
                	free(mensajeRecibido);
                
                	free(parametros[0]);
                	free(parametros[1]);
                	free(parametros);
                    break;
                
                case SALIR: 
                    log_info(logger, "Salimos de la consola");
                    validador = 0;
                	free(mensajeRecibido);
                    free(parametros[0]);
                	free(parametros);
                    liberarMemoria();
                    break;

                default:
                    log_info(logger, "El mensaje ingresado no corresponde a una acción propia del Discordiador");
                	free(mensajeRecibido);
                    free(parametros[0]);
                	free(parametros);
                    break;
            }
            
            if (!validador) {
                break;
            }
        }
    }
}