#include"headers/planificador.h"

/*
void bloqueado (void* nodo){
		tcb* tcb = (tcb) nodo;
    queue_push(cola_io, tcb);
}

funcion execBlocked{
	while(estoyVivo){
			wait(ultimoTripulante)
    		
      signal(bloqueado)
    }

}
funcionTCB{
    while(estoyVivo){
    	
        if(estado == E){

            switch que tarea soy?

                dentro de cada tarea-->
                moverse a X 
                si esta en x
                    moverse a Y
                        si esta en Y -->Llego a ubicacion

                Si I/O 
                    - mande msg IMS
                    - estado = B
                    - contadorExec == tiempoTarea?
                    
                Si no es IO--> aumentar contadorExec en 1
                    - si contadorExec == tiempoTarea     

                si termine tarea{
                    pido tarea nueva a memoria
                        si no hay mas tareas
                            estado = X
            }
        }
    }   

}
*/

enum tipo_tarea {
  GENERAR_OXIGENO,
  CONSUMIR_OXIGENO,
  GENERAR_COMIDA,
  CONSUMIR_COMIDA,
  GENERAR_BASURA,
  DESCARTAR_BASURA,
  OTRA
};

int obtener_tipo_tarea(char *tarea) {
    if(!strcmp(tarea, "GENERAR_OXIGENO")) {
            return GENERAR_OXIGENO;
    };
    if(!strcmp(tarea, "CONSUMIR_OXIGENO")) {
            return CONSUMIR_OXIGENO;
    };
    if(!strcmp(tarea, "GENERAR_COMIDA")) {
            return GENERAR_COMIDA;
    };
    if(!strcmp(tarea, "CONSUMIR_COMIDA")) {
            return CONSUMIR_COMIDA;
    };
    if(!strcmp(tarea, "GENERAR_BASURA")) {
            return GENERAR_BASURA;
    };
    if(!strcmp(tarea, "DESCARTAR_BASURA")) {
            return DESCARTAR_BASURA;
    };

    for(int i = 0; i < strlen(tarea); i++) {
        if (tarea[i] == ';') {
            return OTRA;
        }
    };
    return -1;
}

/*void moverTripulanteA(tcb* tcbTripulante, int posXfinal, int posYfinal){
    int tamanioBuffer;
    void* buffer;
    while (tcbTripulante->posicionX != posXfinal){
        while(tcbTripulante->posicionY != posYfinal){
            tcbTripulante->posicionY += 1;
            tamanioBuffer = ;
            buffer = malloc(tamanioBuffer);
            buffer = _serialize(tamanioBuffer, "", );
            _send_message(conexion_IMS, "IMS", , buffer, tamanioBuffer);
            _send_message(conexion_RAM, "IMS", , buffer, tamanioBuffer);
            free(buffer);
        }
        tcbTripulante->posicionX += 1;
        tamanioBuffer = ;
        buffer = malloc(tamanioBuffer);
        buffer = _serialize(tamanioBuffer, "", );
        _send_message(conexion_IMS, "IMS", , buffer, tamanioBuffer);
        _send_message(conexion_RAM, "IMS", , buffer, tamanioBuffer);
        free(buffer);
    }
}*/

void funcionTripulante (tcb* tcbTripulante){
    //!
    while(tcbTripulante->estaVivoElHilo == 1){
        while(tcbTripulante->status != 'X'){
            while(tcbTripulante->status == 'E'){
                int numeroTarea = 6;
                int i;
                int tamanioBuffer;
                int tamanioTarea;
                void* buffer;
                //E/S: TAREA PARAMETROS;POS X;POS Y;TIEMPO
                //Otra: TAREA;POS X;POS Y;TIEMPO
                char** tarea = string_n_split(tcbTripulante->instruccion_actual, 1, " ");
                if (tarea[1] != NULL){
                    char** parametrosIO = string_split(tarea[1], ";");
                    /*parametrosIO[0] = PARAMETROS
                    parametrosIO[1] = POSX
                    parametrosIO[2] = POSY
                    parametrosIO[3] = TIEMPO*/
                }

                switch(obtener_tipo_tarea(tarea[0])) {
                    case GENERAR_OXIGENO:
                        log_info(logger, "ENTRO GENERAR OXIGENO");
                        sleep(1);
                        //moverTripulanteA(tcbTripulante, parametros[1], parametros[2]);
                        //empiezo la tarea:
                        tamanioTarea = strlen(tarea[0]);
                        tamanioBuffer = sizeof(int)*5 + tamanioTarea + strlen(parametros[0]);
                        buffer = _serialize(tamanioBuffer, "%d%s%s%d%d%d", tcbTripulante->tid, tarea[0], parametros[0], parametros[1], parametros[2], parametros[3]);
                        _send_message(conexion_IMS, "IMS", 762, buffer, tamanioBuffer, logger);
                        free(buffer);
                        //enviar a bloqueado durante parametrosIO[3]
                        //termino la tarea:
                        /*wait();
                        tamanioBuffer = sizeof(int)*5 + tamanioTarea + strlen(parametros[0]);;
                        buffer = _serialize(tamanioBuffer, , );
                        _send_message(conexion_IMS, "IMS", 763, buffer, tamanioBuffer, logger);
                        free(buffer);*/
                        //SERÍA IGUAL EN TODAS LAS TAREAS DE ENTRADA/SALIDA?
                        break;

                    case CONSUMIR_OXIGENO:
                        log_info(logger, "ENTRO CONSUMIR OXIGENO");
                        break;

                    case GENERAR_COMIDA:
                        log_info(logger, "ENTRO GENERAR COMIDA");
                        break;

                    case CONSUMIR_COMIDA:
                        log_info(logger, "ENTRO CONSUMIR COMIDA");
                        break;

                    case GENERAR_BASURA:
                        log_info(logger, "ENTRO GENERAR BASURA");
                        break;

                    case DESCARTAR_BASURA:
                        log_info(logger, "ENTRO DESCARTAR BASURA");
                        break;
                        
                    case OTRA:
                        log_info(logger, "ENTRO OTRA");
                        break;

                    default:
                        log_info(logger, "La tarea ingresada no posee un formato de tarea correcto");
                        break;
                }

                //pedir proxima tarea
                /*tamanioBuffer = sizeof(int) * 2;
                buffer = _serialize(tamanioBuffer, "%d%d", tcbTripulante->pid, tcbTripulante->tid);
                _send_message(conexion_RAM, "DIS", 520, buffer, tamanioBuffer, logger);
                free(buffer);
                t_mensaje *mensaje = _receive_message(conexion_RAM, logger);
                log_info(logger, "Recibi mensaje de RAM: %s - %d",
                mensaje->identifier,
                mensaje->command);

                if (mensaje->command == 200) {
                    //si memoria me manda la proxima tarea vuelvo a entrar al while
                    // Recibi la siguiente tarea
                    memcpy(&tamanioTarea, mensaje->payload, sizeof(int));
                    tcbTripulante->instruccion_actual = malloc(tamanioTarea + 1);
                    memcpy(tcbTripulante->instruccion_actual, mensaje->payload + sizeof(int), tamanioTarea);
                    tcbTripulante->instruccion_actual[tamanioTarea] = '\0';
                }
                
                else if (mensaje->command == 560) {
                    log_info(logger, "El tripulante ya realizó todas las tareas");
                    //pasarlo a exit
                    break;
                }
                free(mensaje->payload);
                free(mensaje->identifier);
                free(mensaje);*/
            }
        }
    }
}

void funcionhNewaReady (t_log* logger) {
    while (validador==1) {
        while (planificacion_pausada==0) {
            while (!queue_is_empty(cola_new)) // si hay nodos en New, los pasa a Ready
            {   
                tcb* aux_TCB = malloc (sizeof(tcb));
                pthread_mutex_lock(&mutexNew);
                aux_TCB = queue_pop(cola_new);
                pthread_mutex_unlock(&mutexNew);
                aux_TCB->status = 'R';
                pthread_mutex_lock(&mutexReady);
                queue_push(ready, (void*) aux_TCB);
                pthread_mutex_unlock(&mutexReady);
            }
        }
    }
}

void funcionhReadyaExec (t_log* logger){
     while (validador==1) {
        while (planificacion_pausada==0) {
            while (!queue_is_empty(ready) && queue_size(exec) < grado_multitarea) // si hay nodos en Ready, los pasa a Exec
            {
                tcb* aux_TCB = malloc (sizeof(tcb));
                pthread_mutex_lock(&mutexReady);
                aux_TCB = queue_pop(ready);
                pthread_mutex_unlock(&mutexReady);
                aux_TCB->status = 'E';
                pthread_mutex_lock(&mutexExec);
                queue_push(exec, (void*) aux_TCB);
                pthread_mutex_unlock(&mutexExec);
            }
        }
    }
}

void funcionCambioExecIO(void* nodo, int posicion){
    tcb* aux = (tcb *) nodo;
    if(aux->status == 'I'){
        pthread_mutex_lock(&mutexExec);
  		tcb *tcbAMover = list_get(exec->elements, posicion);
        pthread_mutex_unlock(&mutexExec);
        pthread_mutex_lock(&mutexBloqIO);
        queue_push(bloq_io, (void*)tcbAMover);
        pthread_mutex_unlock(&mutexBloqIO);
        pthread_mutex_lock(&mutexExec);
        list_remove(exec->elements, posicion);
        pthread_mutex_unlock(&mutexExec);
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
 
void funcionhExecaBloqIO (t_log* logger){
    while (validador==1) {
        while (planificacion_pausada==0) {
            while (!queue_is_empty(exec)){  
                list_iterate_position(exec->elements, funcionCambioExecIO); 
            }
        }
    }
}

/*
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

void funcionhBloqIOaReady (t_log* logger){
    while (validador){
        while(planificacion_pausada == 0) {
            while (!queue_is_empty(bloq_io))
            {
                tcb* aux_TCB = malloc (sizeof(tcb));
                aux_TCB = queue_peek(bloq_io);
                queue_pop(bloq_io);
                queue_push(ready, (void*) aux_TCB);
                aux_TCB->status = 'R';
                free(aux_TCB);
            }
        }
    }
}

void funcionhExecaExit (t_log* logger){
    while (validador){
        while(planificacion_pausada == 0) {
            while (!queue_is_empty(bloq_emer))
            {
                tcb* aux_TCB = malloc (sizeof(tcb));
                aux_TCB = queue_peek(exec);
                queue_pop(exec);
                queue_push(cola_exit, (void*) aux_TCB);
                aux_TCB->status = 'X';
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
        // printf("Length: %d - String: %s", read, line);

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
            if (parametros[2+i] == NULL) {//Si no exite ese elemento/índice del vector, las posiciones son 0|0
                hayParametros = false;
                posX = 0;
                posY = 0;
            }else {
                char** posicion = string_split(parametros[2+i], "|");
                posX = atoi(posicion[0]);
                posY = atoi(posicion[1]);
            }
        }
      
        int tid = (nuevoPCB->pid) * 100 + i;
      	
        tcb* nuevoTCB = crear_TCB(contadorPCBs, posX, posY, tid, logger);
        list_add (nuevoPCB->listaTCB, (void*) nuevoTCB);
        
      	// Copiar cada TCB al buffer
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
  	// Envio todo el buffer a memoria
    _send_message(conexion_RAM, "DIS", 610, buffer_a_enviar, tamanioBuffer, logger);
  	t_mensaje *mensaje = _receive_message(conexion_RAM, logger);

  	if (mensaje->command == 200) {
      return nuevoPCB;
    }
  
  	return NULL;
}