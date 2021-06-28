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
    bool cumplioElTiempoEnExec = false;
    bool esTareaNormal = false;
    tcb *tcbTripulante;

    log_info(aux->logger,"----------------------------------");

    pthread_mutex_lock(&mutexValidador);
    int temp_validador = validador;
    pthread_mutex_unlock(&mutexValidador);

    while(temp_validador){
        sem_wait(&semTripulantes[aux->idSemaforo]);


        pthread_mutex_lock(&mutexExec);
        tcbTripulante = get_by_id(exec->elements, aux->idSemaforo);
        pthread_mutex_unlock(&mutexExec);
        
        log_info(aux->logger, "Se ejecuta el hilo de Funcion Tripulante (Exec) de Tripulante:%d ", tcbTripulante->tid);
        
        tcbTripulante->tiempoEnExec = 0;
        if(tcbTripulante->estaVivoElHilo){
            
            log_info(aux->logger, "Tripulante:%d no fue expulsado ni terminó su ejecución de la tarea");
            if(tcbTripulante->status == 'E' && planificacion_viva){
                log_info(aux->logger, "Se ejecuta la tarea del tripulante");

                //Spliteo la tarea
                tarea = string_n_split(tcbTripulante->instruccion_actual, 1, " ");
                esTareaNormal = tarea[1]==NULL;

                if (!esTareaNormal){
                    parametrosTarea = string_split(tarea[1], ";");
                }else{
                    parametrosTarea = string_split(tarea[0], ";");
                    strcpy(parametrosTarea[0],"-1");
                }


                int posicionX = atoi(parametrosTarea[1]);
                int posicionY = atoi(parametrosTarea[2]);
                int tiempo = atoi(parametrosTarea[3]);
                
                llegoALaPosicion = llegoAPosicion(tcbTripulante->posicionX,tcbTripulante->posicionY,posicionX,posicionY);
                cumplioElTiempoEnExec = tcbTripulante->tiempoEnExec == tiempo;
                
                if(llegoALaPosicion){
                    log_info(aux->logger, "El tripulante llegó a la posición");
                    log_info(aux->logger, "Se procede a enviar los mensajes a los otros procesos..");

                    if(mensajeInicialIMS == 0){//Manda mensaje a IMS de "Comienza Ejecucion Tarea"
                        tamanioTarea = strlen(tarea[0]);
                        tamanioBuffer = sizeof(int)*5 + tamanioTarea + strlen(parametros[0]);
                        buffer = _serialize(tamanioBuffer, "%d%s%d%d%d%d", tcbTripulante->tid, tarea[0], atoi(parametrosTarea[0]), atoi(parametrosTarea[1]), atoi(parametrosTarea[2]), atoi(parametrosTarea[3]));
                        _send_message(conexion_IMS, "IMS", COMENZAR_EJECUCION_TAREA, buffer, tamanioBuffer, logger);
                        free(buffer);
                        mensajeInicialIMS = 1;
                    }
                    
                    if(esTareaNormal && cumplioElTiempoEnExec){ //Si no es de IO y termino CPU
                        tamanioBuffer = sizeof(int)*2 + tamanioTarea;
                        buffer = _serialize(tamanioBuffer, "%d%s", tcbTripulante->tid, tarea[0]);
                        _send_message(conexion_IMS, "IMS", FINALIZAR_EJECUCION_TAREA, buffer, tamanioBuffer, logger);
                        free(buffer);
                        
                        pedirProximaTarea(tcbTripulante);
                    } 
                    else if(esTareaIO(tarea[0])){ //Si es de IO manda signal a hilo de Exec a Bloqueado
                        log_info(logger, "Se debe realizar una tarea de I/O");
                        sleep(1);
                        tcbTripulante->status = 'I';
                        sem_post(&semEBIO);
                    }
                    else if (esTareaIO(tarea[0]) == 0) {
                        log_info(logger, "Se debe realizar una tarea normal (no de I/O)");
                        tcbTripulante->tiempoEnExec ++;
                        tcbTripulante->ciclosCumplidos ++;

                        if(!strcmp(algoritmo,"RR") && tcbTripulante->ciclosCumplidos==quantum_RR){
                            tcbTripulante->status = 'R';
                            tcbTripulante->ciclosCumplidos = 0;
                            sem_post(&semER);
                        }
                        else if(!strcmp(algoritmo,"RR") && tcbTripulante->ciclosCumplidos!=quantum_RR || !strcmp(algoritmo,"FIFO")){
                            cantidadTripulantesEnExec = queue_size(exec);
                            _signal(1, cantidadTripulantesEnExec, semBLOCKIO);
                        }
                        else
                            log_info(logger,"No es un algoritmo válido");
                    }

                    else{
                        log_error(logger, "La tarea ingresada no posee un formato de tarea correcto");
                    }

                }
                else{
                    log_info(aux->logger, "Se comienza a mover el tripulante una posicion..");
                    moverTripulanteUno(tcbTripulante, atoi(parametros[1]), atoi(parametros[2]));
                    tcbTripulante->ciclosCumplidos++;
                    if(!strcmp(algoritmo,"RR") && tcbTripulante->ciclosCumplidos==quantum_RR){
                        tcbTripulante->status = 'R';
                        tcbTripulante->ciclosCumplidos = 0;
                        sem_post(&semER);
                    }
                    else if(!strcmp(algoritmo,"RR") && tcbTripulante->ciclosCumplidos!=quantum_RR || !strcmp(algoritmo,"FIFO")){
                        cantidadTripulantesEnExec = queue_size(exec);
                        _signal(1, cantidadTripulantesEnExec, semBLOCKIO);
                    }
                    else
                        log_info(logger,"No es un algoritmo válido");
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

void pedirProximaTarea(tcb* tcbTripulante){
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
        log_info(logger, "Tripulante:%d ya tiene una nueva tarea a realizar.",tcbTripulante->tid);
    }
    else if (mensaje->command == ERROR_NO_HAY_TAREAS) {
        log_info(logger, "Tripulante:%d ya realizó todas las tareas", tcbTripulante->tid);
        tcbTripulante->status = 'X';
        sem_wait(&semEaX);
    }

    free(mensaje->payload);
    free(mensaje->identifier);
    free(mensaje);
}

int esTareaIO(char *tarea) {
    if(!strcmp(tarea, "GENERAR_OXIGENO")) {
            return 1;
    }else if(!strcmp(tarea, "CONSUMIR_OXIGENO")) {
            return 1;
    }else if(!strcmp(tarea, "GENERAR_COMIDA")) {
            return 1;
    }else if(!strcmp(tarea, "CONSUMIR_COMIDA")) {
            return 1;
    }else if(!strcmp(tarea, "GENERAR_BASURA")) {
            return 1;
    }else if(!strcmp(tarea, "DESCARTAR_BASURA")) {
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
    if (tcbTrip->posicionX != posXfinal){
        tcbTrip->posicionX++;

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
        free(bufferAIMS);
    }
    else if (tcbTrip->posicionY != posYfinal){
        tcbTrip->posicionY++;

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
        free(bufferAIMS);
    }
    else{
        log_info(logger, "El tripulante ya llegó a la posición de la tarea");
    }
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

        for(int i=cantidadVieja; i<cantidadActual; i++){
            parametrosThread *parametros = malloc(sizeof(parametrosThread));
            parametros->logger=logger;
            parametros->idSemaforo=i;

            
            pthread_create(&hiloTripulante[i], NULL, (void *) funcionTripulante, parametros);
            //pthread_detach(hiloTripulante);

        }
       
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

void _signal(int incremento, int valorMax, sem_t semaforo) {
    contadorSemGlobal += incremento;
    if (contadorSemGlobal == valorMax) {
        sem_post(&semaforo);
        contadorSemGlobal = 0;
    }
}