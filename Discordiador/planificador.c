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
        sem_wait(&semNR);
        
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
                log_info(logger,"Se ejecutó NEW->READY");
                log_info(logger,"----------------------------------");
            }
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

        // log_info(logger,"Cantidad tcb en exec: %d", cantidadTCBEnExec);

        if(cantidadTCBEnExec <= 0){
            // log_info(logger, "NO hay tripulantes");
            sem_post(&semBLOCKIO);
        }else{
            // log_info(logger, "HAY tripulantes");
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
        sem_wait(&semEBIO); //espera los N hilos de tripulantes

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

        sem_post(&semER);
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
        sem_wait(&semER);

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

        pthread_mutex_lock(&mutexBuffer);
        buffer = _serialize(2*sizeof(int), "%d%d", tcbAMover->pid, tcbAMover->tid);
        _send_message(conexion_RAM, "DIS", EXPULSAR_TRIPULANTE, buffer, 2*sizeof(int), logger);
        free(buffer);
        t_mensaje* mensajeRecibido = _receive_message(conexion_RAM, logger);
        pthread_mutex_unlock(&mutexBuffer);

        if (mensajeRecibido->command != SUCCESS) {
            log_error(logger, "Memoria no expulsó al tripulante %d correctamente", tcbAMover->tid);
        }
        free(mensajeRecibido->identifier);
        free(mensajeRecibido->payload);
        free(mensajeRecibido);
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
        _send_message(conexion_IMS, "DIS", FINALIZAR_EJECUCION_TAREA, buffer, tamanioBuffer, logger);
        free(buffer);
        pthread_mutex_unlock(&mutexBuffer);

        log_info(logger, "Se finalizo la tarea:%s. Tripulante:%d pide la próxima tarea",tcbTripulante->instruccion_actual, tcbTripulante->tid);
        log_info(logger,"----------------------------------");

        pedirProximaTarea(tcbTripulante);

        if (tcbTripulante -> status != 'X') {
            tcbTripulante->status = 'R';

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
    else{
        log_info(logger,"----------------------------------");
        log_info(logger, "Tripulante: %d tiene que seguir en BlockIO", tcbTripulante->tid);
        log_info(logger, "Va esperando: %d ciclos", tcbTripulante->tiempoEnBloqIO);
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
            // log_info(logger, "El sabotaje está activado");
            sem_post(&semERM);
        }else{
            // log_info(logger, "El sabotaje está desactivado");
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
        tcb* tcbFixerAntesSabotaje = malloc(sizeof(tcb));

        log_info(logger, "Entró a ReadyExec->BlockedEmer");

        pthread_mutex_lock(&mutexSabotajeActivado);
        int temp_sabotaje_activado = sabotaje_activado;
        pthread_mutex_unlock(&mutexSabotajeActivado);

        pthread_mutex_lock(&mutexPlanificacionViva);
        int temp_planificacion_viva = planificacion_viva;
        pthread_mutex_unlock(&mutexPlanificacionViva);

        if(temp_sabotaje_activado && temp_planificacion_viva){
        
            pthread_mutex_lock(&mutexCiclosTranscurridosSabotaje);
            int temp_ciclos_transcurridos_sabotaje = ciclos_transcurridos_sabotaje;
            pthread_mutex_unlock(&mutexCiclosTranscurridosSabotaje);

            if(temp_ciclos_transcurridos_sabotaje == duracion_sabotaje) // SI SE COMPLETÓ EL SABOTAJE
            {
                sabotaje_terminado=1;

                //SE SACA AL TRIPULANTE DE LA COLA DE EXEC Y SE LO COLOCA AL FINAL DE LA COLA DE BLOQ_EMER
                pthread_mutex_lock(&mutexExec);
                tcb* tripulanteFixer = queue_pop(exec);
                pthread_mutex_unlock(&mutexExec);

                tripulanteFixer->status = 'M';
                
                tripulanteFixer->tid = tcbFixerAntesSabotaje->tid;
                tripulanteFixer->pid = tcbFixerAntesSabotaje->pid;
                tripulanteFixer->posicionX = tcbFixerAntesSabotaje->posicionX;
                tripulanteFixer->posicionY = tcbFixerAntesSabotaje->posicionY;
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

            else{ // SI NO SE COMPLETÓ EL SABOTAJE

                list_sort(exec->elements, comparadorTid);
                while (!queue_is_empty(exec))// SE PASAN LOS TRIPULANTES DE EXEC A BLOCK_EMER
                {
                    pthread_mutex_lock(&mutexExec);
                    tcb* aux_TCB = queue_pop(exec);
                    pthread_mutex_unlock(&mutexExec);

                    aux_TCB->status = 'M';

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

                tcbFixerAntesSabotaje->tid = tripulanteFixer->tid;
                tcbFixerAntesSabotaje->pid = tripulanteFixer->pid;
                tcbFixerAntesSabotaje->posicionX = tripulanteFixer->posicionX;
                tcbFixerAntesSabotaje->posicionY = tripulanteFixer->posicionY;
                tcbFixerAntesSabotaje->estaVivoElHilo = tripulanteFixer->estaVivoElHilo;
                tcbFixerAntesSabotaje->tiempoEnExec = tripulanteFixer->tiempoEnExec;
                tcbFixerAntesSabotaje->tiempoEnBloqIO = tripulanteFixer->tiempoEnBloqIO;
                tcbFixerAntesSabotaje->ciclosCumplidos = tripulanteFixer->ciclosCumplidos;
                log_info(logger, "Intrucción actual del tripulante fixer: %s", tripulanteFixer->instruccion_actual);
                tcbFixerAntesSabotaje->instruccion_actual = malloc(strlen(tripulanteFixer->instruccion_actual) + 1);
                strcpy(tcbFixerAntesSabotaje->instruccion_actual, tripulanteFixer->instruccion_actual);

                tripulanteFixer->ciclosCumplidos=0;
                tripulanteFixer->tiempoEnExec=0;
                tripulanteFixer->tiempoEnBloqIO=0;

                //SE AVISA A IMS QUE SE ATENDERÁ EL SABOTAJE
                int idTripulante = tripulanteFixer->tid;

                pthread_mutex_lock(&mutexBuffer);
                buffer = _serialize(sizeof(int), "%d", idTripulante);
                _send_message(conexion_IMS, "DIS", ATIENDE_SABOTAJE, buffer, sizeof(int), logger);
                free(buffer);
                pthread_mutex_unlock(&mutexBuffer);

                //SACA AL FIXER DE BLOCK_EMER Y LO COLOCA EN READY

                pthread_mutex_lock(&mutexBloqEmer);
                int posicion = list_iterate_obtener_posicion(bloq_emer->elements, idTripulante);
                tcb* aux_Fixer = list_remove(bloq_emer->elements, posicion);
                pthread_mutex_unlock(&mutexBloqEmer);

                aux_Fixer->status = 'R';

                pthread_mutex_lock(&mutexReady);
                queue_push(ready,(void*) aux_Fixer);
                pthread_mutex_unlock(&mutexReady);
            }
            //checkear si me quieren meter sabotaje despues de que todos los tripulantes terminen
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

                pthread_mutex_lock(&mutexReady);
                queue_push(ready, (void*) aux_TCB);
                pthread_mutex_unlock(&mutexReady);
            }
            
            pthread_mutex_lock(&mutexSabotajeActivado);
            sabotaje_activado = 0;
            pthread_mutex_lock(&mutexSabotajeActivado);

            //SE ENVÍA A IMS QUE SE TERMINÓ EL SABOTAJE
            char* bufferAEnviar = string_new();
            string_append(&bufferAEnviar, "Se resolvio el sabotaje");
           
            pthread_mutex_lock(&mutexBuffer);
            buffer = _serialize(sizeof(int) + string_length(bufferAEnviar), "%s", bufferAEnviar);
            _send_message(conexion_IMS, "DIS", RESOLUCION_SABOTAJE, buffer, sizeof(int) + strlen(bufferAEnviar), logger);
            free(bufferAEnviar);
            free(buffer);
            pthread_mutex_unlock(&mutexBuffer); 
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

                    pthread_mutex_lock(&mutexListaPCB);
                    list_iterate(listaPCB, eliminarPatotaEnRAM);
                    pthread_mutex_unlock(&mutexListaPCB);
                    if(!queue_is_empty(cola_new) || !queue_is_empty(ready) ||  !queue_is_empty(exec) || !queue_is_empty(bloq_io) || !queue_is_empty(bloq_emer)){
                        log_info(logger,"Se ejecutó Exit");
                        log_info(logger,"----------------------------------");
                    }
                }
            }
        }
        
        sem_post(&semNR);
    }    
}


/*--------------------------------ADICIONALES--------------------------------*/

void eliminarPatotaEnRAM(void* item){
    pcb* pcbEliminado = (pcb*) item;
    int todosTerminaron = 0;

    if(pcbEliminado->todosLosTCBsTerminaron == 0){
        todosTerminaron = list_iterate_todos_terminaron(pcbEliminado->listaTCB);

        if(todosTerminaron > 0){
            
            pthread_mutex_lock(&mutexBuffer);
            buffer = _serialize(sizeof(int), "%d", pcbEliminado->pid);
            _send_message(conexion_RAM, "DIS", ELIMINAR_PATOTA, buffer, sizeof(int), logger);
            free(buffer);
            t_mensaje *mensajeRecibido = _receive_message(conexion_RAM, logger);
            pthread_mutex_unlock(&mutexBuffer);

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
    //log_info(logger, "se hizo un post al tripulante: %d", tcbTripulante->tid);
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
        //tcb* tcbLista = (tcb*) element->data;

        tcb* tcbLista = list_get(self, i);
        if(tcbLista->status == 'X'){
            cantTerminados++;
        }

        i++;
        aux = element->next;
		element = aux;
	}

    log_info(logger, "cantTerminados = %d", cantTerminados);
    log_info(logger, "cantidad de TCB en la patota = %d", list_size(self));

    if (cantTerminados == list_size(self)){
        return 1;
    }
    return -1;
}