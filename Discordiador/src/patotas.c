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
  	nuevoPCB->rutaTareas = malloc(strlen(parametros[2]) + 1);
    strcpy(nuevoPCB->rutaTareas, parametros[2]);
    nuevoPCB->rutaTareas[strlen(parametros[2])]='\0';
    nuevoPCB->listaTCB = list_create();
      
  	char *tareas = get_tareas(nuevoPCB->rutaTareas, logger);
		
  	// buffer->[idpcb, largo_tareas, lista_tareas, cant_tripulantes, n_tcbs ()....]
  	int tamanioBuffer = sizeof(int) * 3 + strlen(tareas) + cant_tripulantes * (sizeof(int)*4 + sizeof(char));
  	void *buffer_a_enviar = malloc(tamanioBuffer);	
  
  	memcpy(buffer_a_enviar, &nuevoPCB->pid, sizeof(int));
    offset += sizeof(int);

  	void *temp = _serialize(sizeof(int) + strlen(tareas), "%s", tareas);
  
  	memcpy(buffer_a_enviar + offset, temp, sizeof(int) + strlen(tareas));
  	offset += sizeof(int) + strlen(tareas);
    free(tareas);
  	free(temp);
    
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

    }
    cantidadActual += cant_tripulantes;
    _send_message(conexion_RAM, "DIS", INICIAR_PATOTA, buffer_a_enviar, tamanioBuffer, logger);
    free(buffer_a_enviar);
    
  	t_mensaje *mensaje = _receive_message(conexion_RAM, logger);

  	if (mensaje->command == SUCCESS) {
        log_info(logger,"Se guardó en Memoria OK");
    }
 
    free(mensaje->payload);
    free(mensaje->identifier);
    free(mensaje);
  	return nuevoPCB;
}

void destruirTCB(void* nodo){
    tcb* tcbADestruir = (tcb*) nodo;
    free(tcbADestruir->instruccion_actual);
    //free(tcbADestruir);
}

void destruirPCB(void* nodo){
    pcb* pcbADestruir = (pcb*) nodo;
    free(pcbADestruir->rutaTareas);
    list_destroy_and_destroy_elements(pcbADestruir->listaTCB, destruirTCB);
    // free(pcbADestruir);
}


/*---------------------------FUNCION TRIPULANTE (EXEC)----------------------*/
void funcionTripulante (void* item) {
    parametrosThread* aux = (parametrosThread*) item;
    int tamanioBuffer;
    int tamanioTarea;
    void* buffer;
    char** parametrosTarea;
    char** tarea;
    int mensajeInicialIMS=0;
    bool llegoALaPosicion = false;
    int puedeEnviarSignal = 1;
    int parametro;
    int posicionX;
    int posicionY;
    int tiempoTarea;
    tcb *tcbTripulante;

    log_info(aux->logger,"----------------------------------");
    log_info(aux->logger,"[Tripulante %d] Esperando Signal...", aux->idSemaforo);

    pthread_mutex_lock(&mutexValidador);
    int temp_validador = validador;
    pthread_mutex_unlock(&mutexValidador);

    while(temp_validador){// MIENTRAS ESTÉ EN FUNCIONAMIENTO EL PROCESO
        sem_wait(&semTripulantes[aux->idSemaforo]);
        log_info(aux->logger,"[Tripulante %d] EJECUTANDO...", aux->idSemaforo);

        pthread_mutex_lock(&mutexExec);
        tcbTripulante = get_by_id(exec->elements, aux->idSemaforo);
        pthread_mutex_unlock(&mutexExec);

        if(tcbTripulante->estaVivoElHilo){// SI ESTÁ VIVO EL TRIPULANTE (HILO)
            
            if(tcbTripulante->status == 'E' && planificacion_viva){//SI EL TRIULANTE ESTÁ EN EXEC Y LA PLANIFICACIÓN NO ESTÁ PAUSADA
                log_info(aux->logger, "[Tripulante %d] esta en ejecución", aux->idSemaforo);

                if(sabotaje_activado){// SI HAY UN SABOTAJE
                    log_info(aux->logger,"[Tripulante %d] DATOS TRIPULANTE: %d - ATENDIENDO SABOTAJE", aux->idSemaforo, tcbTripulante -> tid);

                    log_info(aux->logger, "La posición en X del sabotaje es: %d", posSabotajeX);
                    log_info(aux->logger, "La posición en Y del sabotaje es: %d", posSabotajeY);
                    log_info(aux->logger, "La duración del sabotaje es: %d", duracion_sabotaje);
                    llegoALaPosicion = llegoAPosicion(tcbTripulante->posicionX, tcbTripulante->posicionY, posSabotajeX, posSabotajeY);

                    if (llegoALaPosicion){// LLEGÓ A LA POSICIÓN DEL SABOTAJE

                        pthread_mutex_lock(&mutexCiclosTranscurridosSabotaje);
                        if (ciclos_transcurridos_sabotaje == 0) {//SI TODAVÍA NO SE COMENZÓ A REPARAR EL SABOTAJE
                            pthread_mutex_unlock(&mutexCiclosTranscurridosSabotaje);

                            char* bufferAEnviar = string_new();
                            string_append(&bufferAEnviar, "Se invoco FSCK");
                            buffer = _serialize(sizeof(int) + string_length(bufferAEnviar),"%s", bufferAEnviar);
                            _send_message(conexion_IMS, "DIS", INVOCAR_FSCK, buffer,sizeof(int) + string_length(bufferAEnviar), logger);
                            free(bufferAEnviar);
                            free(buffer);
                        }

                        pthread_mutex_lock(&mutexCiclosTranscurridosSabotaje);
                        if (ciclos_transcurridos_sabotaje == duracion_sabotaje) {//SI SE TERMINÓ DE REPARAR EL SABOTAJE
                            pthread_mutex_unlock(&mutexCiclosTranscurridosSabotaje);
                            sem_post(&semERM);
                        }
                        else {// SI NO SE TERMINÓ DE REPARAR EL SABOTAJE
                            pthread_mutex_unlock(&mutexCiclosTranscurridosSabotaje);
                            tcbTripulante->ciclosCumplidos++;

                            pthread_mutex_lock(&mutexCiclosTranscurridosSabotaje);
                            ciclos_transcurridos_sabotaje++;
                            pthread_mutex_unlock(&mutexCiclosTranscurridosSabotaje);

                            if(!strcmp(algoritmo,"RR") && tcbTripulante->ciclosCumplidos == quantum_RR) {// SI EL ALGORITMO ES RR Y SE COMPLETÓ EL QUANTUM
                                tcbTripulante->ciclosCumplidos = 0;
                                tcbTripulante->status='R';
                            }
                            sem_post(&semER);
                        }
                    }

                    else {// NO LLEGÓ A LA POSICIÓN DEL SABOTAJE
                        log_info(aux->logger, "[Tripulante %d] Se comienza a mover a la posición %d - %d", aux -> idSemaforo, posSabotajeX, posSabotajeY);
                        moverTripulanteUno(tcbTripulante, posSabotajeX, posSabotajeY);
                        log_info(aux->logger, "[Tripulante %d] se movio una posicion el tripulante", aux -> idSemaforo);
                        tcbTripulante->ciclosCumplidos++;

                        if(!strcmp(algoritmo,"RR") && tcbTripulante->ciclosCumplidos==quantum_RR){// SI EL ALGORITMO ES RR Y SE COMPLETÓ EL QUANTUM
                            tcbTripulante->status = 'R';
                            tcbTripulante->ciclosCumplidos = 0;
                            log_info(aux->logger,"[Tripulante %d] Ejecuto POST de semER con hilo %d", aux -> idSemaforo, aux -> idSemaforo);
                            sem_post(&semER);
                        }
                        else if((!strcmp(algoritmo,"RR") && tcbTripulante->ciclosCumplidos!=quantum_RR )|| !strcmp(algoritmo,"FIFO")){// SI ES FIFO O RR Y NO COMPLETÓ EL QUANTUM
                            log_info(aux->logger,"[Tripulante %d] Ejecuto _SIGNAL con hilo %d", aux -> idSemaforo, aux -> idSemaforo);
                            sem_post(&semER);
                        }
                        else // NO ES FIFO NI RR
                            log_info(aux->logger,"No es un algoritmo válido");
                    }
                }


                else {// SI NO HAY UN SABOTAJE
                    log_info(aux->logger,"[Tripulante %d] DATOS TRIPULANTE: %d - %s", aux->idSemaforo, tcbTripulante -> tid, tcbTripulante -> instruccion_actual);

                    tarea = string_split(tcbTripulante->instruccion_actual, " ");

                    if (tarea[1]!=NULL){
                    log_info(aux->logger, "Es tarea IO");
                    parametrosTarea = string_split(tarea[1], ";");
                    }
                    else{
                    log_info(aux->logger, "Es tarea normal");
                    parametrosTarea = string_split(tarea[0], ";");
                    strcpy(parametrosTarea[0],"-1");
                    }
                    parametro = atoi(parametrosTarea[0]);
                    posicionX = atoi(parametrosTarea[1]);
                    posicionY = atoi(parametrosTarea[2]);
                    tiempoTarea = atoi(parametrosTarea[3]);
                
                    log_info(aux->logger, "El parametro de la tarea es: %d", parametro);
                    log_info(aux->logger, "La posición en X de la tarea es: %d", posicionX);
                    log_info(aux->logger, "La posición en Y de la tarea es: %d", posicionY);
                    log_info(aux->logger, "El tiempo de la tarea es: %d", tiempoTarea);
                    llegoALaPosicion = llegoAPosicion(tcbTripulante->posicionX, tcbTripulante->posicionY, posicionX, posicionY);

                    if (llegoALaPosicion){// LLEGÓ A LA POSICIÓN DE LA TAREA
                        log_info(aux->logger, "[Tripulante %d] Llegó a la posición, la tarea es %s, es tarea IO? %d", aux -> idSemaforo, tarea[0], esTareaIO(tarea[0]));
                        
                        if (mensajeInicialIMS == 0){//MANDA MENSAJE A IMS DE "Comienza Ejecucion Tarea"
                            tamanioTarea = strlen(tarea[0]);
                            tamanioBuffer = sizeof(int)*6 + tamanioTarea;
                            buffer = _serialize(tamanioBuffer, "%d%s%d%d%d%d", tcbTripulante->tid, tarea[0], parametro, posicionX, posicionY, tiempoTarea);
                            _send_message(conexion_IMS, "IMS", COMENZAR_EJECUCION_TAREA, buffer, tamanioBuffer, aux->logger);
                            free(buffer);
                            mensajeInicialIMS = 1;
                        }

                        if (esTareaIO(tarea[0])) {// ES TAREA DE I/O
                            log_info(aux->logger, "[Tripulante %d] Se debe realizar una tarea de I/O", aux -> idSemaforo);
                            sleep(1);
                            tcbTripulante->status = 'I';
                            tcbTripulante->ciclosCumplidos = 0;
                            log_info(aux->logger,"[Tripulante %d] Ejecuto POST de semEBIO con hilo %d", aux -> idSemaforo, aux -> idSemaforo);
                            sem_post(&semEBIO);
                        }
                        else if (esTareaIO(tarea[0]) == 0) {// ES TAREA NORMAL
                            log_info(aux->logger, "[Tripulante %d] Se debe realizar una tarea normal (no de I/O)", aux -> idSemaforo);
                            tcbTripulante->tiempoEnExec++;
                            tcbTripulante->ciclosCumplidos++;
                            log_info(aux->logger, "El tiempo en exec es: %d", tcbTripulante->tiempoEnExec);

                            if(tcbTripulante->tiempoEnExec == tiempoTarea){// COMPLETÓ LA TAREA NORMAL
                                log_info(aux->logger, "[Tripulante %d] Termino la tarea normal el tripulante %d", aux -> idSemaforo, tcbTripulante->tid);
                                tamanioBuffer = sizeof(int)*2 + tamanioTarea;
                                buffer = _serialize(tamanioBuffer, "%d%s", tcbTripulante->tid, tarea[0]);
                                _send_message(conexion_IMS, "IMS", FINALIZAR_EJECUCION_TAREA, buffer, tamanioBuffer, logger);
                                free(buffer);

                                puedeEnviarSignal = pedirProximaTarea(tcbTripulante);
                                tcbTripulante->tiempoEnExec = 0;
                            } 

                            if(puedeEnviarSignal >= 0 && !strcmp(algoritmo,"RR") && tcbTripulante->ciclosCumplidos==quantum_RR){//ES RR Y COMPLETÓ EL QUANTUM
                                tcbTripulante->status = 'R';
                                tcbTripulante->ciclosCumplidos = 0;
                                log_info(aux->logger,"[Tripulante %d] Ejecuto POST de semER con hilo %d", aux -> idSemaforo, aux -> idSemaforo);
                                sem_post(&semER);
                            }
                            else {// ES FIFO O RR Y NO COMPLETÓ EL QUANTUM
                                log_info(aux->logger,"[Tripulante %d] Ejecuto _SIGNAL con hilo %d", aux -> idSemaforo, aux -> idSemaforo);
                                _signal(1, cantidadTCBEnExec, &semBLOCKIO);
                            }
                        }
                        else
                            log_error(aux->logger, "[Tripulante %d] La tarea ingresada no posee un formato de tarea correcto", aux -> idSemaforo);
            
                    }
                    else {// NO LLEGÓ A LA POSICIÓN DE LA TAREA
                        log_info(aux->logger, "[Tripulante %d] Se comienza a mover a la posición %d - %d", aux -> idSemaforo, posicionX, posicionY);
                        moverTripulanteUno(tcbTripulante, posicionX, posicionY);
                        log_info(aux->logger, "[Tripulante %d] se movio una posicion el tripulante", aux -> idSemaforo);
                        tcbTripulante->ciclosCumplidos++;

                        if(!strcmp(algoritmo,"RR") && tcbTripulante->ciclosCumplidos==quantum_RR){// SI EL ALGORITMO ES RR Y SE COMPLETÓ EL QUANTUM
                            tcbTripulante->status = 'R';
                            tcbTripulante->ciclosCumplidos = 0;
                            log_info(aux->logger,"[Tripulante %d] Ejecuto POST de semER con hilo %d", aux -> idSemaforo, aux -> idSemaforo);
                            sem_post(&semER);
                        }
                        else if((!strcmp(algoritmo,"RR") && tcbTripulante->ciclosCumplidos!=quantum_RR )|| !strcmp(algoritmo,"FIFO")){// SI ES FIFO O RR Y NO COMPLETÓ EL QUANTUM
                            log_info(aux->logger,"[Tripulante %d] Ejecuto _SIGNAL con hilo %d", aux -> idSemaforo, aux -> idSemaforo);
                            _signal(1, cantidadTCBEnExec, &semBLOCKIO);
                        }
                        else //NO ES FIFO NI RR
                            log_info(aux->logger,"No es un algoritmo válido");
                    }
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

        }
        else {// NO ESTÁ VIVO EL TRIPULANTE (HILO)
            free(tcbTripulante->instruccion_actual);
            free(tcbTripulante);
            break;
        }
    }

    free(aux);
}

/*-------------------------------ADICIONALES------------------------------*/

bool llegoAPosicion(int tripulante_posX,int tripulante_posY,int posX, int posY  ){
    return tripulante_posX== posX && tripulante_posY == posY;
}

int pedirProximaTarea(tcb* tcbTripulante){
    log_info(logger, "entra a proxtarea");
    int tamanioBuffer;
    void* buffer;
    tamanioBuffer = sizeof(int) * 2;

    buffer = _serialize(tamanioBuffer, "%d%d", tcbTripulante->pid, tcbTripulante->tid);
    _send_message(conexion_RAM, "DIS",ENVIAR_TAREA, buffer, tamanioBuffer, logger);
    free(buffer);

    t_mensaje *mensajito = _receive_message(conexion_RAM, logger);

    if (mensajito->command == SUCCESS) {
        log_info(logger, "SUCCESS: Pedirproximatarea");
        
        // void* temporal = malloc(mensajito->pay_len + 1);
        // memmove(temporal, mensajito->payload , mensajito->pay_len);
         int tamanioTareaRecibida;

        memcpy(&tamanioTareaRecibida, mensajito->payload, sizeof(int));
        log_info(logger, "El tamaño de la tarea es: %d", tamanioTareaRecibida);

        char* buffer_recibido = malloc(tamanioTareaRecibida + 1);
        memmove(buffer_recibido,mensajito->payload + sizeof(int), tamanioTareaRecibida);
        buffer_recibido[tamanioTareaRecibida]='\0';
        
        log_info(logger, "El tamanio de la tarea recibida es:%d", tamanioTareaRecibida);
        log_info(logger, "Buffer recibido: %s", buffer_recibido);


        free(tcbTripulante->instruccion_actual);
        
        tcbTripulante->instruccion_actual = malloc(tamanioTareaRecibida+1);
        memmove(tcbTripulante->instruccion_actual, mensajito->payload + sizeof(int), tamanioTareaRecibida);
        tcbTripulante->instruccion_actual[tamanioTareaRecibida] = '\0';
        
        log_info(logger, "La tarea del tripulante %d ahora es: %s",tcbTripulante->tid, tcbTripulante->instruccion_actual);
        free(mensajito->payload);
        free(mensajito->identifier);
        free(mensajito);
        return 1;
    }
    else if (mensajito->command == ERROR_NO_HAY_TAREAS) {
        log_info(logger, "Tripulante: %d ya realizó todas las tareas", tcbTripulante->tid);
        tcbTripulante->status = 'X';
        free(mensajito->payload);
        free(mensajito->identifier);
        free(mensajito);
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

    t_mensaje *mensajito;

    log_info(logger, "Moviendo al tripulante %d", tcbTrip->tid);
    log_info(logger, "Posicion X Nueva %d", posXfinal);
    log_info(logger, "Posicion Y Nueva %d", posYfinal);
    log_info(logger, "Posicion X Actual %d", tcbTrip->posicionX);
    log_info(logger, "Posicion Y Actual %d", tcbTrip->posicionY);
    if (tcbTrip->posicionX < posXfinal){
        tcbTrip->posicionX++;
        //Notificar desplazamiento a RAM
        tamanioBufferARAM = sizeof(int)*4;
        bufferARAM = _serialize(tamanioBufferARAM, "%d%d%d%d", tcbTrip->pid, tcbTrip->tid, tcbTrip->posicionX, tcbTrip->posicionY);
        _send_message(conexion_RAM, "RAM", RECIBIR_UBICACION_TRIPULANTE, bufferARAM, tamanioBufferARAM, logger);
        free(bufferARAM);

        mensajito = _receive_message(conexion_RAM, logger);
        if (mensajito->command != SUCCESS) {
            perror("Comando RECIBIR_UBICACION_TRIPULANTE\n");
        }
        free(mensajito -> identifier);
        free(mensajito -> payload);
        free(mensajito);
        
        //Notificar desplazamiento a IMS
        posXVieja = tcbTrip->posicionX - 1;
        tamanioBufferAIMS = sizeof(int)*5;
        bufferAIMS = _serialize(tamanioBufferAIMS, "%d%d%d%d%d", tcbTrip->tid, posXVieja, tcbTrip->posicionY, tcbTrip->posicionX, tcbTrip->posicionY);
        _send_message(conexion_IMS, "IMS", MOVER_TRIPULANTE, bufferAIMS, tamanioBufferAIMS, logger);
        free(bufferAIMS);

    }
    else if (tcbTrip->posicionX > posXfinal){
        tcbTrip->posicionX--;
        
        //Notificar desplazamiento a RAM
        tamanioBufferARAM = sizeof(int)*4;
        bufferARAM = _serialize(tamanioBufferARAM, "%d%d%d%d", tcbTrip->pid, tcbTrip->tid, tcbTrip->posicionX, tcbTrip->posicionY);
        _send_message(conexion_RAM, "RAM", RECIBIR_UBICACION_TRIPULANTE, bufferARAM, tamanioBufferARAM, logger);
        free(bufferARAM);

        mensajito = _receive_message(conexion_RAM, logger);
        if (mensajito->command != SUCCESS) {
            perror("Comando RECIBIR_UBICACION_TRIPULANTE\n");
        }
        free(mensajito -> identifier);
        free(mensajito -> payload);
        free(mensajito);
        
        //Notificar desplazamiento a IMS
        posXVieja = tcbTrip->posicionX + 1;
        tamanioBufferAIMS = sizeof(int)*5;
        bufferAIMS = _serialize(tamanioBufferAIMS, "%d%d%d%d%d", tcbTrip->tid, posXVieja, tcbTrip->posicionY, tcbTrip->posicionX, tcbTrip->posicionY);
        _send_message(conexion_IMS, "IMS", MOVER_TRIPULANTE, bufferAIMS, tamanioBufferAIMS, logger);
        free(bufferAIMS);

    }
    else if (tcbTrip->posicionY < posYfinal){
        tcbTrip->posicionY++;
        
        //Notificar desplazamiento a RAM
        tamanioBufferARAM = sizeof(int)*4;
        bufferARAM = _serialize(tamanioBufferARAM, "%d%d%d%d", tcbTrip->pid, tcbTrip->tid, tcbTrip->posicionX, tcbTrip->posicionY);
        _send_message(conexion_RAM, "RAM", RECIBIR_UBICACION_TRIPULANTE, bufferARAM, tamanioBufferARAM, logger);
        free(bufferARAM);

        mensajito = _receive_message(conexion_RAM, logger);
        if (mensajito->command != SUCCESS) {
            perror("Comando RECIBIR_UBICACION_TRIPULANTE\n");
        }
        free(mensajito -> identifier);
        free(mensajito -> payload);
        free(mensajito);

        //Notificar desplazamiento a IMS
        posYVieja = tcbTrip->posicionY - 1;
        tamanioBufferAIMS = sizeof(int)*5;
        bufferAIMS = _serialize(tamanioBufferAIMS, "%d%d%d%d%d", tcbTrip->tid, tcbTrip->posicionX, posYVieja, tcbTrip->posicionX, tcbTrip->posicionY);
        _send_message(conexion_IMS, "IMS", MOVER_TRIPULANTE, bufferAIMS, tamanioBufferAIMS, logger);
        free(bufferAIMS);

    }
    else if (tcbTrip->posicionY > posYfinal){
        tcbTrip->posicionY--;
        
        //Notificar desplazamiento a RAM
        tamanioBufferARAM = sizeof(int)*4;
        bufferARAM = _serialize(tamanioBufferARAM, "%d%d%d%d", tcbTrip->pid, tcbTrip->tid, tcbTrip->posicionX, tcbTrip->posicionY);
        _send_message(conexion_RAM, "RAM", RECIBIR_UBICACION_TRIPULANTE, bufferARAM, tamanioBufferARAM, logger);
        free(bufferARAM);

        mensajito = _receive_message(conexion_RAM, logger);
        if (mensajito->command != SUCCESS) {
            perror("Comando RECIBIR_UBICACION_TRIPULANTE\n");
        }
        free(mensajito -> identifier);
        free(mensajito -> payload);
        free(mensajito);

        //Notificar desplazamiento a IMS
        posYVieja = tcbTrip->posicionY + 1;
        tamanioBufferAIMS = sizeof(int)*5;
        bufferAIMS = _serialize(tamanioBufferAIMS, "%d%d%d%d%d", tcbTrip->tid, tcbTrip->posicionX, posYVieja, tcbTrip->posicionX, tcbTrip->posicionY);
        _send_message(conexion_IMS, "IMS", MOVER_TRIPULANTE, bufferAIMS, tamanioBufferAIMS, logger);
        free(bufferAIMS);
    }
    else{
        log_info(logger, "El tripulante ya llegó a la posición de la tarea");
    }

    log_info(logger, "Posiciones despues de mover al tripulante %d", tcbTrip->tid);
    log_info(logger, "Posicion X Actual %d", tcbTrip->posicionX);
    log_info(logger, "Posicion Y Actual %d", tcbTrip->posicionY);
}

char *get_tareas(char *ruta_archivo, t_log* logger) {
    FILE* archivo;
    // FILE* archivo2;
    char* stringTemporal = string_new();
    char * line = NULL;
    size_t len = 0;
    ssize_t read;
    int contador = 0;

    archivo = fopen(ruta_archivo,"r");

    while ((read = getline(&line, &len, archivo)) != -1) {
        contador++;
    }
    log_info(logger,"%d",contador);
    fclose(archivo);

    archivo = fopen(ruta_archivo,"r");
    while ((read = getline(&line, &len, archivo)) != -1) {
        int posicionFinal = read;
        if(contador == 1){ //ULTIMA LINEA /UNICA LINEA
            posicionFinal = read;
        }else{
            posicionFinal = read-1;
        }
        contador--;
        memset(line+posicionFinal,'\0',1);
        string_append(&stringTemporal, line);
    }
    fclose(archivo);
    if (line)
        free(line);

    log_info(logger, "Las tareas a mandar son: %s", stringTemporal);
    return stringTemporal;

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
        memmove(&tamanioTarea, mensaje->payload, sizeof(int));
        log_info(logger, "el tamaño de la tarea es: %d", tamanioTarea);
        aux->instruccion_actual = malloc(tamanioTarea + 1);
        memmove(aux->instruccion_actual, mensaje->payload + sizeof(int), tamanioTarea);
        aux->instruccion_actual[tamanioTarea] = '\0';
        log_info(logger, "la tarea es: %s", aux->instruccion_actual);

        aux->estaVivoElHilo = 1;
        pthread_mutex_lock(&mutexNew);
        queue_push (cola_new, (void*) aux);
        pthread_mutex_unlock(&mutexNew);

        // for(int i=cantidadVieja; i<cantidadActual; i++){
        parametrosThread *parametros = malloc(sizeof(parametrosThread));
        parametros->logger=logger;
        parametros->idSemaforo=indice_tcb_temporal;

        
        pthread_create(&hiloTripulante[indice_tcb_temporal], NULL, (void *) funcionTripulante, parametros);
        // pthread_detach(&hiloTripulante);

        // }
       
    } else {
    	log_error(logger, "No hay tareas disponibles");
    }

    free(mensaje->identifier);
    free(mensaje->payload);
    free(mensaje);
}

void * get_by_id(t_list * self, int id) {
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

    contadorSemGlobal += incremento;
    if (contadorSemGlobal == valorMax) {
        log_info(logger, "manda signal a bloqio");

        pthread_mutex_lock(&mutex_cantidadTCB);
        cantidadTCBEnExec = queue_size(exec);
        pthread_mutex_unlock(&mutex_cantidadTCB);

        sem_post(semaforo);
        contadorSemGlobal = 0;
    }
}

