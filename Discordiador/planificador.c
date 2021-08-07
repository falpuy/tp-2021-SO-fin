#include"headers/planificador.h"

/*------------------------FUNCIONES DE PLANIFICACION--------------------*/
void funcionPlanificador(t_log* logger) {
    cola_new = queue_create();
    ready = queue_create();
    exec = queue_create();
    bloq_io = queue_create();
    bloq_emer = queue_create();
    bloq_emer_sorted = queue_create();
    cola_exit = queue_create();
    colaContSab = queue_create();
    buffersAEnviar = queue_create();
    listaSemaforos = list_create();

    listaPCB = list_create();
}
/*---------------------------------NEW->READY--------------------------*/

void iterate_next_task(t_list *self) {
    t_link_element *element = self->head;
	t_link_element *aux = NULL;
	while (element != NULL) {
		aux = element->next;
		
        tcb *aux_TCB = (tcb *) element -> data;

        int tamanioBuffer = sizeof(int)*2;
        void* buffer = _serialize(tamanioBuffer, "%d%d", aux_TCB->pid, aux_TCB->tid);
        int conexion_RAM = _connect(ip_RAM, puerto_RAM, logger);
        _send_message(conexion_RAM, "DIS", ENVIAR_TAREA, buffer, tamanioBuffer, logger);
        free(buffer);

        t_mensaje *mensaje = _receive_message(conexion_RAM, logger);
        close(conexion_RAM);
        
        if (mensaje->command == SUCCESS) { // Recibi la primer tarea
            int tamanioTarea;
            memmove(&tamanioTarea, mensaje->payload, sizeof(int));

            aux_TCB->instruccion_actual = malloc(tamanioTarea + 1);
            memmove(aux_TCB->instruccion_actual, mensaje->payload + sizeof(int), tamanioTarea);
            aux_TCB->instruccion_actual[tamanioTarea] = '\0';

            aux_TCB -> status = 'R';
            
            log_info(logger, "Tarea Inicial: %s", aux_TCB->instruccion_actual);
        }
        else{
            log_error(logger, "No hay tareas disponibles para el tripulante %d", aux_TCB->tid);
            aux_TCB->status = 'X';
            
        }
        free(mensaje->identifier);
        free(mensaje->payload);
        free(mensaje);

		element = aux;
	}
}

void funcionhNewaReady (t_log* logger) {
    pthread_mutex_lock(&mutexValidador);
    int temp_validador = validador;
    pthread_mutex_unlock(&mutexValidador);

    while (temp_validador) {
        sem_wait(&pausar);
        sem_wait(&semNR);
        
        pthread_mutex_lock(&mutexPlanificacionViva);
        int temp_planificacion_viva = planificacion_viva;
        pthread_mutex_unlock(&mutexPlanificacionViva);

        pthread_mutex_lock(&mutexSabotajeActivado);
        int temp_sabotaje_activado = sabotaje_activado;
        pthread_mutex_unlock(&mutexSabotajeActivado);

        if(temp_planificacion_viva && temp_sabotaje_activado == 0){
            // log_info(logger, "CICLO CPU NRO: %d", contadorCicloCPU);

            while(!queue_is_empty(buffersAEnviar)){
                pthread_mutex_lock(&mutexBuffersAEnviar);
                t_mensaje* mensaje2 = (t_mensaje* ) queue_pop(buffersAEnviar);
                pthread_mutex_unlock(&mutexBuffersAEnviar);

                int conexion_RAM = _connect(ip_RAM, puerto_RAM, logger);
                _send_message(conexion_RAM, "DIS", INICIAR_PATOTA, mensaje2->payload, mensaje2->pay_len, logger);

                t_mensaje *mensaje = _receive_message(conexion_RAM, logger);
                close(conexion_RAM);

                if (mensaje->command == SUCCESS) {
                    log_info(logger,"Se guardó en Memoria OK");

                    // itero los tripulantes y le pido la tarea
                    // Command tiene el id de patota temporal
                    pcb *pcbTemp = get_pcb_by_id(listaPCB, mensaje2 -> command);
                    iterate_next_task(pcbTemp -> listaTCB);

                }
                else if (mensaje->command == ERROR_POR_FALTA_DE_MEMORIA){
                    log_error(logger,"No hay memoria para almacenar este PCB");
                }
                free(mensaje->payload);
                free(mensaje->identifier);
                free(mensaje);

                free(mensaje2->payload);
                free(mensaje2);
            }

            while(!queue_is_empty(cola_new)){   
                
                log_info(logger,"----------------------------------");
                log_info(logger, "Se ejecuta el hilo de New a Ready");

                pthread_mutex_lock(&mutexNew);
                tcb* aux_TCB = queue_pop(cola_new);
                pthread_mutex_unlock(&mutexNew);

                log_info(logger,"Tripulante: %d encontrado en New. Moviéndolo a Ready...", aux_TCB->tid);

                if (aux_TCB -> status == 'R') {
                    pthread_mutex_lock(&mutexReady);
                    queue_push(ready, (void*) aux_TCB);
                    pthread_mutex_unlock(&mutexReady);
                } else {
                    pthread_mutex_lock(&mutexExit);
                    queue_push(cola_exit, (void*) aux_TCB);
                    pthread_mutex_unlock(&mutexExit);
                }

                log_info(logger,"----------------------------------");
                log_info(logger,"Se ejecutó NEW->READY");
                log_info(logger,"----------------------------------");
            }
        }
        sem_post(&semEBIO);
        
    }
}

/*---------------------------------READY->EXEC--------------------------*/
void funcionhReadyaExec (t_log* logger){
    
    pthread_mutex_lock(&mutexValidador);
    int temp_validador = validador;
    pthread_mutex_unlock(&mutexValidador);
    
    while (temp_validador) {
        sem_wait(&semRE);


        pthread_mutex_lock(&mutexPlanificacionViva);
        int temp_planificacion_viva = planificacion_viva;
        pthread_mutex_unlock(&mutexPlanificacionViva);

        if(temp_planificacion_viva) {

            while(!queue_is_empty(ready) && queue_size(exec) < grado_multitarea){
                pthread_mutex_lock(&mutexReady);
                tcb* aux_TCB = queue_pop(ready);
                pthread_mutex_unlock(&mutexReady);

                log_info(logger,"Tripulante: %d encontrado en Ready. Moviéndolo a Exec...", aux_TCB->tid);

                pthread_mutex_lock(&mutexSabotajeActivado);
                int temp_sabotaje_activado = sabotaje_activado;
                pthread_mutex_unlock(&mutexSabotajeActivado);

                if(!temp_sabotaje_activado){
                    log_info(logger,"Instrucción Actual: %s", aux_TCB->instruccion_actual);
                }
                else{
                    log_info(logger,"Instrucción Actual: ATENDER SABOTAJE");
                }
                
                
                aux_TCB->status = 'E';
                int conexion_RAM = _connect(ip_RAM, puerto_RAM, logger);
                void *msg = _serialize(sizeof(int) * 2 + sizeof(char), "%d%d%c", aux_TCB->pid, aux_TCB->tid, aux_TCB->status);
                _send_message(conexion_RAM, "DIS", ENVIAR_CAMBIO_DE_ESTADO, msg, sizeof(int) * 2 + sizeof(char), logger);
                free(msg);
                t_mensaje *mensaje = _receive_message(conexion_RAM, logger);
                close(conexion_RAM);
                free(mensaje->identifier);
                free(mensaje->payload);
                free(mensaje);

                pthread_mutex_lock(&mutexExec);
                queue_push(exec, (void*) aux_TCB);
                pthread_mutex_unlock(&mutexExec);
            
                log_info(logger, "Se paso Tripulante a Exec");
                log_info(logger, "Hay %d nodos en Exec", queue_size(exec));

                log_info(logger,"Se ejecutó Ready->Exec");
                log_info(logger,"----------------------------------");
            }
        }

        pthread_mutex_lock(&mutex_cantidadTCB);
        cantidadTCBEnExec = queue_size(exec);
        pthread_mutex_unlock(&mutex_cantidadTCB);

        if(cantidadTCBEnExec <= 0){
            sem_post(&semBLOCKIO);
        }else{
            list_iterate(exec->elements, signalHilosTripulantes);
        }
    }
}


/*---------------------------------EXEC -> BLOCKED_IO---------------------*/
void funcionCambioExecIO(void* nodo){
    tcb* aux = (tcb *) nodo;
    int posicion;
    if(aux->status == 'I'){

        pthread_mutex_lock(&mutexExec);
        posicion = list_iterate_obtener_posicion(exec->elements, aux->tid);
  		tcb *tcbAMover = list_remove(exec->elements, posicion);
        pthread_mutex_unlock(&mutexExec);

        log_info(logger,"Tripulante: %d encontrado en Exec. Moviéndolo a BlockIO...", tcbAMover->tid);
        
        pthread_mutex_lock(&mutexBloqIO);
        queue_push(bloq_io, (void*)tcbAMover);
        pthread_mutex_unlock(&mutexBloqIO);
        log_info(logger,"Tripulante: %d movido a BlockIO.", tcbAMover->tid);
    }
}
 
void funcionhExecaBloqIO (t_log* logger){
    pthread_mutex_lock(&mutexValidador);
    int temp_validador = validador;
    pthread_mutex_unlock(&mutexValidador);
    
    while (temp_validador) {
        sem_wait(&semEBIO);

        pthread_mutex_lock(&mutexPlanificacionViva);
        int temp_planificacion_viva = planificacion_viva;
        pthread_mutex_unlock(&mutexPlanificacionViva);

        if(temp_planificacion_viva) {
            if(queue_size(exec) > 0){ 
                list_iterate(exec->elements, funcionCambioExecIO);
                log_info(logger,"Se ejecutó Exec->BlockedIO");
                log_info(logger,"----------------------------------");
            } 
        }  
        sem_post(&semRE);  
    }
}

/*---------------------------------EXEC->READY-----------------------------*/
void funcionCambioExecReady(void* nodo){
    tcb* aux = (tcb *) nodo;
    int posicion;
    if(aux->status == 'R'){
        pthread_mutex_lock(&mutexExec);
        posicion = list_iterate_obtener_posicion(exec->elements, aux->tid);
  		tcb *tcbAMover = list_remove(exec->elements, posicion);
        pthread_mutex_unlock(&mutexExec);

        log_info(logger,"Tripulante: %d encontrado en Exec. Moviéndolo a Ready...", tcbAMover->tid);

        pthread_mutex_lock(&mutexReady);
        queue_push(ready, (void*)tcbAMover);
        pthread_mutex_unlock(&mutexReady);
    }
}

void funcionhExecaReady (t_log* logger) {
    pthread_mutex_lock(&mutexValidador);
    int temp_validador = validador;
    pthread_mutex_unlock(&mutexValidador);

    while (temp_validador) {
        sem_wait(&semER);// Espera el _signal de los N tripulantes

        pthread_mutex_lock(&mutexPlanificacionViva);
        int temp_planificacion_viva = planificacion_viva;
        pthread_mutex_unlock(&mutexPlanificacionViva);

        if(temp_planificacion_viva) {
            if(!queue_is_empty(exec)){   
                log_info(logger,"----------------------------------");
                log_info(logger, "Se ejecuta el hilo de Exec a Ready");
            
                list_iterate(exec->elements, funcionCambioExecReady);
                log_info(logger,"Se ejecutó Exec->Ready");
                log_info(logger,"----------------------------------");
            }
        }
        sem_post(&semEaX);     
    }
}

/*---------------------------------EXEC-> EXIT------------------------------*/

void funcionCambioExecExit(void* nodo){
    tcb* aux = (tcb *) nodo;
    int posicion;
    if(aux->status == 'X'){
        
        pthread_mutex_lock(&mutexExec);
        posicion = list_iterate_obtener_posicion(exec->elements, aux->tid);
  		tcb *tcbAMover = list_remove(exec->elements, posicion);
        pthread_mutex_unlock(&mutexExec);

        log_info(logger,"----------------------------------");
        log_info(logger,"Tripulante: %d encontrado en Exec. Moviéndolo a Exit...", tcbAMover->tid);
        log_info(logger,"----------------------------------");

        pthread_mutex_lock(&mutexExit);
        queue_push(cola_exit, (void*)tcbAMover);
        pthread_mutex_unlock(&mutexExit);

        //pthread_mutex_lock(&mutexBuffer);
        void* buffer = _serialize(2*sizeof(int), "%d%d", tcbAMover->pid, tcbAMover->tid);
        int conexion_RAM = _connect(ip_RAM, puerto_RAM, logger);
        _send_message(conexion_RAM, "DIS", EXPULSAR_TRIPULANTE, buffer, 2*sizeof(int), logger);
        free(buffer);
        t_mensaje* mensajeRecibido = _receive_message(conexion_RAM, logger);
        close(conexion_RAM);
        //pthread_mutex_unlock(&mutexBuffer);

        if (mensajeRecibido->command != SUCCESS) {
            log_error(logger, "Memoria no expulsó al tripulante %d correctamente", tcbAMover->tid);
        }
        free(mensajeRecibido->identifier);
        free(mensajeRecibido->payload);
        free(mensajeRecibido);

        pcb *pcbEliminado = get_pcb_by_id(listaPCB, tcbAMover -> pid);

        int todosTerminaron = list_iterate_todos_terminaron(pcbEliminado->listaTCB);

        if(todosTerminaron > 0){
            
            //pthread_mutex_lock(&mutexBuffer);
            void* buffer = _serialize(sizeof(int), "%d", pcbEliminado->pid);
            int conexion_RAM = _connect(ip_RAM, puerto_RAM, logger);
            _send_message(conexion_RAM, "DIS", ELIMINAR_PATOTA, buffer, sizeof(int), logger);
            free(buffer);
            t_mensaje *mensajeRecibido = _receive_message(conexion_RAM, logger);
            close(conexion_RAM);
            //pthread_mutex_unlock(&mutexBuffer);

            if (mensajeRecibido->command == SUCCESS) {
                log_info(logger, "Se eliminaron todos los tcb de la patota: %d en RAM", pcbEliminado->pid);
                pcbEliminado->todosLosTCBsTerminaron = 1;
            }

            free(mensajeRecibido->identifier);
            free(mensajeRecibido->payload);
            free(mensajeRecibido);
        }
    }
}

void funcionhExecaExit (t_log* logger){
    pthread_mutex_lock(&mutexValidador);
    int temp_validador = validador;
    pthread_mutex_unlock(&mutexValidador);

    while (temp_validador) {
        sem_wait(&semEaX);

        pthread_mutex_lock(&mutexPlanificacionViva);
        int temp_planificacion_viva = planificacion_viva;
        pthread_mutex_unlock(&mutexPlanificacionViva);

        if (temp_planificacion_viva) {

            if(!queue_is_empty(exec)){
                list_iterate(exec->elements, funcionCambioExecExit);
                log_info(logger,"Se ejecutó Exec->Exit");
                log_info(logger,"----------------------------------");
            }
        }    
        sem_post(&semBLOCKIO);
    }
}

/*---------------------------------BlockedIO---------------------*/
void funcionContadorEnBloqIO(void* nodo){

    tcb* tcbTripulante = (tcb *) nodo;
    int tamanioTarea;
    int tamanioBuffer;
    char** tareaIO;
    char** parametrosTareaIO;

    tareaIO = string_split(tcbTripulante->instruccion_actual, " ");
    parametrosTareaIO = string_split(tareaIO[1], ";");
    int tiempoAPasarEnBloqIO = atoi(parametrosTareaIO[3]);

    if(tcbTripulante->tiempoEnBloqIO != tiempoAPasarEnBloqIO){

        tcbTripulante->tiempoEnBloqIO++;

        log_info(logger,"----------------------------------");
        log_info(logger, "Tripulante: %d tiene que seguir en BlockIO", tcbTripulante->tid);
        log_info(logger, "Va esperando: %d ciclos", tcbTripulante->tiempoEnBloqIO);
        log_info(logger, "Son en total: %d", tiempoAPasarEnBloqIO);
        log_info(logger,"----------------------------------");
    }

    if(tcbTripulante->tiempoEnBloqIO == tiempoAPasarEnBloqIO){
        
        log_info(logger,"----------------------------------");
        log_info(logger, "Tripulante:%d pasó el tiempo en bloqueado IO", tcbTripulante->tid);
        tcbTripulante = queue_pop(bloq_io);
        
        tcbTripulante->tiempoEnBloqIO = 0;
        tamanioTarea = strlen(tareaIO[0]);
        tamanioBuffer = sizeof(int)*2 + tamanioTarea;
        
        //pthread_mutex_lock(&mutexBuffer);
        void* buffer = _serialize(tamanioBuffer, "%d%s", tcbTripulante->tid, tareaIO[0]);
        int conexion_IMS = _connect(ip_IMS, puerto_IMS, logger);
        _send_message(conexion_IMS, "DIS", FINALIZAR_EJECUCION_TAREA, buffer, tamanioBuffer, logger);
        free(buffer);
        close(conexion_IMS);
        //pthread_mutex_unlock(&mutexBuffer);

        log_info(logger, "Se finalizó la tarea: %s. Tripulante: %d pide la próxima tarea",tcbTripulante->instruccion_actual, tcbTripulante->tid);
        log_info(logger,"----------------------------------");

        pedirProximaTarea(tcbTripulante);

        if (tcbTripulante -> status != 'X') {
            tcbTripulante->status = 'R';
            int conexion_RAM = _connect(ip_RAM, puerto_RAM, logger);
            void *msg = _serialize(sizeof(int) * 2 + sizeof(char), "%d%d%c", tcbTripulante->pid, tcbTripulante->tid, tcbTripulante->status);
            _send_message(conexion_RAM, "DIS", ENVIAR_CAMBIO_DE_ESTADO, msg, sizeof(int) * 2 + sizeof(char), logger);
            free(msg);
            t_mensaje *mensaje = _receive_message(conexion_RAM, logger);
            close(conexion_RAM);
            free(mensaje->identifier);
            free(mensaje->payload);
            free(mensaje);

            pthread_mutex_lock(&mutexReady);
            queue_push(ready, (void*) tcbTripulante);
            pthread_mutex_unlock(&mutexReady);
            
            log_info(logger,"----------------------------------");
            log_info(logger, "Se paso nodo tripulante: %d a Ready con nueva tarea", tcbTripulante->tid);
            log_info(logger,"----------------------------------");
        } else {
            log_info(logger,"----------------------------------");
            log_info(logger, "Se cambia estado de Tripulante: %d a Exit", tcbTripulante->tid);
            log_info(logger,"----------------------------------");
        }
    }

    free(tareaIO[0]);
    free(tareaIO[1]);
    free(tareaIO);
    free(parametrosTareaIO[0]);
    free(parametrosTareaIO[1]);
    free(parametrosTareaIO[2]);
    free(parametrosTareaIO[3]);
    free(parametrosTareaIO);
}

void funcionhBloqIO (t_log* logger){
    pthread_mutex_lock(&mutexValidador);
    int temp_validador = validador;
    pthread_mutex_unlock(&mutexValidador);
    
    while (temp_validador){
        sem_wait(&semBLOCKIO);

        pthread_mutex_lock(&mutexPlanificacionViva);
        int temp_planificacion_viva = planificacion_viva;
        pthread_mutex_unlock(&mutexPlanificacionViva);

        pthread_mutex_lock(&mutexSabotajeActivado);
        int temp_sabotaje_activado = sabotaje_activado;
        pthread_mutex_unlock(&mutexSabotajeActivado);
        
        if(temp_planificacion_viva && temp_sabotaje_activado == 0 && queue_size(bloq_io) > 0){
            pthread_mutex_lock(&mutexBloqIO);
            tcb* tcbTripulante = queue_peek(bloq_io);
            log_info(logger, "Tripulante: %d\t Patota:%d\t Posición:%d-%d\t Tarea:%s\t", tcbTripulante->tid, tcbTripulante->pid, tcbTripulante->posicionX, tcbTripulante->posicionY, tcbTripulante->instruccion_actual);
            pthread_mutex_unlock(&mutexBloqIO);

            funcionContadorEnBloqIO(tcbTripulante);

            for(int i = 1; i < queue_size(bloq_io); i++){
                pthread_mutex_lock(&mutexBloqIO);
                tcb* tcbTripulante = list_get(bloq_io->elements, i);
                log_info(logger, "El tripulante: %d está esperando su turno en la cola de bloqueados", tcbTripulante->tid);
                pthread_mutex_unlock(&mutexBloqIO);
            }
            log_info(logger,"Se ejecutó BlockedIO");
            log_info(logger,"----------------------------------");
        }
        if(sabotaje_activado){
            sem_post(&semERM);
        }else{
            sem_post(&semEXIT);
        }
    }
}


/*-------------------------------READY Y EXEC A BLOQ POR EMERGENCIA-------------------------------------*/
void funcionhExecReadyaBloqEmer (t_log* logger) {
    
    pthread_mutex_lock(&mutexValidador);
    int temp_validador = validador;
    pthread_mutex_unlock(&mutexValidador);

  	while(temp_validador) {
        sem_wait(&semERM);

        pthread_mutex_lock(&mutexSabotajeActivado);
        int temp_sabotaje_activado = sabotaje_activado;
        pthread_mutex_unlock(&mutexSabotajeActivado);

        pthread_mutex_lock(&mutexPlanificacionViva);
        int temp_planificacion_viva = planificacion_viva;
        pthread_mutex_unlock(&mutexPlanificacionViva);

        if(temp_sabotaje_activado && temp_planificacion_viva){

            tcb* tripulanteFixer;
        
            pthread_mutex_lock(&mutexCiclosTranscurridosSabotaje);
            int temp_ciclos_transcurridos_sabotaje = ciclos_transcurridos_sabotaje;
            pthread_mutex_unlock(&mutexCiclosTranscurridosSabotaje);

            if(temp_ciclos_transcurridos_sabotaje == duracion_sabotaje) // SI SE COMPLETÓ EL SABOTAJE
            {
                log_info(logger, "Se ejecuta ReadyExec->BlockedEmer");
                sabotaje_terminado=1;

                //SE SACA AL TRIPULANTE DE LA COLA DE EXEC Y SE LO COLOCA AL FINAL DE LA COLA DE BLOQ_EMER
                pthread_mutex_lock(&mutexExec);
                tripulanteFixer = queue_pop(exec);
                pthread_mutex_unlock(&mutexExec);

                if(!tripulanteFixer){ // RR, FIXER ESTABA EN READY CUANDO TERMINÓ EL SABOTAJE
                    pthread_mutex_lock(&mutexReady);
                    tripulanteFixer = queue_pop(ready);
                    pthread_mutex_unlock(&mutexReady);
                }

                primerCicloSabotaje = 0;

                tripulanteFixer->status = 'M';
                int conexion_RAM = _connect(ip_RAM, puerto_RAM, logger);
                void *msg = _serialize(sizeof(int) * 2 + sizeof(char), "%d%d%c", tripulanteFixer->pid, tripulanteFixer->tid, tripulanteFixer->status);
                _send_message(conexion_RAM, "DIS", ENVIAR_CAMBIO_DE_ESTADO, msg, sizeof(int) * 2 + sizeof(char), logger);
                free(msg);
                t_mensaje *mensaje = _receive_message(conexion_RAM, logger);
                close(conexion_RAM);
                free(mensaje->identifier);
                free(mensaje->payload);
                free(mensaje);

                log_info(logger, "El tripulante %d resolvió el sabotaje", tripulanteFixer->tid);
                
                pthread_mutex_lock(&mutexContextoSabotaje);
                tcb* tcbFixerAntesSabotaje = queue_pop(colaContSab);
                pthread_mutex_unlock(&mutexContextoSabotaje);

                tripulanteFixer->tid = tcbFixerAntesSabotaje->tid;
                tripulanteFixer->pid = tcbFixerAntesSabotaje->pid;
                tripulanteFixer->estaVivoElHilo = tcbFixerAntesSabotaje->estaVivoElHilo;
                tripulanteFixer->tiempoEnExec = tcbFixerAntesSabotaje->tiempoEnExec;
                tripulanteFixer->tiempoEnBloqIO = tcbFixerAntesSabotaje->tiempoEnBloqIO;
                tripulanteFixer->ciclosCumplidos = tcbFixerAntesSabotaje->ciclosCumplidos;

                free(tripulanteFixer->instruccion_actual);
                tripulanteFixer->instruccion_actual = malloc(strlen(tcbFixerAntesSabotaje->instruccion_actual) + 1);
                strcpy(tripulanteFixer->instruccion_actual, tcbFixerAntesSabotaje->instruccion_actual);

                free(tcbFixerAntesSabotaje->instruccion_actual);
                free(tcbFixerAntesSabotaje);

                pthread_mutex_lock(&mutexBloqEmer);
                queue_push(bloq_emer,(void*) tripulanteFixer);
                pthread_mutex_unlock(&mutexBloqEmer);
            }

            else if(primerCicloSabotaje == 0){// SI NO SE COMPLETÓ EL SABOTAJE Y ES EL PRIMER CICLO

                log_info(logger, "Se ejecuta ReadyExec->BlockedEmer");
                list_sort(exec->elements, comparadorTid);
                while (!queue_is_empty(exec))// SE PASAN LOS TRIPULANTES DE EXEC A BLOCK_EMER
                {
                    pthread_mutex_lock(&mutexExec);
                    tcb* aux_TCB = queue_pop(exec);
                    pthread_mutex_unlock(&mutexExec);

                    aux_TCB->status = 'M';
                    int conexion_RAM = _connect(ip_RAM, puerto_RAM, logger);
                    void *msg = _serialize(sizeof(int) * 2 + sizeof(char), "%d%d%c", aux_TCB->pid, aux_TCB->tid, aux_TCB->status);
                    _send_message(conexion_RAM, "DIS", ENVIAR_CAMBIO_DE_ESTADO, msg, sizeof(int) * 2 + sizeof(char), logger);
                    free(msg);
                    t_mensaje *mensaje = _receive_message(conexion_RAM, logger);
                    close(conexion_RAM);
                    free(mensaje->identifier);
                    free(mensaje->payload);
                    free(mensaje);

                    pthread_mutex_lock(&mutexBloqEmer);
                    queue_push(bloq_emer, (void*) aux_TCB);
                    pthread_mutex_unlock(&mutexBloqEmer);

                    pthread_mutex_lock(&mutexBloqEmerSorted);
                    list_add_sorted(bloq_emer_sorted->elements,(void*) aux_TCB,ordenarMasCercano);
                    pthread_mutex_unlock(&mutexBloqEmerSorted);
                }

                list_sort(ready->elements, comparadorTid);
                while (!queue_is_empty(ready))// SE PASAN LOS TRIPULANTES DE READY A BLOCK_EMER
                {
                    pthread_mutex_lock(&mutexReady);
                    tcb* aux_TCB = queue_pop(ready);
                    pthread_mutex_unlock(&mutexReady);

                    aux_TCB->status = 'M';
                    int conexion_RAM = _connect(ip_RAM, puerto_RAM, logger);
                    void *msg = _serialize(sizeof(int) * 2 + sizeof(char), "%d%d%c", aux_TCB->pid, aux_TCB->tid, aux_TCB->status);
                    _send_message(conexion_RAM, "DIS", ENVIAR_CAMBIO_DE_ESTADO, msg, sizeof(int) * 2 + sizeof(char), logger);
                    free(msg);
                    t_mensaje *mensaje = _receive_message(conexion_RAM, logger);
                    close(conexion_RAM);
                    free(mensaje->identifier);
                    free(mensaje->payload);
                    free(mensaje);

                    pthread_mutex_lock(&mutexBloqEmer);
                    queue_push(bloq_emer, (void*) aux_TCB);
                    pthread_mutex_unlock(&mutexBloqEmer);

                    pthread_mutex_lock(&mutexBloqEmerSorted);
                    list_add_sorted(bloq_emer_sorted->elements,(void*) aux_TCB,ordenarMasCercano);
                    pthread_mutex_unlock(&mutexBloqEmerSorted);
                }

                tripulanteFixer = queue_pop(bloq_emer_sorted); // SE ELIGIÓ AL TRIPULANTE QUE VA A ARREGLAR EL SABOTAJE: FIXER

                while (!queue_is_empty(bloq_emer_sorted)){// SE VACÍA LA COLA DE BLOQ_EMER_SORTED, YA NO ES NECESARIA
                    queue_pop(bloq_emer_sorted);
                }    

                tcb* tcbFixerAntesSabotaje = malloc(sizeof(tcb));

                tcbFixerAntesSabotaje->tid = tripulanteFixer->tid;
                tcbFixerAntesSabotaje->pid = tripulanteFixer->pid;
                tcbFixerAntesSabotaje->posicionX = tripulanteFixer->posicionX;
                tcbFixerAntesSabotaje->posicionY = tripulanteFixer->posicionY;
                tcbFixerAntesSabotaje->estaVivoElHilo = tripulanteFixer->estaVivoElHilo;
                tcbFixerAntesSabotaje->tiempoEnExec = tripulanteFixer->tiempoEnExec;
                tcbFixerAntesSabotaje->tiempoEnBloqIO = tripulanteFixer->tiempoEnBloqIO;
                tcbFixerAntesSabotaje->ciclosCumplidos = tripulanteFixer->ciclosCumplidos;

                tcbFixerAntesSabotaje->instruccion_actual = malloc(strlen(tripulanteFixer->instruccion_actual) + 1);
                strcpy(tcbFixerAntesSabotaje->instruccion_actual, tripulanteFixer->instruccion_actual);

                pthread_mutex_lock(&mutexContextoSabotaje);
                queue_push(colaContSab, (void*) tcbFixerAntesSabotaje);
                pthread_mutex_unlock(&mutexContextoSabotaje);

                tripulanteFixer->ciclosCumplidos=0;
                tripulanteFixer->tiempoEnExec=0;
                tripulanteFixer->tiempoEnBloqIO=0;

                //SE AVISA A IMS QUE SE ATENDERÁ EL SABOTAJE
                int idTripulante = tripulanteFixer->tid;

                //pthread_mutex_lock(&mutexBuffer);
                void* buffer = _serialize(sizeof(int), "%d", idTripulante);
                int conexion_IMS = _connect(ip_IMS, puerto_IMS, logger);
                _send_message(conexion_IMS, "DIS", ATIENDE_SABOTAJE, buffer, sizeof(int), logger);
                free(buffer);
                close(conexion_IMS);
                //pthread_mutex_unlock(&mutexBuffer);

                //SACA AL FIXER DE BLOCK_EMER Y LO COLOCA EN READY
                pthread_mutex_lock(&mutexBloqEmer);
                int posicion = list_iterate_obtener_posicion(bloq_emer->elements, idTripulante);
                tcb* aux_Fixer = list_remove(bloq_emer->elements, posicion);
                pthread_mutex_unlock(&mutexBloqEmer);

                aux_Fixer->status = 'R';
                int conexion_RAM = _connect(ip_RAM, puerto_RAM, logger);
                void *msg = _serialize(sizeof(int) * 2 + sizeof(char), "%d%d%c", aux_Fixer->pid, aux_Fixer->tid, aux_Fixer->status);
                _send_message(conexion_RAM, "DIS", ENVIAR_CAMBIO_DE_ESTADO, msg, sizeof(int) * 2 + sizeof(char), logger);
                free(msg);
                t_mensaje *mensaje = _receive_message(conexion_RAM, logger);
                close(conexion_RAM);
                free(mensaje->identifier);
                free(mensaje->payload);
                free(mensaje);

                pthread_mutex_lock(&mutexReady);
                queue_push(ready,(void*) aux_Fixer);
                pthread_mutex_unlock(&mutexReady);

                primerCicloSabotaje = 1;
            }

            log_info(logger,"Se ejecutó ReadyExec->BlockedEmer");
            log_info(logger,"----------------------------------");
        }
        sem_post(&semMR);
    }
}


/*-------------------------------BLOQ POR EMERGENCIA A READY-------------------------------------*/
void funcionhBloqEmeraReady (t_log* logger){// SE PASAN TODOS LOS TRIPULANTES QUE ESTÁN EN BLOQ_EMER, A READY
    
    pthread_mutex_lock(&mutexValidador);
    int temp_validador = validador;
    pthread_mutex_unlock(&mutexValidador);

  	while(temp_validador){
        sem_wait(&semMR);

        pthread_mutex_lock(&mutexPlanificacionViva);
        int temp_planificacion_viva = planificacion_viva;
        pthread_mutex_unlock(&mutexPlanificacionViva);

        pthread_mutex_lock(&mutexSabotajeActivado);
        int temp_sabotaje_activado = sabotaje_activado;
        pthread_mutex_unlock(&mutexSabotajeActivado);

        if (temp_planificacion_viva && temp_sabotaje_activado && sabotaje_terminado) {

            log_info(logger, "Entró a BlockedEmer->Ready");

            while (!queue_is_empty(bloq_emer))
            {
                pthread_mutex_lock(&mutexBloqEmer);
                tcb* aux_TCB = queue_pop(bloq_emer);
                pthread_mutex_unlock(&mutexBloqEmer);

                aux_TCB->status = 'R';
                int conexion_RAM = _connect(ip_RAM, puerto_RAM, logger);
                void *msg = _serialize(sizeof(int) * 2 + sizeof(char), "%d%d%c", aux_TCB->pid, aux_TCB->tid, aux_TCB->status);
                _send_message(conexion_RAM, "DIS", ENVIAR_CAMBIO_DE_ESTADO, msg, sizeof(int) * 2 + sizeof(char), logger);
                free(msg);
                t_mensaje *mensaje = _receive_message(conexion_RAM, logger);
                close(conexion_RAM);
                free(mensaje->identifier);
                free(mensaje->payload);
                free(mensaje);

                pthread_mutex_lock(&mutexReady);
                queue_push(ready, (void*) aux_TCB);
                pthread_mutex_unlock(&mutexReady);
            }
            
            pthread_mutex_lock(&mutexSabotajeActivado);
            sabotaje_activado = 0;
            pthread_mutex_unlock(&mutexSabotajeActivado);

            //SE ENVÍA A IMS QUE SE TERMINÓ EL SABOTAJE
            char* bufferAEnviar = string_new();
            string_append(&bufferAEnviar, "Se resolvio el sabotaje");
           
            //pthread_mutex_lock(&mutexBuffer);
            void* buffer = _serialize(sizeof(int) + string_length(bufferAEnviar), "%s", bufferAEnviar);
            int conexion_IMS = _connect(ip_IMS, puerto_IMS, logger);
            _send_message(conexion_IMS, "DIS", RESOLUCION_SABOTAJE, buffer, sizeof(int) + strlen(bufferAEnviar), logger);
            free(bufferAEnviar);
            free(buffer);
            close(conexion_IMS);
            //pthread_mutex_unlock(&mutexBuffer);

            //same que ready/exec a blockemer
            log_info(logger,"Se ejecutó BlockedEmer->Ready");
            log_info(logger,"----------------------------------");
        }
        sem_post(&semEXIT);
    }
}


/*-------------------------------EXIT-------------------------------------*/
void funcionhExit (t_log* logger){
    pthread_mutex_lock(&mutexValidador);
    int temp_validador = validador;
    pthread_mutex_unlock(&mutexValidador);

    while (temp_validador) {
        sem_wait(&semEXIT);

        pthread_mutex_lock(&mutexPlanificacionViva);
        int temp_planificacion_viva = planificacion_viva;
        pthread_mutex_unlock(&mutexPlanificacionViva);

        if(temp_planificacion_viva) {
            if(!queue_is_empty(cola_exit)){

                pthread_mutex_lock(&mutexValidador);
                temp_validador = validador;
                pthread_mutex_unlock(&mutexValidador); 
                if(temp_validador){

                    pthread_mutex_lock(&mutexExit);
                    list_iterate(cola_exit->elements, deletearTripulante);
                    pthread_mutex_unlock(&mutexExit);

                    if(queue_is_empty(cola_new) && queue_is_empty(ready) &&  queue_is_empty(exec) && queue_is_empty(bloq_io) && queue_is_empty(bloq_emer)){
                        pthread_mutex_lock(&mutexListaPCB);
                        list_iterate(listaPCB, eliminarPatotaEnRAM);
                        pthread_mutex_unlock(&mutexListaPCB);
                    }
                    if(!queue_is_empty(cola_new) || !queue_is_empty(ready) ||  !queue_is_empty(exec) || !queue_is_empty(bloq_io) || !queue_is_empty(bloq_emer)){
                        log_info(logger,"Se ejecutó Exit");
                        log_info(logger,"----------------------------------");
                    }
                }
            }

            pthread_mutex_lock(&mutexReady);
            list_iterate(ready->elements, terminaUnCiclo);
            pthread_mutex_unlock(&mutexReady);

            pthread_mutex_lock(&mutexExec);
            list_iterate(exec->elements, terminaUnCiclo);
            pthread_mutex_unlock(&mutexExec);

            pthread_mutex_lock(&mutexBloqIO);
            list_iterate(bloq_io->elements, terminaUnCiclo);
            pthread_mutex_unlock(&mutexBloqIO);
            
            pthread_mutex_lock(&mutexBloqEmer);
            list_iterate(bloq_emer->elements, terminaUnCiclo);
            pthread_mutex_unlock(&mutexBloqEmer);
            contadorCicloCPU++;
        }
        sem_post(&pausar);
        sem_post(&semNR);
    }    
}


/*--------------------------------ADICIONALES--------------------------------*/

void terminaUnCiclo(void* item){
    tcb* tcbAModificar = (tcb*) item;
    tcbAModificar->cicloCPUCumplido=0;
}

void eliminarPatotaEnRAM(void* item){
    pcb* pcbEliminado = (pcb*) item;
    int todosTerminaron = 0;

    if(pcbEliminado->todosLosTCBsTerminaron == 0){
        todosTerminaron = list_iterate_todos_terminaron(pcbEliminado->listaTCB);

        if(todosTerminaron > 0){
            
            //pthread_mutex_lock(&mutexBuffer);
            void* buffer = _serialize(sizeof(int), "%d", pcbEliminado->pid);
            int conexion_RAM = _connect(ip_RAM, puerto_RAM, logger);
            _send_message(conexion_RAM, "DIS", ELIMINAR_PATOTA, buffer, sizeof(int), logger);
            free(buffer);
            t_mensaje *mensajeRecibido = _receive_message(conexion_RAM, logger);
            close(conexion_RAM);
            //pthread_mutex_unlock(&mutexBuffer);

            if (mensajeRecibido->command == SUCCESS) {
                log_info(logger, "Se eliminaron todos los tcb de la patota: %d en RAM", pcbEliminado->pid);
                pcbEliminado->todosLosTCBsTerminaron = 1;
            }

            free(mensajeRecibido->identifier);
            free(mensajeRecibido->payload);
            free(mensajeRecibido);
        }
    }
}


void deletearTripulante(void* nodo){
    tcb* tcbTripulante = (tcb*) nodo;
    if(tcbTripulante->estaVivoElHilo == 1){
        log_info(logger,"Finalizando Tripulante: %d...",tcbTripulante->tid);
        tcbTripulante->estaVivoElHilo = 0;
    }
}

void signalHilosTripulantes(void *nodo) {
    tcb *tcbTripulante = (tcb *) nodo;
    sem_t* semaforo = list_get(listaSemaforos,tcbTripulante->tid);
    sem_post(semaforo);
}

int list_iterate_obtener_posicion(t_list* self, int tid) {
    int i = 0;
	t_link_element *element = self->head;
	t_link_element *aux = NULL;
	while (element != NULL) {
		aux = element->next;

        tcb* tcbLista = list_get(self, i);
        if(tid == tcbLista->tid){
            return i;
        }
        i++;

        element = aux;
	}

    return -1;
}

int list_iterate_todos_terminaron(t_list* self) {
    int i = 0;
    int cantTerminados = 0;
	t_link_element *element = self->head;
	t_link_element *aux = NULL;

	while (element != NULL) {

        tcb* tcbLista = list_get(self, i);
        if(tcbLista->status == 'X'){
            cantTerminados++;
        }

        i++;
        aux = element->next;
		element = aux;
	}

    if (cantTerminados == list_size(self)){
        return 1;
    }
    return -1;
}