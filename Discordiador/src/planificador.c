#include"headers/planificador.h"

// algoritmos (FIFO y RR)

// crear hilo

//pthread_create(pthread_t *th, const pthread_attr_t *attr, void *(* func)(void *), void *arg);
//pthread_join(pthread_t t, void **res);

//destroy

// hilos que pasan de estado
/*void funcionhReadyaExec (t_log* logger){
    
    while (validador) {
        while (ready != NULL && contReady != 0 && contExec <= grado_multitarea) // si hay nodos en Ready, los pasa a Exec
        {
            tcb* aux_TCB = malloc (sizeof(tcb));
            aux_TCB = queue_peek(ready);
            queue_pop(ready);
            queue_push(exec, (void*) aux_TCB);
            aux_TCB -> status = 'E';
            free(aux_TCB);
            contReady--;
            contExec++;
        }
    }
}

void funcionhExecaBloqEmer (t_log* logger){
    
    while (validador) {
        while (exec != NULL && contExec != 0)
        {
            tcb* aux_TCB = malloc (sizeof(tcb));
            aux_TCB = queue_peek(exec);
            queue_pop(exec);
            queue_push(bloq_emer, (void*) aux_TCB);
            aux_TCB -> status = 'M';
            free(aux_TCB);
            contExec--;
            contBloqEmer++;
        }
    }
}

void funcionhExecaBloqIO (t_log* logger){
    
    while (validador) {
        while (exec != NULL && contExec != 0)
        {
            tcb* aux_TCB = malloc (sizeof(tcb));
            aux_TCB = queue_peek(exec);
            queue_pop(exec);
            queue_push(bloq_io, (void*) aux_TCB);
            aux_TCB -> status = 'I';
            free(aux_TCB);
            contExec--;
            contBloqIO++;
        }
    }
}

void funcionhBloqEmeraReady (t_log* logger){
    
    while (validador) {
        while (bloq_emer != NULL && contBloqEmer != 0)
        {
            tcb* aux_TCB = malloc (sizeof(tcb));
            aux_TCB = queue_peek(bloq_emer);
            queue_pop(bloq_emer);
            queue_push(ready, (void*) aux_TCB);
            aux_TCB -> status = 'R';
            free(aux_TCB);
            contBloqEmer--;
            contReady++;
        }
    }
}

void funcionhBloqIOaReady (t_log* logger){
    
    while (validador) {
        while (bloq_io != NULL && contBloqIO != 0)
        {
            tcb* aux_TCB = malloc (sizeof(tcb));
            aux_TCB = queue_peek(bloq_io);
            queue_pop(bloq_io);
            queue_push(ready, (void*) aux_TCB);
            aux_TCB -> status = 'R';
            free(aux_TCB);
            contBloqIO--;
            contReady++;
        }
    }
}

void funcionhExecaExit (t_log* logger){
    
    while (validador) {
        while (exec != NULL && contExec != 0)
        {
            tcb* aux_TCB = malloc (sizeof(tcb));
            aux_TCB = queue_peek(exec);
            queue_pop(exec);
            queue_push(cola_exit, (void*) aux_TCB);
            aux_TCB -> status = 'X';
            free(aux_TCB);
            contExec--;
            contExit++;
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

tcb* crear_TCB(int idP, int posX, int posY, int idT, char* tarea)
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
    //crear hilo de tripulante
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
        log_info(logger, "%d", tid);
        log_info(logger, "%d", contadorPCBs);
        log_info(logger, "%d", posX);
        log_info(logger, "%d", posY);
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
        tcb* nuevoTCB = crear_TCB(contadorPCBs, posX, posY, tid, temp_tarea);
        list_add (nuevoPCB -> listaTCB, (void*) nuevoTCB);
        queue_push (cola_new, (void*) nuevoTCB);
        free(buffer);
        free(temp_tarea);
    }
    return nuevoPCB;
}

/*void funcionEliminarListaPatotas(void* nodoPatota) {

    free (np1 -> PID);
    //free (np1 -> cantTrip); este no porque es int???
    free (np1 -> bufferIDTrip);
    free (np1);
}

void funcionEliminarListaTripulantes(void* nodoTripulante) {

    free (nt1 -> TID);
    free (nt1 -> PID);
    free (nt1 -> status);
    //free (nt1 -> posicionX; estos no porque son int???
    //free (nt1 -> posicionY);
    free (nt1);
}

void funcionTerminarPlanificacion() {
    list_destroy_and_destroy_elements (listaPCB, funcionEliminarListaPatotas);
    list_destroy_and_destroy_elements (listaTCB, funcionEliminarListaTripulantes);
}*/