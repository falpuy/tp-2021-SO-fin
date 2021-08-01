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
    nuevoTCB->mensajeInicialIMS = 0;

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
    pthread_mutex_lock(&mutex_cantidadActual);
    cantidadActual += cant_tripulantes;
    pthread_mutex_unlock(&mutex_cantidadActual);
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
     if(tcbADestruir){
         if(tcbADestruir->instruccion_actual){
             free(tcbADestruir->instruccion_actual);
        }
        free(tcbADestruir);
     }
     
}

void destruirPCB(void* nodo){
    pcb* pcbADestruir = (pcb*) nodo;
    free(pcbADestruir->rutaTareas);
    list_destroy_and_destroy_elements(pcbADestruir->listaTCB, destruirTCB);
    free(pcbADestruir);
}


/*---------------------------FUNCION TRIPULANTE (EXEC)----------------------*/
void funcionTripulante (void* elemento) {
    parametrosThread* param = (parametrosThread*) elemento;
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
            log_info(logger, "El tripulante es: %d",tcbTripulante->tid);
        }       
        
        hiloTripulanteYPlaniVivos((void*) tcbTripulante,(void*) param);

        //pthread_mutex_lock(&mutex_cantidadTCB); 
        _signal(1,cantidadTCBEnExec,&semEBIO);
        //pthread_mutex_unlock(&mutex_cantidadTCB);
    }
}

/*-------------------------------ADICIONALES------------------------------*/

void hiloTripulanteYPlaniVivos (void* tcbTrip, void* param){
    tcb* tripulante = (tcb*) tcbTrip;
    parametrosThread* parametros = (parametrosThread*) param;
    if(tripulante->estaVivoElHilo && planificacion_viva){// SI ESTÁ VIVO EL TRIPULANTE (HILO)
        if(sabotaje_activado){
            resolviendoSabotaje((void*) tripulante, (void*) parametros);
        }
        else{
            operandoSinSabotaje((void*) tripulante, (void*) parametros);
        }
        sleep(ciclo_CPU);
    }
}

void resolviendoSabotaje(void* tcbTrip, void* param){
    tcb* tripulante = (tcb*) tcbTrip;
    parametrosThread* parametros = (parametrosThread*) param;
    
    log_info(logger,"[Tripulante %d] Se está atendiendo sabotaje", parametros->idSemaforo);
    log_info(logger, "La posición en X|Y del sabotaje es: %d|%d con duración: %d", posSabotajeX, posSabotajeY, duracion_sabotaje);   
    int llegoALaPosicion = llegoAPosicion(tripulante->posicionX, tripulante->posicionY, posSabotajeX, posSabotajeY);

    if (llegoALaPosicion){// LLEGÓ A LA POSICIÓN DEL SABOTAJE

        pthread_mutex_lock(&mutexCiclosTranscurridosSabotaje);
        int temp_ciclos_transcurridos_sabotaje = ciclos_transcurridos_sabotaje;
        pthread_mutex_unlock(&mutexCiclosTranscurridosSabotaje);

        if (temp_ciclos_transcurridos_sabotaje == 0) {//SI TODAVÍA NO SE COMENZÓ A REPARAR EL SABOTAJE
            void* buffer;
            char* bufferAEnviar = string_new();
            string_append(&bufferAEnviar, "Se invoco FSCK");

            pthread_mutex_lock(&mutexBuffer);
            buffer = _serialize(sizeof(int) + string_length(bufferAEnviar),"%s", bufferAEnviar);
            _send_message(conexion_IMS, "DIS", INVOCAR_FSCK, buffer,sizeof(int) + string_length(bufferAEnviar), logger);
            free(bufferAEnviar);
            free(buffer);
            pthread_mutex_unlock(&mutexBuffer);
        }

        tripulante->ciclosCumplidos++;
        log_info(logger, "Ciclos transcurridos del sabotaje: %d", tripulante->ciclosCumplidos);

        pthread_mutex_lock(&mutexCiclosTranscurridosSabotaje);
        ciclos_transcurridos_sabotaje++;
        pthread_mutex_unlock(&mutexCiclosTranscurridosSabotaje);

        if(!strcmp(algoritmo,"RR") && tripulante->ciclosCumplidos == quantum_RR) {// SI EL ALGORITMO ES RR Y SE COMPLETÓ EL QUANTUM
            tripulante->ciclosCumplidos = 0;
            tripulante->status='R';
        }
    }
    else {// NO LLEGÓ A LA POSICIÓN DEL SABOTAJE
        moverTripulanteUno(tripulante, posSabotajeX, posSabotajeY);
        tripulante->ciclosCumplidos++;

        if(!strcmp(algoritmo,"RR") && tripulante->ciclosCumplidos==quantum_RR){// SI EL ALGORITMO ES RR Y SE COMPLETÓ EL QUANTUM
            tripulante->status = 'R';
            tripulante->ciclosCumplidos = 0;
        }
    }

}

void operandoSinSabotaje(void* tcbTrip, void* param){
    tcb* tripulante = (tcb*) tcbTrip;
    parametrosThread* parametros = (parametrosThread*) param;
    char** tarea = string_split(tripulante->instruccion_actual, " ");
    for(int i=0; tarea[i]!=NULL; i++){
        log_info(logger, "parametro: %s", tarea[i]);
    }

    if (esTareaIO(tarea[0])) {// ES TAREA DE I/O
        operandoSinSabotajeTareaIO((void*) tripulante, (void*) parametros, tarea);
    }
    else if (esTareaIO(tarea[0]) == 0) {// ES TAREA NORMAL
        operandoSinSabotajeTareaNormal((void*) tripulante, (void*) parametros, tarea);    
    }   
}

void operandoSinSabotajeTareaNormal(void* tcbTrip, void* param, char** tarea){

    tcb* tripulante = (tcb*) tcbTrip;
    parametrosThread* parametros = (parametrosThread*) param;
    log_info(logger, "[Tripulante %d] Se debe realizar una tarea normal",parametros->idSemaforo);

    char** parametrosTarea = string_split(tarea[0], ";");
    // for(int i=0; parametrosTarea[i]!=NULL; i++){
    //     log_info(logger, "parametro: %s", parametrosTarea[i]);
    // }
    char* nombreTareaNormal = malloc(strlen(parametrosTarea[0]) + 1);
    strcpy(nombreTareaNormal, parametrosTarea[0]);
    log_info(logger, "nombre de tarea (tarea normal): %s", nombreTareaNormal);
    int posicionX = atoi(parametrosTarea[1]);
    int posicionY = atoi(parametrosTarea[2]);
    int tiempoTarea = atoi(parametrosTarea[3]);

    int llegoALaPosicion = llegoAPosicion(tripulante->posicionX, tripulante->posicionY, posicionX, posicionY);

    if(llegoALaPosicion){

        if(tripulante->mensajeInicialIMS==0){// NO SE MANDÓ TODAVÍA EL MENSAJE A IMS DE INICIO DE TAREA
            int tamanioNombreTarea = strlen(nombreTareaNormal);
            int tamanioBuffer = sizeof(int)*5 + tamanioNombreTarea;
                    
            void* buffer;
            pthread_mutex_lock(&mutexBuffer);
            buffer = _serialize(tamanioBuffer, "%d%s%d%d%d", tripulante->tid, nombreTareaNormal, posicionX, posicionY, tiempoTarea);
            _send_message(conexion_IMS, "DIS", COMENZAR_EJECUCION_TAREA, buffer, tamanioBuffer, logger);
            free(buffer);
            tripulante->mensajeInicialIMS = 1;
            pthread_mutex_unlock(&mutexBuffer);
        }

        tripulante->tiempoEnExec++;
        tripulante->ciclosCumplidos++;
        log_info(logger, "El tiempo en exec del tripulante %d es: %d", tripulante->tid, tripulante->tiempoEnExec);

        if(tripulante->tiempoEnExec == tiempoTarea){// COMPLETÓ LA TAREA NORMAL
            log_info(logger, "[Tripulante %d] Termino la tarea normal el tripulante", tripulante->tid);
            int tamanioNombreTarea = strlen(nombreTareaNormal);
            int tamanioBuffer = sizeof(int)*2 + tamanioNombreTarea;
                        
            void* buffer;        
            pthread_mutex_lock(&mutexBuffer);
            buffer = _serialize(tamanioBuffer, "%d%s", tripulante->tid, nombreTareaNormal);
            _send_message(conexion_IMS, "DIS", FINALIZAR_EJECUCION_TAREA, buffer, tamanioBuffer, logger);
            free(buffer);
            pthread_mutex_unlock(&mutexBuffer);

            pthread_mutex_lock(&mutexListaPCB);
            pedirProximaTarea(tripulante);
            pthread_mutex_unlock(&mutexListaPCB);
            tripulante->tiempoEnExec = 0;
            tripulante->mensajeInicialIMS = 0;
        } 

        if(!strcmp(algoritmo,"RR") && tripulante->ciclosCumplidos==quantum_RR && tripulante->status != 'X'){//ES RR Y COMPLETÓ EL QUANTUM Y NO TERMINÓ TODAS LAS TAREAS
            tripulante->status = 'R';
            tripulante->ciclosCumplidos = 0;
            log_info(logger, "El Tripulante: %d completó su quantum, se debe mover a Ready", tripulante->tid);
        }                 
    }
    else{// NO LLEGÓ A LA POSICIÓN DE LA TAREA
        moverTripulanteUno(tripulante, posicionX, posicionY);
        tripulante->ciclosCumplidos++;

        if(!strcmp(algoritmo,"RR") && tripulante->ciclosCumplidos==quantum_RR){//ES RR Y COMPLETÓ EL QUANTUM
            tripulante->status = 'R';
            tripulante->ciclosCumplidos = 0;
            log_info(logger, "El Tripulante: %d completó su quantum, se debe mover a Ready", tripulante->tid);
        }
    }
    free(nombreTareaNormal);
    for(int i =0; parametrosTarea[i]!=NULL; i++){
        free(parametrosTarea[i]);
    }
    free(parametrosTarea);
    for(int i =0; tarea[i]!=NULL; i++){
        free(tarea[i]);
    }
    free(tarea);

}

void operandoSinSabotajeTareaIO(void* tcbTrip, void* param, char** tarea){
    
    tcb* tripulante = (tcb*) tcbTrip;
    parametrosThread* parametros = (parametrosThread*) param;
    
    char** parametrosTarea = string_split(tarea[1], ";");
    for(int i=0; parametrosTarea[i]!=NULL; i++){
        log_info(logger, "parametro: %s", parametrosTarea[i]);
    }
    int parametroIO = atoi(parametrosTarea[0]);
    int posicionX = atoi(parametrosTarea[1]);
    int posicionY = atoi(parametrosTarea[2]);
    int tiempoTarea = atoi(parametrosTarea[3]);

    int llegoALaPosicion = llegoAPosicion(tripulante->posicionX, tripulante->posicionY, posicionX, posicionY);

    if (llegoALaPosicion){// LLEGÓ A LA POSICIÓN DE LA TAREA
        log_info(logger, "[Tripulante %d] Llegó a la posición, la tarea es %s, es tarea IO",parametros->idSemaforo, tarea[0]);
                
        int tamanioTarea = strlen(tarea[0]);
        int tamanioBuffer = sizeof(int)*6 + tamanioTarea;

        void* buffer;
        pthread_mutex_lock(&mutexBuffer);
        buffer = _serialize(tamanioBuffer, "%d%s%d%d%d%d", tripulante->tid, tarea[0], parametroIO, posicionX, posicionY, tiempoTarea);
        _send_message(conexion_IMS, "DIS", COMENZAR_EJECUCION_TAREA, buffer, tamanioBuffer, logger);
        free(buffer);
        pthread_mutex_unlock(&mutexBuffer);

        tripulante->status = 'I';
        tripulante->ciclosCumplidos = 0;
        log_info(logger, "[Tripulante %d] Se debe realizar una tarea de I/O",parametros->idSemaforo);
    }
    else{// NO LLEGÓ A LA POSICIÓN DE LA TAREA
        moverTripulanteUno(tripulante, posicionX, posicionY);
        tripulante->ciclosCumplidos++;

        if(!strcmp(algoritmo,"RR") && tripulante->ciclosCumplidos==quantum_RR){//ES RR Y COMPLETÓ EL QUANTUM
            tripulante->status = 'R';
            tripulante->ciclosCumplidos = 0;
            log_info(logger, "El Tripulante: %d completó su quantum, se debe mover a Ready", tripulante->tid);
        }
    }
    for(int i =0; parametrosTarea[i]!=NULL; i++){
        free(parametrosTarea[i]);
    }
    free(parametrosTarea);
    for(int i =0; tarea[i]!=NULL; i++){
        free(tarea[i]);
    }
    free(tarea);
}

int llegoAPosicion(int tripulante_posX,int tripulante_posY,int posX, int posY  ){
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

int validacionPosMenor(int posTrip, int posFinal){
    return posTrip<posFinal;
}

int validacionPosMayor(int posTrip, int posFinal){
    return posTrip>posFinal;
}

void moverTripulanteUno(tcb* tcbTrip, int posXfinal, int posYfinal){
    int tamanioBufferARAM;
    int tamanioBufferAIMS;
    void* bufferARAM;
    void* bufferAIMS;
    int posXVieja;
    int posYVieja;

    t_mensaje *mensajito;

    pthread_mutex_lock(&mutexValidacionPos);
    int validacion = validacionPosMenor(tcbTrip->posicionX, posXfinal);
    pthread_mutex_unlock(&mutexValidacionPos);
    log_info(logger, "Moviendo al tripulante %d de %d|%d a %d|%d", tcbTrip->tid,tcbTrip->posicionX,tcbTrip->posicionY,posXfinal,posYfinal);
    if (validacion){
        pthread_mutex_lock(&mutexListaPCB);
        tcbTrip->posicionX++;
        pthread_mutex_unlock(&mutexListaPCB);
        //Notificar desplazamiento a RAM
        tamanioBufferARAM = sizeof(int)*4;

        bufferARAM = _serialize(tamanioBufferARAM, "%d%d%d%d", tcbTrip->pid, tcbTrip->tid, tcbTrip->posicionX, tcbTrip->posicionY);
        
        pthread_mutex_lock(&mutexBuffer);
        _send_message(conexion_RAM, "DIS", ENVIAR_UBICACION_TRIPULANTE, bufferARAM, tamanioBufferARAM, logger);
        free(bufferARAM);

        mensajito = _receive_message(conexion_RAM, logger);
        pthread_mutex_unlock(&mutexBuffer);

        if (mensajito->command != SUCCESS) {
            perror("Comando ENVIAR_UBICACION_TRIPULANTE\n");
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
    else{
        pthread_mutex_lock(&mutexValidacionPos);
        validacion = validacionPosMayor(tcbTrip->posicionX, posXfinal);
        pthread_mutex_unlock(&mutexValidacionPos);

        if (validacion){
            pthread_mutex_lock(&mutexListaPCB);
            tcbTrip->posicionX--;
            pthread_mutex_unlock(&mutexListaPCB);
        
            //Notificar desplazamiento a RAM
            tamanioBufferARAM = sizeof(int)*4;
            bufferARAM = _serialize(tamanioBufferARAM, "%d%d%d%d", tcbTrip->pid, tcbTrip->tid, tcbTrip->posicionX, tcbTrip->posicionY);
            
            pthread_mutex_lock(&mutexBuffer);
            _send_message(conexion_RAM, "DIS", ENVIAR_UBICACION_TRIPULANTE, bufferARAM, tamanioBufferARAM, logger);
            free(bufferARAM);

            mensajito = _receive_message(conexion_RAM, logger);
            pthread_mutex_unlock(&mutexBuffer);
            
            if (mensajito->command != SUCCESS) {
                perror("Comando ENVIAR_UBICACION_TRIPULANTE\n");
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
    
        else{

            pthread_mutex_lock(&mutexValidacionPos);
            validacion = validacionPosMenor(tcbTrip->posicionY, posYfinal);
            pthread_mutex_unlock(&mutexValidacionPos);
            if (tcbTrip->posicionY < posYfinal){
                pthread_mutex_lock(&mutexListaPCB);
                tcbTrip->posicionY++;
                pthread_mutex_unlock(&mutexListaPCB);
                
                //Notificar desplazamiento a RAM
                tamanioBufferARAM = sizeof(int)*4;
                bufferARAM = _serialize(tamanioBufferARAM, "%d%d%d%d", tcbTrip->pid, tcbTrip->tid, tcbTrip->posicionX, tcbTrip->posicionY);
                
                pthread_mutex_lock(&mutexBuffer);
                _send_message(conexion_RAM, "DIS", ENVIAR_UBICACION_TRIPULANTE, bufferARAM, tamanioBufferARAM, logger);
                free(bufferARAM);

                mensajito = _receive_message(conexion_RAM, logger);
                pthread_mutex_unlock(&mutexBuffer);
                if (mensajito->command != SUCCESS) {
                    perror("Comando ENVIAR_UBICACION_TRIPULANTE\n");
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
        
            else{

                pthread_mutex_lock(&mutexValidacionPos);
                validacion = validacionPosMayor(tcbTrip->posicionY, posYfinal);
                pthread_mutex_unlock(&mutexValidacionPos);
                if (tcbTrip->posicionY > posYfinal){
                    pthread_mutex_lock(&mutexListaPCB);
                    tcbTrip->posicionY--;
                    pthread_mutex_unlock(&mutexListaPCB);
                    
                    //Notificar desplazamiento a RAM
                    tamanioBufferARAM = sizeof(int)*4;
                    bufferARAM = _serialize(tamanioBufferARAM, "%d%d%d%d", tcbTrip->pid, tcbTrip->tid, tcbTrip->posicionX, tcbTrip->posicionY);
                    
                    pthread_mutex_lock(&mutexBuffer);
                    _send_message(conexion_RAM, "DIS", ENVIAR_UBICACION_TRIPULANTE, bufferARAM, tamanioBufferARAM, logger);
                    free(bufferARAM);

                    mensajito = _receive_message(conexion_RAM, logger);
                    pthread_mutex_unlock(&mutexBuffer);
                    
                    if (mensajito->command != SUCCESS) {
                        perror("Comando ENVIAR_UBICACION_TRIPULANTE\n");
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
        }
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
    void* buffer;
    
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
        //free(parametrosHilo); ???
       
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
    int validacion = contadorSemGlobal==valorMax;

    if (validacion) {
        pthread_mutex_lock(&mutex_cantidadTCB);
        cantidadTCBEnExec = queue_size(exec);
        pthread_mutex_unlock(&mutex_cantidadTCB);

        sem_post(semaforo); 

        contadorSemGlobal = 0;
    }
}




