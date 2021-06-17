#include"headers/planificador.h"

// algoritmos (FIFO y RR)

/*
funcionTCB{
    while(estoyVivo){
        while(estado == E){

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


char** listaTareas[CANTTAREAS] = {"GENERAR_OXIGENO", "CONSUMIR_OXIGENO", "GENERAR_COMIDA", "CONSUMIR_COMIDA", "GENERAR_BASURA", "DESCARTAR_BASURA"};

void funcionTripulante (t_log* logger, tcb* tcbTripulante){
    while(tcbTripulante->status != 'X'){
        while(tcbTripulante->status == 'E'){
            int tarea = 6;
            int i;
            //TAREA PARAMETROS;POS X;POS Y;TIEMPO
            for(i=0; i<CANTTAREAS; i++) {
                if (!strcmp(tcbTripulante->instruccion_actual, listaTareas[i])) {
                    tarea = i;
                    break;
                }
            }

            switch(tarea) {
                case 0://GENERAR_OXIGENO
                    log_info(logger, "ENTRO GENERAR OXIGENO");
                    break;

                case 1: //CONSUMIR_OXIGENO
                    log_info(logger, "ENTRO CONSUMIR OXIGENO");
                    break;

                case 2://GENERAR_COMIDA
                    log_info(logger, "ENTRO GENERAR COMIDA");
                    break;

                case 3: //CONSUMIR_COMIDA
                    log_info(logger, "ENTRO CONSUMIR COMIDA");
                    break;

                case 4://GENERAR_BASURA
                    log_info(logger, "ENTRO GENERAR BASURA");
                    break;

                case 5: //DESCARTAR_BASURA
                    log_info(logger, "ENTRO DESCARTAR BASURA");
                    break;
                
                case 6: //OTROS
                    log_info(logger, "ENTRO OTROS");
                    break;

                default: //CUALQUIER VERDURA
                    log_info(logger, "La tarea ingresada no posee un formato de tarea correcto");
                    break;

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
                aux_TCB -> status = 'R';
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
                aux_TCB -> status = 'E';
                pthread_mutex_lock(&mutexExec);
                queue_push(exec, (void*) aux_TCB);
                pthread_mutex_unlock(&mutexExec);
            }
        }
    }
}

/*void funcionhExecaBloqEmer (t_log* logger){
    
    while (validador && planificacion_pausada == 0) {
        while (exec != NULL && contExec != 0)
        {
            tcb* aux_TCB = malloc (sizeof(tcb));
            aux_TCB = queue_peek(exec);
            queue_pop(exec);
            queue_push(bloq_emer, (void*) aux_TCB);
            aux_TCB -> status = 'M';
            free(aux_TCB);
        }
    }
}

void funcionhExecaBloqIO (t_log* logger){
    
    while (validador && planificacion_pausada == 0) {
        while (exec != NULL && contExec != 0)
        {
            tcb* aux_TCB = malloc (sizeof(tcb));
            aux_TCB = queue_peek(exec);
            queue_pop(exec);
            queue_push(bloq_io, (void*) aux_TCB);
            aux_TCB -> status = 'I';
            free(aux_TCB);
        }
    }
}

void funcionhBloqEmeraReady (t_log* logger){
    
    while (validador && planificacion_pausada == 0) {
        while (bloq_emer != NULL && contBloqEmer != 0)
        {
            tcb* aux_TCB = malloc (sizeof(tcb));
            aux_TCB = queue_peek(bloq_emer);
            queue_pop(bloq_emer);
            queue_push(ready, (void*) aux_TCB);
            aux_TCB -> status = 'R';
            free(aux_TCB);
        }
    }
}

void funcionhBloqIOaReady (t_log* logger){
    
    while (validador && planificacion_pausada == 0) {
        while (bloq_io != NULL && contBloqIO != 0)
        {
            tcb* aux_TCB = malloc (sizeof(tcb));
            aux_TCB = queue_peek(bloq_io);
            queue_pop(bloq_io);
            queue_push(ready, (void*) aux_TCB);
            aux_TCB -> status = 'R';
            free(aux_TCB);
        }
    }
}

void funcionhExecaExit (t_log* logger){
    
    while (validador && planificacion_pausada == 0) {
        while (exec != NULL && contExec != 0)
        {
            tcb* aux_TCB = malloc (sizeof(tcb));
            aux_TCB = queue_peek(exec);
            queue_pop(exec);
            queue_push(cola_exit, (void*) aux_TCB);
            aux_TCB -> status = 'X';
            free(aux_TCB);
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

void send_tareas(int id_pcb, char *ruta_archivo, int conexion_RAM, t_log* logger) {
    FILE * fp;
    char * line = NULL;
    size_t len = 0;
    ssize_t read;

    int b_size = 0;
    int offset = 0;
    int new_size;
    void *temp;

    void *buffer = malloc(sizeof(int));

    memcpy(buffer + offset, &id_pcb, sizeof(int));
    offset += sizeof(int);
    b_size += sizeof(int);

    fp = fopen(ruta_archivo, "r");
    if (fp == NULL)
        exit(EXIT_FAILURE);

    while ((read = getline(&line, &len, fp)) != -1) {

        // printf("Length: %d - String: %s", read, line);

        if (line[ read - 1 ] == '\n') {
            read--;
            memset(line + read, 0, 1);
        }

        new_size = sizeof(int) + read;
        
        temp = _serialize(new_size, "%s", line);

        b_size += new_size;
        buffer = realloc(buffer, b_size);
        
        memcpy(buffer + offset, temp, new_size);
        offset += new_size;

        free(temp);
    }

    fclose(fp);
    if (line)
        free(line);

    _send_message(conexion_RAM, "DIS", 510, buffer, offset, logger);

    free(buffer);
}

tcb* crear_TCB(int idP, int posX, int posY, int idT, char* tarea, t_log* logger)
{
    tcb* nuevoTCB = malloc (sizeof(tcb));
	nuevoTCB -> tid = idT;
    nuevoTCB -> pid = idP;
    nuevoTCB -> status = 'N';
    nuevoTCB -> posicionX = posX;
    nuevoTCB -> posicionY = posY;
    nuevoTCB -> instruccion_actual = malloc (strlen(tarea) + 1);
    strcpy(nuevoTCB -> instruccion_actual, tarea);
    nuevoTCB -> instruccion_actual[strlen(tarea)]='\0';
    pthread_t hiloTripulante;
    pthread_create(&hiloTripulante, NULL, (void *) funcionTripulante, logger, nuevoTCB);
    pthread_detach(hiloTripulante);
    return nuevoTCB;
}

pcb* crear_PCB(char** parametros, int conexion_RAM, t_log* logger)
{                        
    int cant_tripulantes = atoi(parametros[1]);
    contadorPCBs++;
    pcb* nuevoPCB = malloc(sizeof(pcb));
    nuevoPCB -> pid = contadorPCBs;
    nuevoPCB -> listaTCB = list_create();
    nuevoPCB -> rutaTareas = malloc (strlen(parametros[2]) + 1);
    strcpy(nuevoPCB -> rutaTareas, parametros[2]);
    nuevoPCB -> rutaTareas[strlen(parametros[2])]='\0';
    int posX;
    int posY;
    bool hayParametros = true;
    for(int i = 1; i<=cant_tripulantes; i++)
    {
        if (hayParametros) {
            if (parametros[2+i] == NULL) {//Si no exite ese elemento/índice del vector, las posiciones son 0|0
                    hayParametros = false;
                    posX = 0;
                    posY = 0;
            }
            else {
                char** posicion = string_split(parametros[2+i], "|");
                posX = atoi(posicion[0]);
                posY = atoi(posicion[1]);
            }
        }
        int tid = (nuevoPCB -> pid) * 100 + i;
        int tamanioBuffer;
        void* buffer;
        t_mensaje *mensajeRecibido = malloc (sizeof(t_mensaje));
        tamanioBuffer = sizeof(int)*4 + sizeof(char);
        buffer = _serialize(tamanioBuffer, "%d%d%d%d%c", contadorPCBs, tid, posX, posY,'N');
        _send_message(conexion_RAM, "DIS", 500, buffer, tamanioBuffer, logger);
        char* temp_tarea= malloc (strlen("aburrirse")+1);
        strcpy(temp_tarea, "aburrirse");
        /*mensajeRecibido = _receive_message(conexion_RAM, logger);
        int comando = mensajeRecibido -> command;
        char* temp_tarea;

        if (comando == 100) {
            temp_tarea = malloc (strlen(mensajeRecibido -> payload) + 1);
            strcpy(temp_tarea, mensajeRecibido -> payload);}
        else if (comando == 401) {
            send_tareas(contadorPCBs, parametros[2], conexion_RAM, logger);
            //recv!
            _send_message(conexion_RAM, "DIS", 500, buffer, tamanioBuffer, logger);
            mensajeRecibido = _receive_message(conexion_RAM, logger);
            temp_tarea = malloc (strlen(mensajeRecibido -> payload) + 1);
            strcpy(temp_tarea, mensajeRecibido -> payload);}
        else if (comando == 402) {
            log_info(logger, "No hay suficiente memoria para iniciar otro tripulante");
            break;}*/
        tcb* nuevoTCB = crear_TCB(contadorPCBs, posX, posY, tid, temp_tarea, logger);
        list_add (nuevoPCB -> listaTCB, (void*) nuevoTCB);
        queue_push (cola_new, (void*) nuevoTCB);
        free(buffer);
        free(temp_tarea);
    }
    return nuevoPCB;
}

/*void funcionEliminarListaPatotas(void* nodoPatota) {

    free (np1 -> rutaTareas);
    list_destroy(listaTCB);
    free (np1);
}

void funcionEliminarListaTripulantes(void* nodoTripulante) {

    free(instruccion_actual);
    free (nt1);
}

void funcionTerminarPlanificacion() {
    list_destroy_and_destroy_elements (listaPCB, funcionEliminarListaPatotas);
    list_destroy_and_destroy_elements (listaTCB, funcionEliminarListaTripulantes);
}*/