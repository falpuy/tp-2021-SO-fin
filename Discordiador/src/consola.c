#include "headers/consola.h"


void funcionConsola(){
    char* leido;
    
    pthread_mutex_lock(&mutexValidador);
    int temp_validador = validador;
    pthread_mutex_unlock(&mutexValidador);
    
    while(temp_validador) {

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
                    
                    sem_post(&semNR); //Le avisa a New->Ready q es su turno
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
                    log_info(logger, "Entró comando: INICIAR_PATOTA" );
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
                    liberarMemoria();
                    break;

                default:
                    log_info(logger, "El mensaje ingresado no corresponde a una acción propia del Discordiador");
                	free(mensajeRecibido);
                    free(parametros[0]);
                	free(parametros);
                    break;
            }

            if (!temp_validador) {
                break;
            }
        }
    }
}



/*-------------------------------------------FUNCIONES ADICIONALES-----------------------------*/

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

bool buscarTripulante (void* elemento){
    tcb* tripulante = (tcb*) elemento;
    return tripulante->tid == atoi(parametros[1]);
}

void iterar_en_lista(t_list* self, void(*closure)(void*, t_list*, pthread_mutex_t, int), pthread_mutex_t mutexCola) {
    int posicion = 0;
	t_link_element *element = self->head;
	t_link_element *aux = NULL;
	while (element != NULL) {
		aux = element->next;
		closure(element->data, self, mutexCola, posicion);
		element = aux;
        posicion++;
	}
}

void extraerTripulante (void* nodo, t_list* cola, pthread_mutex_t mutexCola, int posicion){
    tcb* tripulanteAExpulsar = (tcb*) nodo;
    if (tripulanteAExpulsar->status=='X'){
        tcb* aux_TCB = malloc (sizeof(tcb));
        pthread_mutex_lock(&mutexCola);
        aux_TCB = list_remove(cola, posicion);
        pthread_mutex_unlock(&mutexCola);
        pthread_mutex_lock(&mutexExit);
        queue_push(cola_exit, (void*) aux_TCB);
        pthread_mutex_unlock(&mutexExit);
    }
}

void expulsarNodo (t_queue* cola, char* nombre_cola, pthread_mutex_t mutexCola){
    if (loEncontro==0){
        tcb* tripulanteAExpulsar = malloc(sizeof(tcb));
        tripulanteAExpulsar = list_find (cola->elements, buscarTripulante);
      	if (tripulanteAExpulsar != NULL) {
            log_info(logger, "Se encontró tripulante: %d a expulsar en la cola: %s", tripulanteAExpulsar->tid, nombre_cola);
          	tripulanteAExpulsar->status = 'X';
            iterar_en_lista(cola->elements, extraerTripulante, mutexCola);
			log_info(logger, "Se movió tripulante: %d de la cola: %s a la cola: EXIT", tripulanteAExpulsar->tid, nombre_cola);
            loEncontro=1;
        }
        else{
            log_info(logger, "No se encontro el tripulante en la cola %s", nombre_cola);
          	free(tripulanteAExpulsar);
        }
    }
}




void liberarMemoria(){
    pthread_join(hNewaReady,NULL);
    pthread_join(hReadyaExec,NULL);
    pthread_join(hExecaReady,NULL);
    pthread_join(hExecaExit,NULL);
    pthread_join(hExecaBloqIO,NULL);
    pthread_join(hBloqIO,NULL);
    pthread_join(hExit,NULL);

     for(int i = 0; i < cantidadTCBTotales; i++){
        pthread_join(hiloTripulante[i],NULL);
    }

  	queue_destroy_and_destroy_elements(cola_new, destruirTCB);
  	queue_destroy_and_destroy_elements(ready, destruirTCB);
  	queue_destroy_and_destroy_elements(exec, destruirTCB);
  	queue_destroy_and_destroy_elements(bloq_io, destruirTCB);
  	queue_destroy_and_destroy_elements(bloq_emer, destruirTCB);
  	queue_destroy_and_destroy_elements(cola_exit, destruirTCB);
		
  	list_destroy_and_destroy_elements(listaPCB, destruirPCB);

		
  	pthread_mutex_destroy(&mutexNew);
  	pthread_mutex_destroy(&mutexReady);
  	pthread_mutex_destroy(&mutexExec);
  	pthread_mutex_destroy(&mutexBloqIO);
  	pthread_mutex_destroy(&mutexBloqEmer);
  	pthread_mutex_destroy(&mutexExit);
    pthread_mutex_destroy(&mutexListaPCB);
    
    sem_destroy(&semNR);
    sem_destroy(&semRE);
    sem_destroy(&semER);
    sem_destroy(&semBLOCKIO);
    sem_destroy(&semEXIT);
    sem_destroy(&semEBIO);
    sem_destroy(&semEaX);

    for(int i = 0; i < cantidadTCBTotales; i++){
        sem_destroy(&semTripulantes[i]);
        //pthread_join(hiloTripulante[i],NULL);
    }

    log_destroy(logger);
  	config_destroy(config);

    exit(1);
  	
}