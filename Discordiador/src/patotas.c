#include "./headers/patotas.h"

tcb* crear_TCB(int idP, int posX, int posY, int idT, t_log* logger){
    tcb* nuevoTCB = malloc (sizeof(tcb));
	nuevoTCB->tid = idT;
    nuevoTCB->pid = idP;
    nuevoTCB->status = 'N';
    nuevoTCB->posicionX = posX;
    nuevoTCB->posicionY = posY;
    nuevoTCB->instruccion_actual = NULL;
    nuevoTCB->tiempoEnExec = 0;
    nuevoTCB->tiempoEnBloqIO = 0;
    nuevoTCB->ciclosCumplidos = 0;

    return nuevoTCB;
}

pcb* crear_PCB(char** parametros, int conexion_RAM, t_log* logger){     
  	int offset = 0;
    int cant_tripulantes = atoi(parametros[1]);
    contadorPCBs++;
  
    pcb* nuevoPCB = malloc(sizeof(pcb));
    nuevoPCB->pid = contadorPCBs;

    nuevoPCB->listaTCB = list_create();
    
  	nuevoPCB->rutaTareas = malloc (strlen(parametros[2]) + 1);
    strcpy(nuevoPCB->rutaTareas, parametros[2]);
    nuevoPCB->rutaTareas[strlen(parametros[2])]='\0';
  	// cargo lista de tareas
  	char *tareas = get_tareas(nuevoPCB->rutaTareas, logger);
    log_info(logger,"Tareas: %s", tareas);
		
  	// buffer->[idpcb, largo_tareas, lista_tareas, cant_tripulantes, n_tcbs....]
  	int tamanioBuffer = sizeof(int) * 3 + strlen(tareas) + cant_tripulantes * (sizeof(int)*4 + sizeof(char));

  	void *buffer_a_enviar = malloc(tamanioBuffer);	
  
  	// copio id pcb
  	memcpy(buffer_a_enviar, &nuevoPCB->pid, sizeof(int));
  	offset += sizeof(int);

  	// copio la lista de tareas serializadas
  	void *temp = _serialize(sizeof(int) + strlen(tareas), "%s", tareas);
  
  	memcpy(buffer_a_enviar + offset, temp, sizeof(int) + strlen(tareas));
  	offset += sizeof(int) + strlen(tareas);

  	
    free(tareas);
  	free(temp);
    log_info(logger,"HOLIS");
    
  
  	// Copio la cantidad de tcbs
  	memcpy(buffer_a_enviar + offset, &cant_tripulantes, sizeof(int));
  	offset += sizeof(int);
    
  	int posX;
    int posY;
    bool hayParametros = true;
    for(int i = 1; i<=cant_tripulantes; i++){
        if (hayParametros) {
            if (parametros[2+i] == NULL) {
                hayParametros = false;
                posX = 0;
                posY = 0;
            }else {
                char** posicion = string_split(parametros[2+i], "|");
                posX = atoi(posicion[0]);
                posY = atoi(posicion[1]);

                for(int j = 0; posicion[j] != NULL; j++){
                    free(posicion[j]);
                }
                free(posicion);
            }
        }
      
        int tid = cantidadTCBTotales;
        cantidadTCBTotales++;
        log_info(logger,"HOLIS");
      	
        tcb* nuevoTCB = crear_TCB(contadorPCBs, posX, posY, tid, logger);
        list_add (nuevoPCB->listaTCB, (void*) nuevoTCB);
        
      	memcpy(buffer_a_enviar + offset, &nuevoTCB->tid, sizeof(int));
        offset += sizeof(int);
        memcpy(buffer_a_enviar + offset, &nuevoTCB->pid, sizeof(int));
        offset += sizeof(int);
        memcpy(buffer_a_enviar + offset, &nuevoTCB->status, sizeof(char));
        offset += sizeof(char);
        memcpy(buffer_a_enviar + offset, &nuevoTCB->posicionX, sizeof(int));
        offset += sizeof(int);
        memcpy(buffer_a_enviar + offset, &nuevoTCB->posicionY, sizeof(int));
        offset += sizeof(int);  
    log_info(logger,"HOLIS");


    }
    
    cantidadActual += cant_tripulantes;

    _send_message(conexion_RAM, "DIS", INICIAR_PATOTA, buffer_a_enviar, tamanioBuffer, logger);
    log_info(logger,"HOLIS");

    
  	t_mensaje *mensaje = _receive_message(conexion_RAM, logger);

  	if (mensaje->command == SUCCESS) {
        free(buffer_a_enviar);
        free(mensaje->payload);
        free(mensaje->identifier);
        free(mensaje);

        log_info(logger,"Memoria OK");
        return nuevoPCB;
    }


    free(buffer_a_enviar);
    free(mensaje->payload);
    free(mensaje->identifier);
    free(mensaje);
  
  	return NULL;
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


/*---------------------------FUNCION TRIPULANTE (EXEC)----------------------*/
void funcionTripulante (void* item){
    parametrosThread* aux = (parametrosThread*) item;
    int tamanioBuffer;
    int tamanioTarea;
    void* buffer;
    char** parametrosTarea;
    char** tarea;
    int mensajeInicialIMS=0;
    int cantidadTripulantesEnExec;
    bool llegoALaPosicion = false;
    bool esTareaNormal = false;
    int puedeEnviarSignal = 1;
    int auxX;
    int auxY;
    tcb *tcbTripulante;

    log_info(aux->logger,"----------------------------------");
    log_info(aux->logger,"[Tripulante %d] Esperando Signal...", aux->idSemaforo);

    pthread_mutex_lock(&mutexValidador);
    int temp_validador = validador;
    pthread_mutex_unlock(&mutexValidador);

    while(temp_validador){
        sem_wait(&semTripulantes[aux->idSemaforo]);
        log_info(aux->logger,"[Tripulante %d] EJECUTANDO...", aux->idSemaforo);
        pthread_mutex_lock(&mutexExec);
        tcbTripulante = get_by_id(exec->elements, aux->idSemaforo);
        pthread_mutex_unlock(&mutexExec);

        log_info(aux->logger,"[Tripulante %d] DATOS TRIPULANTE: %d - %s", aux->idSemaforo, tcbTripulante -> tid, tcbTripulante -> instruccion_actual);
        
        if(tcbTripulante->estaVivoElHilo){
            
            if(tcbTripulante->status == 'E' && planificacion_viva){
                log_info(aux->logger, "[Tripulante %d] esta en ejecucion", aux->idSemaforo);
                
                //Spliteo la tarea
                tarea = string_split(tcbTripulante->instruccion_actual, " ");
                log_info(aux->logger, "el nombre de la tarea es %s", tarea[0]);
                if (tarea[1]!=NULL){
                    log_info(aux->logger, "Es tarea IO");
                    parametrosTarea = string_split(tarea[1], ";");
                }else{
                    log_info(aux->logger, "Es tarea normal");
                    parametrosTarea = string_split(tarea[0], ";");
                    strcpy(parametrosTarea[0],"-1");
                }

                int param = atoi(parametrosTarea[0]);
                log_info(aux->logger, "el parametro de la tarea es: %d", param);
                int posicionX = atoi(parametrosTarea[1]);
                log_info(aux->logger, "la posicion en x de la tarea es: %d", posicionX);
                int posicionY = atoi(parametrosTarea[2]);
                log_info(aux->logger, "la posicion en y de la tarea es: %d", posicionY);
                int tiempoTarea = atoi(parametrosTarea[3]);
                log_info(aux->logger, "el tiempo de la tarea es: %d", tiempoTarea);

                llegoALaPosicion = llegoAPosicion(tcbTripulante->posicionX,tcbTripulante->posicionY,posicionX,posicionY);
                
                if(llegoALaPosicion){
                    log_info(aux->logger, "[Tripulante %d] El tripulante %d llegó a la posición, la tarea es %s, es tarea IO? %d", aux -> idSemaforo, tcbTripulante->tid, tarea[0], esTareaIO(tarea[0]));

                    if(mensajeInicialIMS == 0){//Manda mensaje a IMS de "Comienza Ejecucion Tarea"
                        log_info(aux->logger, "antes de mandar mensaje a IMS");
                        tamanioTarea = strlen(tarea[0]);
                        tamanioBuffer = sizeof(int)*6 + tamanioTarea;
                        buffer = _serialize(tamanioBuffer, "%d%s%d%d%d%d", tcbTripulante->tid, tarea[0], param, posicionX, posicionY, tiempoTarea);
                        _send_message(conexion_IMS, "IMS", COMENZAR_EJECUCION_TAREA, buffer, tamanioBuffer, aux->logger);
                        free(buffer);
                        mensajeInicialIMS = 1;
                        log_info(aux->logger, "despues de mandar mensaje a IMS");
                    }
                    
                    if(esTareaIO(tarea[0])){ //Si es de IO manda signal a hilo de Exec a Bloqueado
                        log_info(aux->logger, "[Tripulante %d] Se debe realizar una tarea de I/O", aux -> idSemaforo);
                        sleep(1);
                        tcbTripulante->status = 'I';
                        log_info(aux->logger,"[Tripulante %d] Ejecuto POST de semEBIO con hilo %d", aux -> idSemaforo, aux -> idSemaforo);
                        sem_post(&semEBIO);
                    }
                    else if (esTareaIO(tarea[0]) == 0) {
                        log_info(aux->logger, "[Tripulante %d] Se debe realizar una tarea normal (no de I/O)", aux -> idSemaforo);
                        tcbTripulante->tiempoEnExec++;
                        tcbTripulante->ciclosCumplidos++;
                        log_info(aux->logger, "El tiempo en exec es: %d", tcbTripulante->tiempoEnExec);
                        if(tcbTripulante->tiempoEnExec == tiempoTarea){
                            log_info(aux->logger, "[Tripulante %d] Termino la tarea normal el tripulante %d", aux -> idSemaforo, tcbTripulante->tid);
                            tamanioBuffer = sizeof(int)*2 + tamanioTarea;
                            buffer = _serialize(tamanioBuffer, "%d%s", tcbTripulante->tid, tarea[0]);
                            _send_message(conexion_IMS, "IMS", FINALIZAR_EJECUCION_TAREA, buffer, tamanioBuffer, logger);
                            free(buffer);

                            puedeEnviarSignal = pedirProximaTarea(tcbTripulante);
                            tcbTripulante->tiempoEnExec = 0;
                        } 

                        if(puedeEnviarSignal >= 0 && !strcmp(algoritmo,"RR") && tcbTripulante->ciclosCumplidos==quantum_RR){
                            tcbTripulante->status = 'R';
                            tcbTripulante->ciclosCumplidos = 0;
                            log_info(aux->logger,"[Tripulante %d] Ejecuto POST de semER con hilo %d", aux -> idSemaforo, aux -> idSemaforo);
                            sem_post(&semER);
                        }
                        else if(puedeEnviarSignal >= 0 && ((!strcmp(algoritmo,"RR") && tcbTripulante->ciclosCumplidos!=quantum_RR) || !strcmp(algoritmo,"FIFO"))){
                            cantidadTripulantesEnExec = queue_size(exec);
                            log_info(aux->logger,"[Tripulante %d] Ejecuto _SIGNAL con hilo %d", aux -> idSemaforo, aux -> idSemaforo);
                            _signal(1, cantidadTripulantesEnExec, &semBLOCKIO);
                        }
                        else
                            log_info(aux->logger,"[Tripulante %d] No es un algoritmo válido", aux -> idSemaforo);
                    }

                    else{
                        log_error(aux->logger, "[Tripulante %d] La tarea ingresada no posee un formato de tarea correcto", aux -> idSemaforo);
                    }

                }
                else{
                    log_info(aux->logger, "[Tripulante %d] Se comienza a mover el tripulante %d a la posicion %d - %d", aux -> idSemaforo, tcbTripulante -> tid, posicionX, posicionY);
                    moverTripulanteUno(tcbTripulante, posicionX, posicionY);
                    log_info(aux->logger, "[Tripulante %d] se movio una posicion el tripulante", aux -> idSemaforo);
                    tcbTripulante->ciclosCumplidos++;
                    if(!strcmp(algoritmo,"RR") && tcbTripulante->ciclosCumplidos==quantum_RR){
                        tcbTripulante->status = 'R';
                        tcbTripulante->ciclosCumplidos = 0;
                        log_info(aux->logger,"[Tripulante %d] Ejecuto POST de semER con hilo %d", aux -> idSemaforo, aux -> idSemaforo);
                        sem_post(&semER);
                    }
                    else if(!strcmp(algoritmo,"RR") && tcbTripulante->ciclosCumplidos!=quantum_RR || !strcmp(algoritmo,"FIFO")){
                        cantidadTripulantesEnExec = queue_size(exec);
                        log_info(aux->logger,"[Tripulante %d] Ejecuto _SIGNAL con hilo %d", aux -> idSemaforo, aux -> idSemaforo);
                        _signal(1, cantidadTripulantesEnExec, &semBLOCKIO);
                    }
                    else
                        log_info(aux->logger,"No es un algoritmo válido");
                }

                free(parametrosTarea[0]);
                free(parametrosTarea[1]);
                free(parametrosTarea[2]);
                free(parametrosTarea[3]);
                free(parametrosTarea);
                free(tarea[0]);
                free(tarea[1]);
                free(tarea);
            }
        }else{
            free(tcbTripulante->instruccion_actual);
            free(tcbTripulante);
            break;
        }
    }
}

/*-------------------------------ADICIONALES------------------------------*/

bool llegoAPosicion(int tripulante_posX,int tripulante_posY,int posX, int posY  ){
    return tripulante_posX== posX && tripulante_posY == posY;
}

int pedirProximaTarea(tcb* tcbTripulante){
    int cantidadTripulantesEnExec;
    int tamanioBuffer;
    int tamanioTarea;
    void* buffer;
    tamanioBuffer = sizeof(int) * 2;

    buffer = _serialize(tamanioBuffer, "%d%d", tcbTripulante->pid, tcbTripulante->tid);
    _send_message(conexion_RAM, "DIS",ENVIAR_TAREA, buffer, tamanioBuffer, logger);
    free(buffer);

    t_mensaje *mensaje = _receive_message(conexion_RAM, logger);

    if (mensaje->command == SUCCESS) {
        memcpy(&tamanioTarea, mensaje->payload, sizeof(int));
       
        tcbTripulante->instruccion_actual = malloc(tamanioTarea + 1);
        memcpy(tcbTripulante->instruccion_actual, mensaje->payload + sizeof(int), tamanioTarea);
        tcbTripulante->instruccion_actual[tamanioTarea] = '\0';
        log_info(logger, "Tripulante: %d ya tiene una nueva tarea a realizar",tcbTripulante->tid);
        free(mensaje->payload);
        free(mensaje->identifier);
        free(mensaje);
        return 1;
    }
    else if (mensaje->command == ERROR_NO_HAY_TAREAS) {
        log_info(logger, "Tripulante: %d ya realizó todas las tareas", tcbTripulante->tid);
        tcbTripulante->status = 'X';
        free(mensaje->payload);
        free(mensaje->identifier);
        free(mensaje);
        sem_post(&semEaX);
    }
    return -1;
}

int esTareaIO(char *tarea) {
    log_info(logger, "COMPARANDO TAREA: [%s]", tarea);
    if(!strcmp(tarea, "GENERAR_OXIGENO")) {
            return 1;
    }
    if(!strcmp(tarea, "CONSUMIR_OXIGENO")) {
            return 1;
    }
    if(!strcmp(tarea, "GENERAR_COMIDA")) {
            return 1;
    }
    if(!strcmp(tarea, "CONSUMIR_COMIDA")) {
            return 1;
    }
    if(!strcmp(tarea, "GENERAR_BASURA")) {
            return 1;
    }
    if(!strcmp(tarea, "DESCARTAR_BASURA")) {
            return 1;
    }

    for(int i = 0; i < strlen(tarea); i++) {
        if (tarea[i] == ';') {
            return 0;
        }
    };

    return -1;//si el formato de la tarea es incorrecto
}

void moverTripulanteUno(tcb* tcbTrip, int posXfinal, int posYfinal){
    int tamanioBufferARAM;
    int tamanioBufferAIMS;
    void* bufferARAM;
    void* bufferAIMS;
    int posXVieja;
    int posYVieja;
    log_info(logger, "Moviendo al tripulante %d", tcbTrip->tid);
    log_info(logger, "Posicion X Nueva %d", posXfinal);
    log_info(logger, "Posicion Y Nueva %d", posYfinal);
    log_info(logger, "Posicion X Actual %d", tcbTrip->posicionX);
    log_info(logger, "Posicion Y Actual %d", tcbTrip->posicionY);
    if (tcbTrip->posicionX < posXfinal){
        tcbTrip->posicionX++;
        /*
        //Notificar desplazamiento a RAM
        tamanioBufferARAM = sizeof(int)*4;
        bufferARAM = _serialize(tamanioBufferARAM, "%d%d%d%d", tcbTrip->pid, tcbTrip->tid, tcbTrip->posicionX, tcbTrip->posicionY);
        _send_message(conexion_RAM, "RAM", RECIBIR_UBICACION_TRIPULANTE, bufferARAM, tamanioBufferARAM, logger);
        free(bufferARAM);
        
        //Notificar desplazamiento a IMS
        posXVieja = tcbTrip->posicionX - 1;
        tamanioBufferAIMS = sizeof(int)*5;
        bufferAIMS = _serialize(tamanioBufferAIMS, "%d%d%d%d%d", tcbTrip->tid, posXVieja, tcbTrip->posicionY, tcbTrip->posicionX, tcbTrip->posicionY);
        _send_message(conexion_IMS, "IMS", MOVER_TRIPULANTE, bufferAIMS, tamanioBufferAIMS, logger);
        free(bufferAIMS);*/
    }
    else if (tcbTrip->posicionX > posXfinal){
        tcbTrip->posicionX--;
        /*
        //Notificar desplazamiento a RAM
        tamanioBufferARAM = sizeof(int)*4;
        bufferARAM = _serialize(tamanioBufferARAM, "%d%d%d%d", tcbTrip->pid, tcbTrip->tid, tcbTrip->posicionX, tcbTrip->posicionY);
        _send_message(conexion_RAM, "RAM", RECIBIR_UBICACION_TRIPULANTE, bufferARAM, tamanioBufferARAM, logger);
        free(bufferARAM);
        
        //Notificar desplazamiento a IMS
        posXVieja = tcbTrip->posicionX + 1;
        tamanioBufferAIMS = sizeof(int)*5;
        bufferAIMS = _serialize(tamanioBufferAIMS, "%d%d%d%d%d", tcbTrip->tid, posXVieja, tcbTrip->posicionY, tcbTrip->posicionX, tcbTrip->posicionY);
        _send_message(conexion_IMS, "IMS", MOVER_TRIPULANTE, bufferAIMS, tamanioBufferAIMS, logger);
        free(bufferAIMS);*/
    }
    else if (tcbTrip->posicionY < posYfinal){
        tcbTrip->posicionY++;
        /*
        //Notificar desplazamiento a RAM
        tamanioBufferARAM = sizeof(int)*4;
        bufferARAM = _serialize(tamanioBufferARAM, "%d%d%d%d", tcbTrip->pid, tcbTrip->tid, tcbTrip->posicionX, tcbTrip->posicionY);
        _send_message(conexion_RAM, "RAM", RECIBIR_UBICACION_TRIPULANTE, bufferARAM, tamanioBufferARAM, logger);
        free(bufferARAM);

        //Notificar desplazamiento a IMS
        posYVieja = tcbTrip->posicionY - 1;
        tamanioBufferAIMS = sizeof(int)*5;
        bufferAIMS = _serialize(tamanioBufferAIMS, "%d%d%d%d%d", tcbTrip->tid, tcbTrip->posicionX, posYVieja, tcbTrip->posicionX, tcbTrip->posicionY);
        _send_message(conexion_IMS, "IMS", MOVER_TRIPULANTE, bufferAIMS, tamanioBufferAIMS, logger);
        free(bufferAIMS);*/
    }
    else if (tcbTrip->posicionY > posYfinal){
        tcbTrip->posicionY--;
        /*
        //Notificar desplazamiento a RAM
        tamanioBufferARAM = sizeof(int)*4;
        bufferARAM = _serialize(tamanioBufferARAM, "%d%d%d%d", tcbTrip->pid, tcbTrip->tid, tcbTrip->posicionX, tcbTrip->posicionY);
        _send_message(conexion_RAM, "RAM", RECIBIR_UBICACION_TRIPULANTE, bufferARAM, tamanioBufferARAM, logger);
        free(bufferARAM);

        //Notificar desplazamiento a IMS
        posYVieja = tcbTrip->posicionY + 1;
        tamanioBufferAIMS = sizeof(int)*5;
        bufferAIMS = _serialize(tamanioBufferAIMS, "%d%d%d%d%d", tcbTrip->tid, tcbTrip->posicionX, posYVieja, tcbTrip->posicionX, tcbTrip->posicionY);
        _send_message(conexion_IMS, "IMS", MOVER_TRIPULANTE, bufferAIMS, tamanioBufferAIMS, logger);
        free(bufferAIMS);*/
    }
    else{
        log_info(logger, "El tripulante ya llegó a la posición de la tarea");
    }

    log_info(logger, "Posiciones despues de mover al tripulante %d", tcbTrip->tid);
    log_info(logger, "Posicion X Actual %d", tcbTrip->posicionX);
    log_info(logger, "Posicion Y Actual %d", tcbTrip->posicionY);
}


char *get_tareas(char *ruta_archivo, t_log* logger) {
    FILE * fp;
    char * line = NULL;
    size_t len = 0;
    ssize_t read;

    int b_size = 0;
    int offset = 0;

    char *temp_string = string_new();

    fp = fopen(ruta_archivo, "r");

    if (fp == NULL){
        exit(EXIT_FAILURE);
    }
    while ((read = getline(&line, &len, fp)) != -1) {
        if (line[ read - 1 ] == '\n') {
            read--;
            memset(line + read, 0, 1);
        }
        string_append(&temp_string, line);
    }

    fclose(fp);
    if (line)
        free(line);
    return temp_string;
}

void create_tcb_by_list(t_list* self, void(*closure)(void*, int, int, t_log*), int conexion_RAM, int cantidad_inicial, t_log *logger) {
    int indice_tcb_temporal = cantidad_inicial;
	t_link_element *element = self->head;
	t_link_element *aux = NULL;
	while (element != NULL) {
		aux = element->next;
		closure(element->data, conexion_RAM, indice_tcb_temporal, logger);
        indice_tcb_temporal++;
		element = aux;
	}
}

void iniciar_tcb(void *elemento, int conexion_RAM, int indice_tcb_temporal, t_log *logger) {

	tcb *aux = (tcb *) elemento;
  	int tamanioBuffer = sizeof(int) * 2;
  	void *buffer = _serialize(tamanioBuffer, "%d%d", aux->pid, aux->tid);
  	_send_message(conexion_RAM, "DIS", ENVIAR_TAREA, buffer, tamanioBuffer, logger);
    free(buffer);

  	t_mensaje *mensaje = _receive_message(conexion_RAM, logger);
  	if (mensaje->command == SUCCESS) { // Recibi la primer tarea
        int tamanioTarea;
        memcpy(&tamanioTarea, mensaje->payload, sizeof(int));
        aux->instruccion_actual = malloc(tamanioTarea + 1);
        memcpy(aux->instruccion_actual, mensaje->payload + sizeof(int), tamanioTarea);
        aux->instruccion_actual[tamanioTarea] = '\0';
        aux->estaVivoElHilo = 1;
        queue_push (cola_new, (void*) aux);

        // for(int i=cantidadVieja; i<cantidadActual; i++){
        parametrosThread *parametros = malloc(sizeof(parametrosThread));
        parametros->logger=logger;
        parametros->idSemaforo=indice_tcb_temporal;

        
        pthread_create(&hiloTripulante[indice_tcb_temporal], NULL, (void *) funcionTripulante, parametros);
            //pthread_detach(hiloTripulante);

        // }
       
    } else {
    	log_error(logger, "No hay tareas disponibles");
    }

    free(mensaje->identifier);
    free(mensaje->payload);
    free(mensaje);
}

void * get_by_id(t_list * self, int id) {
    tcb *nodo;
    t_link_element *element = self->head;
	t_link_element *aux = NULL;
	while (element != NULL) {
		aux = element->next;
		tcb *nodo = (tcb *) (element->data);
        if (nodo -> tid == id) {
            return nodo;
        }
		element = aux;
	}
    return NULL;
}

void _signal(int incremento, int valorMax, sem_t *semaforo) {
    log_info(logger, "el valor antes de incrementar es: %d y el valor maximo es: %d", contadorSemGlobal, valorMax);
    contadorSemGlobal += incremento;
    log_info(logger, "el valor despues de incrementar es: %d y el valor maximo es: %d", contadorSemGlobal, valorMax);
    if (contadorSemGlobal == valorMax) {
        log_info(logger, "manda signal a bloqio");
        sem_post(semaforo);
        contadorSemGlobal = 0;
    }
}