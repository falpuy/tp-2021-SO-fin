#include"headers/planificador.h"

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



/*------------------------FUNCIONES DE PLANIFICACION--------------------*/
void funcionPlanificador(t_log* logger) {
    cola_new = queue_create();
    ready = queue_create();
    exec = queue_create();
    bloq_io = queue_create();
    bloq_emer = queue_create();
    bloq_emer_sorted = queue_create();
    cola_exit = queue_create();

    listaPCB = list_create();
}

/*---------------------------------NEW->READY--------------------------*/

void funcionhNewaReady (t_log* logger) {
    pthread_mutex_lock(&mutexValidador);
    int temp_validador = validador;
    pthread_mutex_unlock(&mutexValidador);

    while (temp_validador) {
        sem_wait(&semNR);
        
        if(planificacion_viva && sabotaje_activado == 0){
            while(!queue_is_empty(cola_new)){   
                
                log_info(logger,"----------------------------------");
                log_info(logger, "Se ejecuta el hilo de New a Ready");
                //tcb* aux_TCB = malloc (sizeof(tcb));

                pthread_mutex_lock(&mutexNew);
                tcb* aux_TCB = queue_pop(cola_new);
                pthread_mutex_unlock(&mutexNew);

                log_info(logger,"Tripulante encontrado. Moviendolo a Ready...");
                log_info(logger,"TID:%d", aux_TCB->tid);
                log_info(logger,"Instruccion Actual:%s", aux_TCB->instruccion_actual);

                aux_TCB->status = 'R';
                pthread_mutex_lock(&mutexReady);
                queue_push(ready, (void*) aux_TCB);
                pthread_mutex_unlock(&mutexReady);

                log_info(logger,"----------------------------------");
            }

            log_info(logger,"Se hizo una ejecución de CPU en NEW->READY");
            log_info(logger,"----------------------------------");
        }

        sem_post(&semRE);
    }
}

/*---------------------------------READY->EXEC--------------------------*/
void funcionhReadyaExec (t_log* logger){
    
    pthread_mutex_lock(&mutexValidador);
    int temp_validador = validador;
    pthread_mutex_unlock(&mutexValidador);
    

    while (temp_validador) {
        sem_wait(&semRE);
        if(planificacion_viva) {
            while(!queue_is_empty(ready) && queue_size(exec) <= grado_multitarea){
                log_info(logger,"----------------------------------");
                log_info(logger, "Se ejecuta el hilo de Ready a Exec");
                //  aux_TCB = malloc (sizeof(tcb));

                pthread_mutex_lock(&mutexReady);
                tcb* aux_TCB = queue_pop(ready);
                pthread_mutex_unlock(&mutexReady);

                log_info(logger,"Tripulante encontrado. Moviendolo a Exec...");
                log_info(logger,"TID:%d", aux_TCB->tid);
                if(!sabotaje_activado)
                    log_info(logger,"Instruccion Actual:%s", aux_TCB->instruccion_actual);

                aux_TCB->status = 'E';
                pthread_mutex_lock(&mutexExec);
                queue_push(exec, (void*) aux_TCB);
                pthread_mutex_unlock(&mutexExec);
                
                pthread_mutex_lock(&mutex_cantidadTCB);
                cantidadTCBEnExec--;
                pthread_mutex_unlock(&mutex_cantidadTCB);

                log_info(logger, "Se paso Tripulante a Exec");
                log_info(logger, "Hay %d nodos en Exec", queue_size(exec));

            }

            if (queue_is_empty(exec) && sabotaje_activado == 0) {
                sem_post(&semBLOCKIO);
            } else {
                log_info(logger, "Se comienza a iterar signals...");
                cantidadTCBEnExec = queue_size(exec);
                list_iterate(exec->elements, signalHilosTripulantes);
            }
            
            log_info(logger,"Se hizo una ejecución de CPU en Ready->Exec");
            log_info(logger,"----------------------------------");
        }
    }
}


/*---------------------------------EXEC -> BLOCKED_IO---------------------*/
void funcionCambioExecIO(void* nodo, int posicion){
    tcb* aux = (tcb *) nodo;
    if(aux->status == 'I'){
        
        pthread_mutex_lock(&mutexExec);
  		tcb *tcbAMover = list_remove(exec->elements, posicion);
        pthread_mutex_unlock(&mutexExec);

        log_info(logger,"[CambioExecIO] Tripulante encontrado. Moviendolo a Blocked IO...");
        log_info(logger,"TID: %d", tcbAMover->tid);
        log_info(logger,"Instruccion Actual: %s", tcbAMover->instruccion_actual);
        
        pthread_mutex_lock(&mutexBloqIO);
        queue_push(bloq_io, (void*)tcbAMover);
        pthread_mutex_unlock(&mutexBloqIO);

        log_info(logger, "Se paso nodo a Exec");
    }
}
 
void funcionhExecaBloqIO (t_log* logger){
    pthread_mutex_lock(&mutexValidador);
    int temp_validador = validador;
    pthread_mutex_unlock(&mutexValidador);
    
    while (temp_validador) {
        sem_wait(&semEBIO);
        if(planificacion_viva) {
            if (!queue_is_empty(exec)){ 
                log_info(logger,"----------------------------------");
                log_info(logger, "Se ejecuta el hilo de Exec a BlockedIO");
                list_iterate_position(exec->elements, funcionCambioExecIO);
                // log_info(logger, "[Exec a Bloqio] ejecuto _SIGNAL desde semEBIO...");
                _signal(1, cantidadTCBEnExec, &semBLOCKIO);
            }
            
            log_info(logger,"Se hizo una ejecución de CPU en Exec->BlockedIO");
            log_info(logger,"----------------------------------");
        }
    }
}

/*---------------------------------BlockedIO---------------------*/
void funcionhBloqIO (t_log* logger){
    pthread_mutex_lock(&mutexValidador);
    int temp_validador = validador;
    pthread_mutex_unlock(&mutexValidador);

    
    while (temp_validador){
        sem_wait(&semBLOCKIO);
        
        if(planificacion_viva && sabotaje_activado == 0){
            log_info(logger,"----------------------------------");
            log_info(logger, "Se ejecuta el hilo de Exec a BlockedIO");
            list_iterate_position(bloq_io->elements, funcionContadorEnBloqIO);
            log_info(logger,"Se hizo una ejecución de CPU en BlockedIO");
            log_info(logger,"----------------------------------");
        }
        sem_post(&semEXIT);
    }
}

void funcionContadorEnBloqIO(void* nodo, int posicion){

    tcb* tcbTripulante = (tcb *) nodo;
    int tamanioTarea;
    int tamanioBuffer;
    void* buffer;
    char** tareaIO;
    char** parametrosTareaIO;

    // log_info(logger, "Iterando la funcion de contador en Cola BlockedIO...");

    tareaIO = string_split(tcbTripulante->instruccion_actual, " ");
    parametrosTareaIO = string_split(tareaIO[1], ";");
    int tiempoAPasarEnBloqIO = atoi(parametrosTareaIO[3]);

    if(tcbTripulante->tiempoEnBloqIO == tiempoAPasarEnBloqIO){
        
        log_info(logger,"----------------------------------");
        log_info(logger, "Tripulante:%d pasó el tiempo en bloqueado IO", tcbTripulante->tid);
        pthread_mutex_lock(&mutexBloqIO);
        tcbTripulante = list_remove(bloq_io->elements, posicion);
        pthread_mutex_unlock(&mutexBloqIO);
        
        tcbTripulante->tiempoEnBloqIO = 0;
        tamanioTarea = strlen(tareaIO[0]);
        tamanioBuffer = sizeof(int)*2 + tamanioTarea;
        
        buffer = _serialize(tamanioBuffer, "%d%s", tcbTripulante->tid, tareaIO[0]);
        _send_message(conexion_IMS, "IMS", FINALIZAR_EJECUCION_TAREA, buffer, tamanioBuffer, logger);
        free(buffer);

        log_info(logger,"----------------------------------");
        log_info(logger, "[IMPORTANTE] Se finalizo la tarea. Tripulante:%d pide la próxima tarea", tcbTripulante->tid);
        log_info(logger,"----------------------------------");

        pedirProximaTarea(tcbTripulante);
                
        tcbTripulante->status = 'R';
        pthread_mutex_lock(&mutexReady);
        queue_push(ready, (void*) tcbTripulante);
        pthread_mutex_unlock(&mutexReady);
        
        log_info(logger,"----------------------------------");
        log_info(logger, "Se paso nodo tripulante:%d a Ready con nueva tarea", tcbTripulante->tid);
        log_info(logger,"----------------------------------");


    }
    else{
        log_info(logger,"----------------------------------");
        log_info(logger, "Tripulante:%d tiene que seguir en bloqueadoIO", tcbTripulante->tid);
        log_info(logger, "Va esperando:%d ciclos", tcbTripulante->tiempoEnBloqIO);
        log_info(logger, "Son en total: %d", tiempoAPasarEnBloqIO);
        log_info(logger,"----------------------------------");

        tcbTripulante->tiempoEnBloqIO++;
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
/*---------------------------------EXEC->READY-----------------------------*/
void funcionCambioExecReady(void* nodo, int posicion){
    tcb* aux = (tcb *) nodo;
    if(aux->status == 'R'){
        pthread_mutex_lock(&mutexExec);
  		tcb *tcbAMover = list_remove(exec->elements, posicion);
        pthread_mutex_unlock(&mutexExec);

        log_info(logger,"Tripulante encontrado. Moviendolo a Ready...");
        log_info(logger,"TID:%d", tcbAMover->tid);
        if(!sabotaje_activado)
            log_info(logger,"Instruccion Actual: %s", tcbAMover->instruccion_actual);
        else
            log_info(logger,"Instruccion Actual: ATENDIENDO SABOTAJE");
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
        sem_wait(&semER);
        if(planificacion_viva) {
            while (!queue_is_empty(exec))
            {   
                log_info(logger,"----------------------------------");
                log_info(logger, "Se ejecuta el hilo de Exec a Ready");

                
                list_iterate_position(exec->elements, funcionCambioExecReady);
                log_info(logger, "[Exec a Rdy] ejecuto _SIGNAL desde semER..");
                if(!sabotaje_activado)
                    _signal(1, cantidadTCBEnExec, &semBLOCKIO);
                else
                    sem_post(&semRE);
            }

            log_info(logger,"Se hizo una ejecución de CPU en Exec->Ready");
            log_info(logger,"----------------------------------");
        }
    }
}

/*---------------------------------EXEC-> EXIT------------------------------*/

void funcionCambioExecExit(void* nodo, int posicion){
    tcb* aux = (tcb *) nodo;
    if(aux->status == 'X'){
        pthread_mutex_lock(&mutexExec);
  		tcb *tcbAMover = list_remove(exec->elements, posicion);
        pthread_mutex_unlock(&mutexExec);

        log_info(logger,"----------------------------------");

        log_info(logger,"[CambioExecExit]Tripulante encontrado. Moviendolo a Exit...");
        log_info(logger,"TID:%d", tcbAMover->tid);
        log_info(logger,"Status:%c",tcbAMover->status);
        log_info(logger,"Instruccion Actual:%s", tcbAMover->instruccion_actual);
        log_info(logger,"----------------------------------");


        pthread_mutex_lock(&mutexExit);
        queue_push(cola_exit, (void*)tcbAMover);
        pthread_mutex_unlock(&mutexExit);
        
        pthread_mutex_lock(&mutex_cantidadTCB);
        cantidadTCBEnExec--;
        pthread_mutex_unlock(&mutex_cantidadTCB);
    }
}

void funcionhExecaExit (t_log* logger){
    pthread_mutex_lock(&mutexValidador);
    int temp_validador = validador;
    pthread_mutex_unlock(&mutexValidador);

    
    while (temp_validador) {
        sem_wait(&semEaX);

        if (planificacion_viva) {
            while (!queue_is_empty(exec)){  
                log_info(logger,"----------------------------------");
                log_info(logger, "Se ejecuta el hilo de Exec a Exit");

                list_iterate_position(exec->elements, funcionCambioExecExit);
                _signal(1, cantidadTCBEnExec, &semBLOCKIO);
            }

            log_info(logger,"Se hizo una ejecución de CPU en Exec->Exit");
            log_info(logger,"----------------------------------");
        }
    }
}
/*-------------------------------EXIT-------------------------------------*/
void funcionhExit (t_log* logger){
    pthread_mutex_lock(&mutexValidador);
    int temp_validador = validador;
    pthread_mutex_unlock(&mutexValidador);


    while (temp_validador) {
        sem_wait(&semEXIT);
        if(planificacion_viva) {
            if (!queue_is_empty(cola_exit)){  
                log_info(logger,"----------------------------------");
                log_info(logger, "Se ejecuta el hilo de Exit");

                pthread_mutex_lock(&mutexExit);
                tcb* tcbAEliminar = queue_pop(cola_exit);
                pthread_mutex_unlock(&mutexExit);
                
                if(tcbAEliminar){
                    log_info(logger,"Tripulante encontrado...");
                    log_info(logger,"TID:%d", tcbAEliminar->tid);
                    log_info(logger,"Status:%c",tcbAEliminar->status);
                    log_info(logger,"Instruccion Actual:%s", tcbAEliminar->instruccion_actual);
                    
                    tcbAEliminar->estaVivoElHilo = 0;
                    free(tcbAEliminar->instruccion_actual);
                    free(tcbAEliminar);
                    log_info(logger,"Se elimino Tripulante...");

                }else{
                    log_info(logger, "No existe un tripulante a eliminar");
                }
            }

            log_info(logger,"Se hizo una ejecución de CPU en Exit");
            log_info(logger,"----------------------------------");
        }
        sleep(1);
        sem_post(&semNR);
    }
}


/*--------------------------------ADICIONALES--------------------------------*/

void signalHilosTripulantes(void *nodo) {
    tcb *tcbTripulante = (tcb *) nodo;
    sem_post(&semTripulantes[tcbTripulante->tid]);
}

void list_iterate_position(t_list *self, void(*closure)()){
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
