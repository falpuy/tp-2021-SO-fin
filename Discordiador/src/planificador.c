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
    listaSemaforos = list_create();

    listaPCB = list_create();
}
/*---------------------------------NEW->READY--------------------------*/

void funcionhNewaReady (t_log* logger) {
    pthread_mutex_lock(&mutexValidador);
    int temp_validador = validador;
    pthread_mutex_unlock(&mutexValidador);

    while (temp_validador) {
        pthread_mutex_lock(&semaNR);
        pthread_cond_wait(&semNR, &semaNR);
        pthread_mutex_unlock(&semaNR);
        
        pthread_mutex_lock(&mutexPlanificacionViva);
        int temp_planificacion_viva = planificacion_viva;
        pthread_mutex_unlock(&mutexPlanificacionViva);

        pthread_mutex_lock(&mutexSabotajeActivado);
        int temp_sabotaje_activado = sabotaje_activado;
        pthread_mutex_unlock(&mutexSabotajeActivado);

        if(temp_planificacion_viva && temp_sabotaje_activado == 0){
            while(!queue_is_empty(cola_new)){   
                
                log_info(logger,"----------------------------------");
                log_info(logger, "Se ejecuta el hilo de New a Ready");

                pthread_mutex_lock(&mutexNew);
                tcb* aux_TCB = queue_pop(cola_new);
                pthread_mutex_unlock(&mutexNew);

                log_info(logger,"Tripulante: %d encontrado en New. Moviéndolo a Ready...", aux_TCB->tid);
                log_info(logger,"Instruccion Actual: %s", aux_TCB->instruccion_actual);

                aux_TCB->status = 'R';

                pthread_mutex_lock(&mutexReady);
                queue_push(ready, (void*) aux_TCB);
                pthread_mutex_unlock(&mutexReady);

                log_info(logger,"----------------------------------");
            }
        }

        log_info(logger,"Se ejecutó NEW->READY");
        log_info(logger,"----------------------------------");

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

                aux_TCB->status = 'E';
                pthread_mutex_lock(&mutexExec);
                queue_push(exec, (void*) aux_TCB);
                pthread_mutex_unlock(&mutexExec);
            
                log_info(logger, "Se paso Tripulante a Exec");
                log_info(logger, "Hay %d nodos en Exec", queue_size(exec));

            }

        }
        pthread_mutex_lock(&mutex_cantidadTCB);
        cantidadTCBEnExec = queue_size(exec);
        pthread_mutex_unlock(&mutex_cantidadTCB);

        log_info(logger,"Cantidad tcb en exec: %d", cantidadTCBEnExec);
        log_info(logger,"Se ejecutó Ready->Exec");
        log_info(logger,"----------------------------------");

        if(cantidadTCBEnExec <= 0){
            log_info(logger, "NO hay tripulantes");
            sem_post(&semBLOCKIO);
        }else{
            log_info(logger, "HAY tripulantes");
            //tcb* test = queue_peek(exec);
            //log_info(logger, "El tripulante que esta en exec es: %d, status: %c",test->tid,test->status);
            list_iterate(exec->elements, signalHilosTripulantes);
        }
    }
}


/*---------------------------------EXEC -> BLOCKED_IO---------------------*/
void funcionCambioExecIO(void* nodo){
    tcb* aux = (tcb *) nodo;
    if(aux->status == 'I'){
        pthread_mutex_lock(&mutexExec);
  		tcb *tcbAMover = queue_pop(exec);
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
        sem_wait(&semEBIO); //espera los N hilos de tripulantes

        pthread_mutex_lock(&mutexPlanificacionViva);
        int temp_planificacion_viva = planificacion_viva;
        pthread_mutex_unlock(&mutexPlanificacionViva);

        if(temp_planificacion_viva) {
            if(queue_size(exec) > 0){ 
                list_iterate(exec->elements, funcionCambioExecIO);
            }
        }

        log_info(logger,"Se ejecutó Exec->BlockedIO");
        log_info(logger,"----------------------------------");
        sem_post(&semER);
    }
}

/*---------------------------------BlockedIO---------------------*/
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
        }
        log_info(logger,"Se ejecutó BlockedIO");
        log_info(logger,"----------------------------------");
        sem_post(&semEXIT);
    }
}

void funcionContadorEnBloqIO(void* nodo){

    tcb* tcbTripulante = (tcb *) nodo;
    int tamanioTarea;
    int tamanioBuffer;
    void* buffer;
    char** tareaIO;
    char** parametrosTareaIO;

    tareaIO = string_split(tcbTripulante->instruccion_actual, " ");
    parametrosTareaIO = string_split(tareaIO[1], ";");
    int tiempoAPasarEnBloqIO = atoi(parametrosTareaIO[3]);

    if(tcbTripulante->tiempoEnBloqIO == tiempoAPasarEnBloqIO){
        
        log_info(logger,"----------------------------------");
        log_info(logger, "Tripulante:%d pasó el tiempo en bloqueado IO", tcbTripulante->tid);
        tcbTripulante = queue_pop(bloq_io);
        
        tcbTripulante->tiempoEnBloqIO = 0;
        tamanioTarea = strlen(tareaIO[0]);
        tamanioBuffer = sizeof(int)*2 + tamanioTarea;
        
        pthread_mutex_lock(&mutexBuffer);
        buffer = _serialize(tamanioBuffer, "%d%s", tcbTripulante->tid, tareaIO[0]);
        _send_message(conexion_IMS, "IMS", FINALIZAR_EJECUCION_TAREA, buffer, tamanioBuffer, logger);
        free(buffer);
        pthread_mutex_unlock(&mutexBuffer);

        log_info(logger, "Se finalizo la tarea:%s. Tripulante:%d pide la próxima tarea",tcbTripulante->instruccion_actual, tcbTripulante->tid);
        log_info(logger,"----------------------------------");

        pedirProximaTarea(tcbTripulante);
                
        tcbTripulante->status = 'R';

        pthread_mutex_lock(&mutexReady);
        queue_push(ready, (void*) tcbTripulante);
        pthread_mutex_unlock(&mutexReady);
        
        log_info(logger,"----------------------------------");
        log_info(logger, "Se paso nodo tripulante: %d a Ready con nueva tarea", tcbTripulante->tid);
        log_info(logger,"----------------------------------");


    }
    else{
        log_info(logger,"----------------------------------");
        log_info(logger, "Tripulante: %d tiene que seguir en BlockIO", tcbTripulante->tid);
        log_info(logger, "Va esperando: %d ciclos", tcbTripulante->tiempoEnBloqIO);
        log_info(logger, "Son en total: %d", tiempoAPasarEnBloqIO);
        log_info(logger,"----------------------------------");

        tcbTripulante->tiempoEnBloqIO++;
    }

    sleep(2); //[BORRARRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRr]
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
  		tcb *tcbAMover = list_remove(exec->elements, 0);
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
        sem_wait(&semER);

        pthread_mutex_lock(&mutexPlanificacionViva);
        int temp_planificacion_viva = planificacion_viva;
        pthread_mutex_unlock(&mutexPlanificacionViva);

        if(temp_planificacion_viva) {
            if(!queue_is_empty(exec)){   
                log_info(logger,"----------------------------------");
                log_info(logger, "Se ejecuta el hilo de Exec a Ready");
            
                list_iterate_position(exec->elements, funcionCambioExecReady);
            }
        }
        log_info(logger,"Se ejecutó Exec->Ready");
        log_info(logger,"----------------------------------");
        sem_post(&semEaX);     
    }
}

/*---------------------------------EXEC-> EXIT------------------------------*/

void funcionCambioExecExit(void* nodo, int posicion){
    tcb* aux = (tcb *) nodo;
    if(aux->status == 'X'){
        
        pthread_mutex_lock(&mutexExec);
  		tcb *tcbAMover = list_remove(exec->elements, 0);
        pthread_mutex_unlock(&mutexExec);

        log_info(logger,"----------------------------------");
        log_info(logger,"Tripulante: %d encontrado en Exec. Moviéndolo a Exit...", tcbAMover->tid);
        log_info(logger,"----------------------------------");

        pthread_mutex_lock(&mutexExit);
        queue_push(cola_exit, (void*)tcbAMover);
        pthread_mutex_unlock(&mutexExit);
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
                list_iterate_position(exec->elements, funcionCambioExecExit);
            }
        }

        log_info(logger,"Se ejecutó Exec->Exit");
        log_info(logger,"----------------------------------");
        sem_post(&semBLOCKIO);
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

                pthread_mutex_lock(&mutexExit);
                list_iterate(cola_exit->elements, deletearTripulante);
                pthread_mutex_unlock(&mutexExit);
            }
        }
        log_info(logger,"Se ejecutó Exit");
        log_info(logger,"----------------------------------");
        // sem_post(&semNR);

        pthread_cond_signal(&semNR);
    }    
}


/*--------------------------------ADICIONALES--------------------------------*/

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
    log_info(logger, "se hizo un post al tripulante: %d", tcbTripulante->tid);
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
