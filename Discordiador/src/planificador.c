#include"headers/planificador.h"



int esTareaIO(char *tarea) {
    if(!strcmp(tarea, "GENERAR_OXIGENO")) {
            return 1;
    };
    if(!strcmp(tarea, "CONSUMIR_OXIGENO")) {
            return 1;
    };
    if(!strcmp(tarea, "GENERAR_COMIDA")) {
            return 1;
    };
    if(!strcmp(tarea, "CONSUMIR_COMIDA")) {
            return 1;
    };
    if(!strcmp(tarea, "GENERAR_BASURA")) {
            return 1;
    };
    if(!strcmp(tarea, "DESCARTAR_BASURA")) {
            return 1;
    };

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
        _send_message(conexion_RAM, "RAM", RECIBIR_UBICACION_TRIPULANTE, bufferARAM, tamanioBufferARAM);
        free(bufferARAM);
        //Notificar desplazamiento a IMS
        posXVieja = tcbTrip->posicionX - 1;
        tamanioBufferAIMS = sizeof(int)*5;
        bufferAIMS = _serialize(tamanioBufferAIMS, "%d%d%d%d%d", tcbTrip->tid, posXVieja, tcbTrip->posicionY, tcbTrip->posicionX, tcbTrip->posicionY);
        _send_message(conexion_IMS, "IMS", MOVER_TRIPULANTE, bufferAIMS, tamanioBufferAIMS);
        free(bufferAIMS);
    }
    else if (tcbTrip->posicionY != posYfinal){
        tcbTrip->posicionY++;
        //Notificar desplazamiento a RAM
        tamanioBufferARAM = sizeof(int)*4;
        bufferARAM = _serialize(tamanioBufferARAM, "%d%d%d%d", tcbTrip->pid, tcbTrip->tid, tcbTrip->posicionX, tcbTrip->posicionY);
        _send_message(conexion_RAM, "RAM", RECIBIR_UBICACION_TRIPULANTE, bufferARAM, tamanioBufferARAM);
        free(bufferARAM);
        //Notificar desplazamiento a IMS
        posYVieja = tcbTrip->posicionY - 1;
        tamanioBufferAIMS = sizeof(int)*5;
        bufferAIMS = _serialize(tamanioBufferAIMS, "%d%d%d%d%d", tcbTrip->tid, tcbTrip->posicionX, posYVieja, tcbTrip->posicionX, tcbTrip->posicionY);
        _send_message(conexion_IMS, "IMS", MOVER_TRIPULANTE, bufferAIMS, tamanioBufferAIMS);
        free(bufferAIMS);
    }
    else{
        log_info(logger, "El tripulante ya llegó a la posición de la tarea");
    }
}

void pedirProximaTarea(tcb* tcbTripulante){
    int tamanioBuffer;
    int tamanioTarea;
    void* buffer;
    tamanioBuffer = sizeof(int) * 2;
    buffer = _serialize(tamanioBuffer, "%d%d", tcbTripulante->pid, tcbTripulante->tid);
    _send_message(conexion_RAM, "DIS", 521, buffer, tamanioBuffer, logger);
    free(buffer);
    t_mensaje *mensaje = _receive_message(conexion_RAM, logger);

    if (mensaje->command == SUCCESS) {
        memcpy(&tamanioTarea, mensaje->payload, sizeof(int));
        tcbTripulante->instruccion_actual = malloc(tamanioTarea + 1);
        memcpy(tcbTripulante->instruccion_actual, mensaje->payload + sizeof(int), tamanioTarea);
        tcbTripulante->instruccion_actual[tamanioTarea] = '\0';
    }
    else if (mensaje->command == ERROR_NO_HAY_TAREAS) {
        log_info(logger, "El tripulante ya realizó todas las tareas");
        tcbTripulante->status = 'X';
        break;
    }

    free(mensaje->payload);
    free(mensaje->identifier);
    free(mensaje);
}

/*ACLARACIONES DE TAREAS:
E/S: TAREA PARAMETROS;POS X;POS Y;TIEMPO
tarea[0]=TAREA
tarea[1]=PARAMETROS;POS X;POS Y;TIEMPO
parametros[0] = PARAMETROS
parametros[1] = POSX
parametros[2] = POSY
parametros[3] = TIEMPO
Normal: TAREA;POS X;POS Y;TIEMPO
tarea[0]=TAREA;POS X;POS Y;TIEMPO
parametros[0] = TAREA
parametros[1] = POSX
parametros[2] = POSY
parametros[3] = TIEMPO*/

void _signal(int incremento, int valorMax, sem_t semaforo) {
    contadorSemGlobal += incremento;
    if (contadorSemGlobal == valorMax) {
        sem_post(&semaforo);
        contadorSemGlobal = 0;
    }
}

void funcionTripulante (void* item){
    parametrosThread* aux = (parametrosThread*) item;
    int tamanioBuffer;
    int tamanioTarea;
    void* buffer;
    char** parametrosTarea;
    char** tarea;
    int tiempoEnExec=0;
    int mensajeMandado=0;
    while(validador==1){
        sem_wait(semTripulantes[aux->idSemaforo]);

        pthread_mutex_lock(&mutexExec);
        tcb *tcbTripulante = get_by_id(exec->elements, aux->idSemaforo);
        pthread_mutex_unlock(&mutexExec);

        log_info(aux->logger, "Se ejecuta hilo tripulante: %d", tcbTripulante -> tid);
        
        while(tcbTripulante->estaVivoElHilo == 1){
            if(tcbTripulante->status == 'E'){
                tarea = string_n_split(tcbTripulante->instruccion_actual, 1, " ");

                if (tarea[1]!=NULL){
                    parametrosTarea = string_split(tarea[1], ";");
                }
                else{
                    parametrosTarea = string_split(tarea[0], ";");
                    strcpy(parametrosTarea[0],itoa(-1)); //Tarea Normal
                }
                
                if(tcbTripulante->posicionX == parametrosTarea[1] && tcbTripulante->posicionY == parametrosTarea[2]){//llegó

                    if(mensajeMandado==0){
                    tamanioTarea = strlen(tarea[0]);
                    tamanioBuffer = sizeof(int)*5 + tamanioTarea + strlen(parametros[0]);
                    buffer = _serialize(tamanioBuffer, "%d%s%d%d%d%d", tcbTripulante->tid, tarea[0], atoi(parametrosTarea[0]), atoi(parametrosTarea[1]), atoi(parametrosTarea[2]), atoi(parametrosTarea[3]));
                    _send_message(conexion_IMS, "IMS", COMENZAR_EJECUCION_TAREA, buffer, tamanioBuffer, logger);
                    free(buffer);
                    mensajeMandado=1;}
                    
                    if(tarea[1]==NULL && tiempoEnExec == parametrosTarea[3]){
                        tamanioBuffer = sizeof(int)*2 + tamanioTarea;
                        buffer = _serialize(tamanioBuffer, "%d%s", tcbTripulante->tid, tarea[0]);
                        _send_message(conexion_IMS, "IMS", FINALIZAR_EJECUCION_TAREA, buffer, tamanioBuffer, logger);
                        free(buffer);
                        pedirProximaTarea(tcbTripulante);
                    }
                    
                    else if(esTareaIO(tarea[0])){
                        log_info(logger, "Se debe realizar una tarea de I/O");
                        sleep(1);
                        tcbTripulante->status = 'I';
                    }

                    else if (esTareaIO(tarea[0])==0) {
                        log_info(logger, "Se debe realizar una tarea normal (no de I/O)");
                        tiempoEnExec++;
                    }

                    else{
                        log_error(logger, "La tarea ingresada no posee un formato de tarea correcto");
                    }

                }
                else{
                    moverTripulanteUno(tcbTripulante, parametros[1], parametros[2]);
                }

                free(parametrosTarea[0]);
                free(parametrosTarea[1]);
                free(parametrosTarea[2]);
                free(parametrosTarea[3]);
                free(parametrosTarea);
                free(tarea[0]);
                free(tarea[1]);
                free(tarea);

                _signal(1, queue_size(exec), semBLOCKIO);
            }
        }
    }
}

void funcionhNewaReady (t_log* logger) {
    while (validador==1) {
        while (planificacion_pausada==0) {
            while (!queue_is_empty(cola_new))
            {   
                sem_wait(&semNR);
                log_info(logger, "Se ejecuta el hilo de New a Ready");
                tcb* aux_TCB = malloc (sizeof(tcb));
                pthread_mutex_lock(&mutexNew);
                aux_TCB = queue_pop(cola_new);
                pthread_mutex_unlock(&mutexNew);
                aux_TCB->status = 'R';
                pthread_mutex_lock(&mutexReady);
                queue_push(ready, (void*) aux_TCB);
                pthread_mutex_unlock(&mutexReady)
                sem_post(&semRE);
            }
        }
    }
}

void signalHilosTripulantes(void *nodo) {
    tcb *tcbTripulante = (tcb *) nodo;
    sem_post(semTripulantes[tcbTripulante->tid]);
}

void funcionhReadyaExec (t_log* logger){
     while (validador==1) {
        while (planificacion_pausada==0) {
            while (!queue_is_empty(ready) && queue_size(exec) < grado_multitarea)
            {
                sem_wait(&semRE);
                log_info(logger, "Se ejecuta el hilo de Ready a Exec");
                tcb* aux_TCB = malloc (sizeof(tcb));
                pthread_mutex_lock(&mutexReady);
                aux_TCB = queue_pop(ready);
                pthread_mutex_unlock(&mutexReady);
                aux_TCB->status = 'E';
                pthread_mutex_lock(&mutexExec);
                queue_push(exec, (void*) aux_TCB);
                pthread_mutex_unlock(&mutexExec);
                list_iterate(exec->elements, signalHilosTripulantes);
            }
        }
    }
}

void list_iterate_position(t_list *self, void(*closure)()) {
    int i = 0;
	t_link_element *element = self->head;
	t_link_element *aux = NULL;
	while (element != NULL) {
		aux = element->next;
		closure(element->data, i);
		element = aux;
        i++;
	}
}

void funcionCambioExecIO(void* nodo, int posicion){
    tcb* aux = (tcb *) nodo;
    if(aux->status == 'I'){
        pthread_mutex_lock(&mutexExec);
  		tcb *tcbAMover = list_remove(exec->elements, posicion);
        pthread_mutex_unlock(&mutexExec);
        pthread_mutex_lock(&mutexBloqIO);
        queue_push(bloq_io, (void*)tcbAMover);
        pthread_mutex_unlock(&mutexBloqIO);
    }
}
 
void funcionhExecaBloqIO (t_log* logger){
    while (validador==1) {
        while (planificacion_pausada==0) {
            while (!queue_is_empty(exec)){  
                sem_wait(&semBLOCKIO);
                list_iterate_position(exec->elements, funcionCambioExecIO);
                sem_post(&semEXIT);
            }
        }
    }
}

void funcionhBloqIO (t_log* logger){
    int tamanioTarea;
    int tamanioBuffer;
    void* buffer;
    char** tareaIO;
    char** parametrosTareaIO;
    while (validador){
        while(planificacion_pausada == 0) {
            sem_wait(&semBLOCKIO);
            tcb* tcbTripulante = malloc(sizeof(tcb));
            tareaIO = string_n_split(tcbTripulante->instruccion_actual, 1, " ");
            parametrosTareaIO = string_split(tareaIO[1], ";");
            tcbTripulante = queue_peek(bloq_io);
            if(tcb->tiempoEnBloqIO == parametrosTareaIO[3])
            {
                pthread_mutex_lock(&mutexBloqIO);
                tcbTripulante = queue_pop(bloq_io);
                pthread_mutex_unlock(&mutexBloqIO);
                tamanioTarea = strlen(tarea[0]);
                tamanioBuffer = sizeof(int)*2 + tamanioTarea;
                buffer = _serialize(tamanioBuffer, "%d%s", tcbTripulante->tid, tarea[0]);
                _send_message(conexion_IMS, "IMS", FINALIZAR_EJECUCION_TAREA, buffer, tamanioBuffer, logger);
                free(buffer);
                pedirProximaTarea(tcbTripulante);
                
                if(aux_TCB->status != 'X'){
                    aux_TCB->status = 'R';
                    pthread_mutex_lock(&mutexReady);
                    queue_push(ready, (void*) tcbTripulante);
                    pthread_mutex_unlock(&mutexReady);
                }
                else{
                    pthread_mutex_lock(&mutexExit);
                    queue_push(cola_exit, (void*) tcbTripulante);
                    pthread_mutex_unlock(&mutexExit);
                }
            }
            else{
                tcb->tiempoEnBloqIO++;
            }
            sem_post(&semEXIT);
        }
    }
}

void funcionCambioExecExit(void* nodo, int posicion){
    tcb* aux = (tcb *) nodo;
    if(aux->status == 'X'){
        pthread_mutex_lock(&mutexExec);
  		tcb *tcbAMover = list_remove(exec->elements, posicion);
        pthread_mutex_unlock(&mutexExec);
        pthread_mutex_lock(&mutexExit);
        queue_push(cola_exit, (void*)tcbAMover);
        pthread_mutex_unlock(&mutexExit);
    }
}

void funcionhExit (t_log* logger){
    while (validador==1) {
        while (planificacion_pausada==0) {
            while (!queue_is_empty(exec)){  
                sem_wait(&semEXIT);
                list_iterate_position(exec->elements, funcionCambioExecExit);
                sem_post(&semNR);
            }
        }
    }
}

/*
PARA SABOTAJE:

void funcionhExecaBloqEmer (t_log* logger){
  	while(validador == 1){
        while (planificacion_pausada == 0) {
            while (!queue_is_empty(exec))
            {
                tcb* aux_TCB = malloc (sizeof(tcb));
                aux_TCB = queue_peek(exec);
                queue_pop(exec);
                queue_push(bloq_emer, (void*) aux_TCB);
                aux_TCB->status = 'M';
                free(aux_TCB);
            }
        }
    }
}

void funcionhBloqEmeraReady (t_log* logger){
    while (validador){
        while(planificacion_pausada == 0) {
            while (!queue_is_empty(bloq_emer))
            {
                tcb* aux_TCB = malloc (sizeof(tcb));
                aux_TCB = queue_peek(bloq_emer);
                queue_pop(bloq_emer);
                queue_push(ready, (void*) aux_TCB);
                aux_TCB->status = 'R';
                free(aux_TCB);
            }
        }
    }
}
*/

void funcionPlanificador(t_log* logger) {
    cola_new = queue_create();
    ready = queue_create();
    exec = queue_create();
    bloq_io = queue_create();
    bloq_emer = queue_create();
    cola_exit = queue_create();

    listaPCB = list_create();
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
    if (fp == NULL)
        exit(EXIT_FAILURE);

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

tcb* crear_TCB(int idP, int posX, int posY, int idT, t_log* logger)
{
    tcb* nuevoTCB = malloc (sizeof(tcb));
	nuevoTCB->tid = idT;
    nuevoTCB->pid = idP;
    nuevoTCB->status = 'N';
    nuevoTCB->posicionX = posX;
    nuevoTCB->posicionY = posY;
    nuevoTCB->instruccion_actual = NULL;
    nuevoTCB->tiempoEnExec = 0;
    nuevoTCB->tiempoEnBloqIO = 0;

    return nuevoTCB;
}

pcb* crear_PCB(char** parametros, int conexion_RAM, t_log* logger)
{     
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
  	
  	free(temp);
  
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
    _send_message(conexion_RAM, "DIS", INICIAR_PATOTA, buffer_a_enviar, tamanioBuffer, logger);
  	t_mensaje *mensaje = _receive_message(conexion_RAM, logger);

  	if (mensaje->command == SUCCESS) {
      return nuevoPCB;
    }
  
  	return NULL;
}