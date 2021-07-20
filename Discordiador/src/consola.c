#include "headers/consola.h"

void funcionConsola(){
    
    while(validador) {
        char* leido;
        
        while(strcmp(leido = readline("> "), "") != 0) { 
            parametros = string_split(leido," "); //char**: vector de strings, cada elemento del vector es un parametro, menos el primero que es el mensaje!
            free(leido);

            int tipoMensaje = obtener_tipo_mensaje_consola(parametros[0]);
            switch (tipoMensaje) {
                case C_INICIAR_PLANIFICACION:

                    log_info(logger, "Entró comando: INICIAR_PLANIFICACION");
                    planificacion_viva = 1;  //activa flag para que se ejecuten los hilos
                    log_info(logger, "Se inició la planificación. Estado de la flag: %d", planificacion_viva);

                	free(parametros[0]); 
                	free(parametros);
                    
                    sem_post(&semNR); //Le avisa a New->Ready q es su turno
                    break;

                case C_PAUSAR_PLANIFICACION: 
                	log_info(logger, "Entró comando: PAUSAR_PLANIFICACION");
                	planificacion_viva = 0;
                	log_info(logger, "Se pausó la planificación.");

                	free(parametros[0]); 
                	free(parametros);
                    break;

                case C_INICIAR_PATOTA: 
                    log_info(logger, "Entró comando: INICIAR_PATOTA" );
                    pcb* nuevoPCB = crear_PCB (parametros, conexion_RAM, logger);

                	if (nuevoPCB) {
                        list_add(listaPCB, nuevoPCB);
                        if(cantidadVieja == 0){

                            semTripulantes = malloc(sizeof(sem_t)*cantidadActual); 
                            hiloTripulante = malloc(sizeof(pthread_t) * cantidadActual); 
                        
                        }else{
                            semTripulantes = realloc(semTripulantes,(sizeof(sem_t)) * cantidadActual);
                            hiloTripulante = realloc(hiloTripulante , sizeof(pthread_t) * cantidadActual);

                        } 

                        for(int i = cantidadVieja; i < cantidadActual; i++){
                            sem_init(&semTripulantes[i], 0, 0);

                            log_info(logger, "Inicializo Semaforo de TCB %d", i);
                            
                        }
                        create_tcb_by_list(nuevoPCB->listaTCB, iniciar_tcb, conexion_RAM, cantidadVieja, logger);//recorre la lista de TCBs, los agrega a new y crea el hilo de cada tripulante
                        
                        pthread_mutex_lock(&mutex_cantidadVieja);
                        cantidadVieja += cantidadActual;
                        pthread_mutex_unlock(&mutex_cantidadVieja);

                    } else {
                    	log_error(logger, "No se pudo crear el PCB");
                        cantidadActual-=atoi(parametros[1]);
                    }

                	free(parametros[0]); //iniciarPatota
                	free(parametros[1]);//5 (en formato de char*)
                	free(parametros[2]);//listaTareas
                
                	for(int i = 3 ; parametros[i] != NULL ; i++){
                    	free(parametros[i]);
                    }
                	free(parametros);
                    break;

                case C_LISTAR_TRIPULANTES:
                    log_info(logger, "--------------------------------------------------------------------");
                    log_info(logger, "Entró comando: LISTAR_TRIPULANTES");
                    int hayTripulantesEnNave = hayTripulantesNave();
                    if(hayTripulantesEnNave){
                        char* hora_y_fecha_actual;
                        hora_y_fecha_actual = temporal_get_string_time("%d/%m/%y %H:%M:%S");
                        
                        log_info(logger, "Estado de la Nave: %s", hora_y_fecha_actual);

                        pthread_mutex_lock(&mutexListaPCB);
                        list_iterate(listaPCB, mostrarListaTripulantes);
                        pthread_mutex_unlock(&mutexListaPCB);
                        free(hora_y_fecha_actual);
                    }
                    else{
                        log_info(logger, "No hay tripulantes en la nave, no podemos listar");
                    }
                    log_info(logger, "--------------------------------------------------------------------");
                	
                	free(parametros[0]);
                	free(parametros);

                    break;

                case C_EXPULSAR_TRIPULANTE:
                    log_info(logger, "Entró comando: EXPULSAR_TRIPULANTE");                    
                    if (planificacion_viva) {
					  	int idTripulante = atoi(parametros[1]);
                       
                        pthread_mutex_lock(&mutexListaPCB);
                        tcb* tcbTripulante = obtener_tcb_en_listaPCB(listaPCB);
                        pthread_mutex_unlock(&mutexListaPCB);
                        
                        pthread_mutex_lock(&mutexBuffer);
                        buffer = _serialize(2*sizeof(int), "%d%d", tcbTripulante->pid, idTripulante);
                        _send_message(conexion_RAM, "DIS", EXPULSAR_TRIPULANTE, buffer, 2*sizeof(int), logger);
                        free(buffer);
                      	t_mensaje* mensajeRecibido = _receive_message(conexion_RAM, logger);
                        pthread_mutex_unlock(&mutexBuffer);
                        
                       	if(mensajeRecibido->command == SUCCESS) {
                            log_info(logger, "Se expulsó correctamente el tripulante en memoria");
                            
                            switch(tcbTripulante->status){
                                case 'N':
                                    log_info(logger, "Se expulsa al tripulante de la cola NEW");
                                    expulsarNodo(cola_new, "New", mutexNew);
                                    break;
                                case 'R':
                                    log_info(logger, "Se expulsa al tripulante de la cola READY");
                                    expulsarNodo(ready, "Ready", mutexReady);
                                    break;
                                case 'E':
                                    log_info(logger, "Se expulsa al tripulante de la cola EXEC");
                                    expulsarNodo(exec, "Exec", mutexExec);
                                    break;
                                case 'I':
                                    log_info(logger, "Se expulsa al tripulante de la cola BLOQ IO");
                                    expulsarNodo(bloq_io, "Bloqueado por IO", mutexBloqIO);
                                    break;
                                case 'M':
                                    log_info(logger, "Se expulsa al tripulante de la cola BLOQ EMERGENCIA");
                                    expulsarNodo(bloq_emer, "Bloqueado por emergencia", mutexBloqEmer);
                                    break;
                                case 'X':
                                    log_info(logger, "El tripulante ya se encuentra en la cola EXIT");
                                    break;
                            }
                        }else{
                            log_info(logger, "No se pudo expulsar el tripulante en memoria");
                        }

                        free(mensajeRecibido->identifier);
                        free(mensajeRecibido->payload);
                        free(mensajeRecibido);
                    }else{
                        log_error(logger, "La planificación está pausada, no se puede expulsar a un tripulante");
                    }
                    
                    free(parametros[0]);
                	free(parametros[1]);
                	free(parametros);
                    break;

                case C_OBTENER_BITACORA: 
                    log_info(logger, "Entró comando: OBTENER_BITACORA");
                	int idTripulante = atoi(parametros[1]);
                    
                    pthread_mutex_lock(&mutexBuffer);
                    buffer = _serialize(sizeof(int), "%d", idTripulante);
                    _send_message(conexion_IMS, "DIS", ENVIAR_OBTENER_BITACORA, buffer, sizeof(int), logger); //ENVIAR_OBTENER_BITACORA: 760
                    free(buffer);
                    
                    t_mensaje* mensajeRecibido = _receive_message(conexion_IMS, logger);
                    pthread_mutex_unlock(&mutexBuffer);
                   
                    if(mensajeRecibido->command == RESPUESTA_OBTENER_BITACORA){ //RESPUESTA_OBTENER_BITACORA: 766
                        log_info(logger,"Bitácora del tripulante: %d", idTripulante);
                           
                        int tamanioString;
                        memcpy(&tamanioString,mensajeRecibido->payload, sizeof(int));
                        
                        char* str = malloc(tamanioString + 1);
                        memcpy(str, mensajeRecibido->payload + sizeof(int) , tamanioString);
                        str[tamanioString] = '\0';
                        
                        char** bitacora = string_split(str, "|");
                        for(int i=0; bitacora[i]!=NULL; i++){
                            log_info(logger, "%s", bitacora[i]);
                        }
                        for(int i=0; bitacora[i]!=NULL; i++){
                            free(bitacora[i]);
                        }  
                        free(str);
                        free(bitacora);
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
                
                case C_SALIR: 
                    log_info(logger, "Salimos de la consola");
                
                    free(parametros[0]);
                	free(parametros);
                    
                    liberarMemoria();
                    
                    break;

                default:
                    log_info(logger, "El mensaje ingresado no corresponde a una acción propia del Discordiador");
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



/*-------------------------------------------FUNCIONES ADICIONALES-----------------------------*/

int hayTripulantesNave(){
    if(queue_size(cola_new) || queue_size(ready) || queue_size(exec) || queue_size(bloq_io) || queue_size(bloq_emer) || queue_size(cola_exit)){
        return 1;
    }
    return 0;
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
    log_info(logger, "Tripulante: %d\t Patota:%d\t Tarea:%s\t", tripulante->tid,tripulante->pid,tripulante->instruccion_actual);
    log_info(logger,"Posicion Actual X e Y: %d - %d\n", tripulante->posicionX, tripulante->posicionY);

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
    int idBuscado = atoi(parametros[1]);
    return tripulante->tid == idBuscado;
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
    tcb* tripulanteAExpulsar = malloc(sizeof(tcb));
    tripulanteAExpulsar = list_find(cola->elements, buscarTripulante);

    if (tripulanteAExpulsar!=NULL) {
        tripulanteAExpulsar->status = 'X';
        iterar_en_lista(cola->elements, extraerTripulante, mutexCola);
        log_info(logger, "El tripulante ya fue expulsado de la cola %s", nombre_cola);
    }
    else{
        log_info(logger, "No se encontro el tripulante en la cola %s", nombre_cola);
    }
}

tcb* obtener_tcb_en_listaPCB(t_list* self) {
	t_link_element *element = self->head;
	t_link_element *aux = NULL;
	while (element != NULL) {
		aux = element->next;
        pcb* patota = element->data;
        tcb* tripulante = list_find (patota->listaTCB, buscarTripulante);
        if(tripulante){
            return tripulante;
        }
		element = aux;
	}
    return NULL;
}

void liberarMemoria(){

    validador = 0;

    queue_destroy_and_destroy_elements(cola_new, destruirTCB);
    queue_destroy_and_destroy_elements(ready, destruirTCB);
    queue_destroy_and_destroy_elements(exec, destruirTCB);        
    queue_destroy_and_destroy_elements(bloq_io, destruirTCB);
    queue_destroy_and_destroy_elements(bloq_emer, destruirTCB);
    queue_destroy_and_destroy_elements(cola_exit, destruirTCB);
    queue_destroy_and_destroy_elements(bloq_emer_sorted, destruirTCB);
    list_destroy_and_destroy_elements(listaPCB, destruirPCB);

	
  	pthread_mutex_destroy(&mutexNew);
  	pthread_mutex_destroy(&mutexReady);
  	pthread_mutex_destroy(&mutexExec);
  	pthread_mutex_destroy(&mutexBloqIO);
  	pthread_mutex_destroy(&mutexBloqEmer);
  	pthread_mutex_destroy(&mutexExit);
    pthread_mutex_destroy(&mutexListaPCB);
    pthread_mutex_destroy(&mutex_cantidadVieja);
    pthread_mutex_destroy(&mutex_cantidadActual);
    pthread_mutex_destroy(&mutexBloqEmerSorted);
    pthread_mutex_destroy(&mutexValidador);
    pthread_mutex_destroy(&mutexPlanificacionViva);
    pthread_mutex_destroy(&mutexSabotajeActivado);
    pthread_mutex_destroy(&mutexCiclosTranscurridosSabotaje);
    pthread_mutex_destroy(&mutex_cantidadTCB);
    
    sem_destroy(&semNR);
    sem_destroy(&semRE);
    sem_destroy(&semER);
    sem_destroy(&semBLOCKIO);
    sem_destroy(&semEXIT);
    sem_destroy(&semEBIO);
    sem_destroy(&semEaX);
    sem_destroy(&semERM);
    sem_destroy(&semMR);

    log_destroy(logger);
  	config_destroy(config);

    exit(1);
  	
}