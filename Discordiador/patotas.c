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
    nuevoPCB->todosLosTCBsTerminaron = 0;
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
    log_info(logger, "CantidadActual: %d", cantidadActual);
    
    pthread_mutex_lock(&mutexBuffer);
    _send_message(conexion_RAM, "DIS", INICIAR_PATOTA, buffer_a_enviar, tamanioBuffer, logger);
    free(buffer_a_enviar);

  	t_mensaje *mensaje = _receive_message(conexion_RAM, logger);
    pthread_mutex_unlock(&mutexBuffer);

  	if (mensaje->command == SUCCESS) {
        log_info(logger,"Se guardó en Memoria OK");
    }
    else if (mensaje->command == ERROR_POR_FALTA_DE_MEMORIA){
        free(mensaje->payload);
        free(mensaje->identifier);
        free(mensaje);
        
        return NULL;
    }
    free(mensaje->payload);
    free(mensaje->identifier);
    free(mensaje);

  	return nuevoPCB;
}

void destruirTCB(void* nodo){
    tcb* tcbADestruir = (tcb*) nodo;
    if(tcbADestruir && tcbADestruir->instruccion_actual){
        free(tcbADestruir->instruccion_actual);
    }
}

void destruirPCB(void* nodo){
    pcb* pcbADestruir = (pcb*) nodo;
    free(pcbADestruir->rutaTareas);
    list_destroy_and_destroy_elements(pcbADestruir->listaTCB, destruirTCB);
}


/*---------------------------FUNCION TRIPULANTE (EXEC)----------------------*/
void funcionTripulante (void* elemento) {
    parametrosThread* param = (parametrosThread*) elemento;
    int tamanioBuffer;
    int tamanioTarea;
    int tamanioNombreTarea;
    char* nombreTareaNormal;
    int parametroIO;
    void* buffer;
    char** parametrosTarea;
    char** tarea;
    int mensajeInicialIMS=0;
    bool llegoALaPosicion = false;
    int posicionX;
    int posicionY;
    int tiempoTarea;
    tcb *tcbTripulante;

    pthread_mutex_lock(&mutexValidador);
    int temp_validador = validador;
    pthread_mutex_unlock(&mutexValidador);

    while(temp_validador){// MIENTRAS ESTÉ EN FUNCIONAMIENTO EL PROCESO

        sem_t* semaforo = list_get(listaSemaforos,param->idSemaforo);
        sem_wait(semaforo);

        pthread_mutex_lock(&mutexExec);
        tcbTripulante = get_by_id(exec->elements, param->idSemaforo);
        pthread_mutex_unlock(&mutexExec);

        if(!tcbTripulante){
            pthread_mutex_lock(&mutexExit);
            tcbTripulante = get_by_id(cola_exit->elements, param->idSemaforo);
            pthread_mutex_unlock(&mutexExit);
        }
        
        if(tcbTripulante->estaVivoElHilo && planificacion_viva){// SI ESTÁ VIVO EL TRIPULANTE (HILO)
            log_info(logger, "[Tripulante %d] esta en ejecución", param->idSemaforo);

            // SI HAY UN SABOTAJE:
            if(sabotaje_activado){
                log_info(logger,"[Tripulante %d] Se está atendiendo sabotaje", param->idSemaforo);

                log_info(logger, "La posición en X|Y del sabotaje es: %d|%d con duración: %d", posSabotajeX, posSabotajeY, duracion_sabotaje);   
                llegoALaPosicion = llegoAPosicion(tcbTripulante->posicionX, tcbTripulante->posicionY, posSabotajeX, posSabotajeY);

                if (llegoALaPosicion){// LLEGÓ A LA POSICIÓN DEL SABOTAJE

                    pthread_mutex_lock(&mutexCiclosTranscurridosSabotaje);
                    int temp_ciclos_transcurridos_sabotaje = ciclos_transcurridos_sabotaje;
                    pthread_mutex_unlock(&mutexCiclosTranscurridosSabotaje);

                    if (temp_ciclos_transcurridos_sabotaje == 0) {//SI TODAVÍA NO SE COMENZÓ A REPARAR EL SABOTAJE
                        char* bufferAEnviar = string_new();
                        string_append(&bufferAEnviar, "Se invoco FSCK");

                        pthread_mutex_lock(&mutexBuffer);
                        buffer = _serialize(sizeof(int) + string_length(bufferAEnviar),"%s", bufferAEnviar);
                        _send_message(conexion_IMS, "DIS", INVOCAR_FSCK, buffer,sizeof(int) + string_length(bufferAEnviar), logger);
                        free(bufferAEnviar);
                        free(buffer);
                        pthread_mutex_unlock(&mutexBuffer);
                    }

                    tcbTripulante->ciclosCumplidos++;
                    log_info(logger, "Ciclos transcurridos del sabotaje: %d", tcbTripulante->ciclosCumplidos);

                    pthread_mutex_lock(&mutexCiclosTranscurridosSabotaje);
                    ciclos_transcurridos_sabotaje++;
                    pthread_mutex_unlock(&mutexCiclosTranscurridosSabotaje);

                    if(!strcmp(algoritmo,"RR") && tcbTripulante->ciclosCumplidos == quantum_RR) {// SI EL ALGORITMO ES RR Y SE COMPLETÓ EL QUANTUM
                        tcbTripulante->ciclosCumplidos = 0;
                        tcbTripulante->status='R';
                    }
                }

                else {// NO LLEGÓ A LA POSICIÓN DEL SABOTAJE
                    moverTripulanteUno(tcbTripulante, posSabotajeX, posSabotajeY);
                    tcbTripulante->ciclosCumplidos++;

                    if(!strcmp(algoritmo,"RR") && tcbTripulante->ciclosCumplidos==quantum_RR){// SI EL ALGORITMO ES RR Y SE COMPLETÓ EL QUANTUM
                        tcbTripulante->status = 'R';
                        tcbTripulante->ciclosCumplidos = 0;
                    }
                }
            }

            // SI NO HAY UN SABOTAJE:
            else {

                tarea = string_split(tcbTripulante->instruccion_actual, " ");

                if (esTareaIO(tarea[0])) {// ES TAREA DE I/O

                    parametrosTarea = string_split(tarea[1], ";");
                    parametroIO = atoi(parametrosTarea[0]);
                    posicionX = atoi(parametrosTarea[1]);
                    posicionY = atoi(parametrosTarea[2]);
                    tiempoTarea = atoi(parametrosTarea[3]);

                    llegoALaPosicion = llegoAPosicion(tcbTripulante->posicionX, tcbTripulante->posicionY, posicionX, posicionY);

                    if (llegoALaPosicion){// LLEGÓ A LA POSICIÓN DE LA TAREA
                        log_info(logger, "[Tripulante %d] Llegó a la posición, la tarea es %s, es tarea IO",param->idSemaforo, tarea[0]);
                
                        tamanioTarea = strlen(tarea[0]);
                        tamanioBuffer = sizeof(int)*6 + tamanioTarea;

                        pthread_mutex_lock(&mutexBuffer);
                        buffer = _serialize(tamanioBuffer, "%d%s%d%d%d%d", tcbTripulante->tid, tarea[0], parametroIO, posicionX, posicionY, tiempoTarea);
                        _send_message(conexion_IMS, "DIS", COMENZAR_EJECUCION_TAREA, buffer, tamanioBuffer, logger);
                        free(buffer);
                        pthread_mutex_unlock(&mutexBuffer);

                        tcbTripulante->status = 'I';
                        tcbTripulante->ciclosCumplidos = 0;
                        log_info(logger, "[Tripulante %d] Se debe realizar una tarea de I/O",param->idSemaforo);
                        sleep(ciclo_CPU);
                        free(tarea[1]);
                    }

                    else{// NO LLEGÓ A LA POSICIÓN DE LA TAREA
                        moverTripulanteUno(tcbTripulante, posicionX, posicionY);
                        tcbTripulante->ciclosCumplidos++;

                        if(!strcmp(algoritmo,"RR") && tcbTripulante->ciclosCumplidos==quantum_RR){//ES RR Y COMPLETÓ EL QUANTUM
                            tcbTripulante->status = 'R';
                            tcbTripulante->ciclosCumplidos = 0;
                            log_info(logger, "El Tripulante: %d completó su quantum, se debe mover a Ready", tcbTripulante->tid);
                        }
                    }
                }

                else if (esTareaIO(tarea[0]) == 0) {// ES TAREA NORMAL
                    log_info(logger, "[Tripulante %d] Se debe realizar una tarea normal",param->idSemaforo);

                    parametrosTarea = string_split(tarea[0], ";");
                    nombreTareaNormal = malloc(strlen(parametrosTarea[0]) + 1);
                    strcpy(nombreTareaNormal, parametrosTarea[0]);
                    posicionX = atoi(parametrosTarea[1]);
                    posicionY = atoi(parametrosTarea[2]);
                    tiempoTarea = atoi(parametrosTarea[3]);

                    llegoALaPosicion = llegoAPosicion(tcbTripulante->posicionX, tcbTripulante->posicionY, posicionX, posicionY);

                    if(llegoALaPosicion){

                        if(mensajeInicialIMS==0){// NO SE MANDÓ TODAVÍA EL MENSAJE A IMS DE INICIO DE TAREA
                            tamanioNombreTarea = strlen(nombreTareaNormal);
                            tamanioBuffer = sizeof(int)*5 + tamanioNombreTarea;

                            pthread_mutex_lock(&mutexBuffer);
                            buffer = _serialize(tamanioBuffer, "%d%s%d%d%d", tcbTripulante->tid, nombreTareaNormal, posicionX, posicionY, tiempoTarea);
                            _send_message(conexion_IMS, "DIS", COMENZAR_EJECUCION_TAREA, buffer, tamanioBuffer, logger);
                            free(buffer);
                            pthread_mutex_unlock(&mutexBuffer);
                            mensajeInicialIMS = 1;
                        }

                        tcbTripulante->tiempoEnExec++;
                        tcbTripulante->ciclosCumplidos++;
                        log_info(logger, "El tiempo en exec del tripulante %d es: %d", tcbTripulante->tid, tcbTripulante->tiempoEnExec);

                        if(tcbTripulante->tiempoEnExec == tiempoTarea){// COMPLETÓ LA TAREA NORMAL
                            log_info(logger, "[Tripulante %d] Termino la tarea normal el tripulante", tcbTripulante->tid);
                            tamanioNombreTarea = strlen(nombreTareaNormal);
                            tamanioBuffer = sizeof(int)*2 + tamanioNombreTarea;
                            
                            pthread_mutex_lock(&mutexBuffer);
                            buffer = _serialize(tamanioBuffer, "%d%s", tcbTripulante->tid, nombreTareaNormal);
                            _send_message(conexion_IMS, "DIS", FINALIZAR_EJECUCION_TAREA, buffer, tamanioBuffer, logger);
                            free(buffer);
                            pthread_mutex_unlock(&mutexBuffer);

                            pedirProximaTarea(tcbTripulante);
                            tcbTripulante->tiempoEnExec = 0;
                            mensajeInicialIMS = 0;
                        } 

                        if(!strcmp(algoritmo,"RR") && tcbTripulante->ciclosCumplidos==quantum_RR && tcbTripulante->status != 'X'){//ES RR Y COMPLETÓ EL QUANTUM Y NO TERMINÓ TODAS LAS TAREAS
                            tcbTripulante->status = 'R';
                            tcbTripulante->ciclosCumplidos = 0;
                            log_info(logger, "El Tripulante: %d completó su quantum, se debe mover a Ready", tcbTripulante->tid);
                        }
                        
                    }
                    else{// NO LLEGÓ A LA POSICIÓN DE LA TAREA
                        moverTripulanteUno(tcbTripulante, posicionX, posicionY);
                        tcbTripulante->ciclosCumplidos++;

                        if(!strcmp(algoritmo,"RR") && tcbTripulante->ciclosCumplidos==quantum_RR){//ES RR Y COMPLETÓ EL QUANTUM
                            tcbTripulante->status = 'R';
                            tcbTripulante->ciclosCumplidos = 0;
                            log_info(logger, "El Tripulante: %d completó su quantum, se debe mover a Ready", tcbTripulante->tid);
                        }
                    }
                    free(nombreTareaNormal);
                }

                else{
                    log_error(logger, "[Tripulante %d] La tarea ingresada no posee un formato de tarea correcto",param->idSemaforo);
                }

                free(parametrosTarea[0]);
                free(parametrosTarea[1]);
                free(parametrosTarea[2]);
                free(parametrosTarea[3]);
                free(parametrosTarea);
                free(tarea[0]);
                free(tarea);
            }

            sleep(ciclo_CPU);
        }
        _signal(1,cantidadTCBEnExec,&semEBIO);
    }
}

/*-------------------------------ADICIONALES------------------------------*/

bool llegoAPosicion(int tripulante_posX,int tripulante_posY,int posX, int posY  ){
    return tripulante_posX== posX && tripulante_posY == posY;
}

void pedirProximaTarea(tcb* tcbTripulante){
    int tamanioBuffer;
    void* buffer;
    tamanioBuffer = sizeof(int) * 2;

    pthread_mutex_lock(&mutexBuffer);
    buffer = _serialize(tamanioBuffer, "%d%d", tcbTripulante->pid, tcbTripulante->tid);
    _send_message(conexion_RAM, "DIS",ENVIAR_TAREA, buffer, tamanioBuffer, logger);
    free(buffer);

    t_mensaje *mensajito = _receive_message(conexion_RAM, logger);
    pthread_mutex_unlock(&mutexBuffer);

    if (mensajito->command == SUCCESS) {
        int tamanioTareaRecibida;

        memcpy(&tamanioTareaRecibida, mensajito->payload, sizeof(int));

        char* buffer_recibido = malloc(tamanioTareaRecibida + 1);
        memmove(buffer_recibido,mensajito->payload + sizeof(int), tamanioTareaRecibida);
        buffer_recibido[tamanioTareaRecibida]='\0';
    
        free(tcbTripulante->instruccion_actual);
        
        tcbTripulante->instruccion_actual = malloc(tamanioTareaRecibida+1);
        memmove(tcbTripulante->instruccion_actual, mensajito->payload + sizeof(int), tamanioTareaRecibida);
        tcbTripulante->instruccion_actual[tamanioTareaRecibida] = '\0';
        
        log_info(logger, "La Tarea de Tripulante %d ahora es: %s",tcbTripulante->tid, tcbTripulante->instruccion_actual);
        free(mensajito->payload);
        free(mensajito->identifier);
        free(mensajito);
        free(buffer_recibido);
    }
    else if (mensajito->command == ERROR_NO_HAY_TAREAS) {
        log_info(logger, "Tripulante: %d ya realizó todas las tareas", tcbTripulante->tid);
        tcbTripulante->status = 'X';
        free(mensajito->payload);
        free(mensajito->identifier);
        free(mensajito);
    }
}

int esTareaIO(char *tarea) {
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

    log_info(logger, "Moviendo al tripulante %d de %d|%d a %d|%d", tcbTrip->tid,tcbTrip->posicionX,tcbTrip->posicionY,posXfinal,posYfinal);
    if (tcbTrip->posicionX < posXfinal){
        tcbTrip->posicionX++;
        //Notificar desplazamiento a RAM
        tamanioBufferARAM = sizeof(int)*4;

        bufferARAM = _serialize(tamanioBufferARAM, "%d%d%d%d", tcbTrip->pid, tcbTrip->tid, tcbTrip->posicionX, tcbTrip->posicionY);
        
        pthread_mutex_lock(&mutexBuffer);
        _send_message(conexion_RAM, "DIS", RECIBIR_UBICACION_TRIPULANTE, bufferARAM, tamanioBufferARAM, logger);
        free(bufferARAM);

        mensajito = _receive_message(conexion_RAM, logger);
        pthread_mutex_unlock(&mutexBuffer);

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
        
        pthread_mutex_lock(&mutexBuffer);
        _send_message(conexion_IMS, "DIS", MOVER_TRIPULANTE, bufferAIMS, tamanioBufferAIMS, logger);
        pthread_mutex_unlock(&mutexBuffer);
        free(bufferAIMS);
        log_info(logger, "El tripulante %d fue movido de %d|%d a %d|%d", tcbTrip->tid, posXVieja, tcbTrip->posicionY, tcbTrip->posicionX, tcbTrip->posicionY);
    }
    else if (tcbTrip->posicionX > posXfinal){
        tcbTrip->posicionX--;
        
        //Notificar desplazamiento a RAM
        tamanioBufferARAM = sizeof(int)*4;
        bufferARAM = _serialize(tamanioBufferARAM, "%d%d%d%d", tcbTrip->pid, tcbTrip->tid, tcbTrip->posicionX, tcbTrip->posicionY);
        
        pthread_mutex_lock(&mutexBuffer);
        _send_message(conexion_RAM, "DIS", RECIBIR_UBICACION_TRIPULANTE, bufferARAM, tamanioBufferARAM, logger);
        free(bufferARAM);

        mensajito = _receive_message(conexion_RAM, logger);
        pthread_mutex_unlock(&mutexBuffer);
        
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
       
        pthread_mutex_lock(&mutexBuffer);
        _send_message(conexion_IMS, "DIS", MOVER_TRIPULANTE, bufferAIMS, tamanioBufferAIMS, logger);
        pthread_mutex_unlock(&mutexBuffer);
        
        free(bufferAIMS);
        log_info(logger, "El tripulante %d fue movido de %d|%d a %d|%d", tcbTrip->tid, posXVieja, tcbTrip->posicionY, tcbTrip->posicionX, tcbTrip->posicionY);
    }
    else if (tcbTrip->posicionY < posYfinal){
        tcbTrip->posicionY++;
        
        //Notificar desplazamiento a RAM
        tamanioBufferARAM = sizeof(int)*4;
        bufferARAM = _serialize(tamanioBufferARAM, "%d%d%d%d", tcbTrip->pid, tcbTrip->tid, tcbTrip->posicionX, tcbTrip->posicionY);
        
        pthread_mutex_lock(&mutexBuffer);
        _send_message(conexion_RAM, "DIS", RECIBIR_UBICACION_TRIPULANTE, bufferARAM, tamanioBufferARAM, logger);
        free(bufferARAM);

        mensajito = _receive_message(conexion_RAM, logger);
        pthread_mutex_unlock(&mutexBuffer);
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
        
        pthread_mutex_lock(&mutexBuffer);
        _send_message(conexion_IMS, "DIS", MOVER_TRIPULANTE, bufferAIMS, tamanioBufferAIMS, logger);
        pthread_mutex_unlock(&mutexBuffer);
        free(bufferAIMS);
        log_info(logger, "El tripulante %d fue movido de %d|%d a %d|%d", tcbTrip->tid, tcbTrip->posicionX, posYVieja, tcbTrip->posicionX, tcbTrip->posicionY);

    }
    else if (tcbTrip->posicionY > posYfinal){
        tcbTrip->posicionY--;
        
        //Notificar desplazamiento a RAM
        tamanioBufferARAM = sizeof(int)*4;
        bufferARAM = _serialize(tamanioBufferARAM, "%d%d%d%d", tcbTrip->pid, tcbTrip->tid, tcbTrip->posicionX, tcbTrip->posicionY);
        
        pthread_mutex_lock(&mutexBuffer);
        _send_message(conexion_RAM, "DIS", RECIBIR_UBICACION_TRIPULANTE, bufferARAM, tamanioBufferARAM, logger);
        free(bufferARAM);

        mensajito = _receive_message(conexion_RAM, logger);
        pthread_mutex_unlock(&mutexBuffer);
        
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
        
        pthread_mutex_lock(&mutexBuffer);
        _send_message(conexion_IMS, "DIS", MOVER_TRIPULANTE, bufferAIMS, tamanioBufferAIMS, logger);
        pthread_mutex_unlock(&mutexBuffer);
        
        free(bufferAIMS);
        log_info(logger, "El tripulante %d fue movido de %d|%d a %d|%d", tcbTrip->tid, tcbTrip->posicionX, posYVieja, tcbTrip->posicionX, tcbTrip->posicionY);
    }
    else{
        log_info(logger, "El tripulante ya llegó a la posición de la tarea");
    }

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
    
    pthread_mutex_lock(&mutexBuffer);
  	buffer = _serialize(tamanioBuffer, "%d%d", aux->pid, aux->tid);
  	_send_message(conexion_RAM, "DIS", ENVIAR_TAREA, buffer, tamanioBuffer, logger);
    free(buffer);

  	t_mensaje *mensaje = _receive_message(conexion_RAM, logger);
    pthread_mutex_unlock(&mutexBuffer);
  	
    if (mensaje->command == SUCCESS) { // Recibi la primer tarea
        int tamanioTarea;
        memmove(&tamanioTarea, mensaje->payload, sizeof(int));

        aux->instruccion_actual = malloc(tamanioTarea + 1);
        memmove(aux->instruccion_actual, mensaje->payload + sizeof(int), tamanioTarea);
        aux->instruccion_actual[tamanioTarea] = '\0';
        
        log_info(logger, "Tarea Inicial: %s", aux->instruccion_actual);

        aux->estaVivoElHilo = 1;

        pthread_mutex_lock(&mutexNew);
        queue_push (cola_new, (void*) aux);
        pthread_mutex_unlock(&mutexNew);

        sem_t* semTripulante = malloc(sizeof(sem_t));
        sem_init(semTripulante, 0, 0);

        list_add(listaSemaforos, semTripulante);
        parametrosThread* parametrosHilo = malloc (sizeof(parametrosThread));
        parametrosHilo->idSemaforo=indice_tcb_temporal;

        pthread_create(&hiloTripulante[parametrosHilo->idSemaforo], NULL, (void *) funcionTripulante, parametrosHilo);
        pthread_detach(hiloTripulante[parametrosHilo->idSemaforo]);
       
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

void _signal(int incremento, int valorMax, sem_t* semaforo) {

    contadorSemGlobal += incremento;

    if (contadorSemGlobal == valorMax) {
        pthread_mutex_lock(&mutex_cantidadTCB);
        cantidadTCBEnExec = queue_size(exec);
        pthread_mutex_unlock(&mutex_cantidadTCB);

        sem_post(semaforo);
        contadorSemGlobal = 0;
    }
}


