#include "headers/planificador.h"

// hilos que pasan de estado
// algoritmos (FIFO y RR)

// crear hilo

void funcionPlanificador(t_log* logger) {
    NEW = queue_create();
    READY = queue_create();
    EXEC = queue_create();
    BLOQ_IO = queue_create();
    BLOQ_EMER = queue_create();
    EXIT = queue_create();

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

TCB* crear_TCB(int idP, int posX, int posY, int idT, char* tarea)
{
    TCB* nuevoTCB = malloc (sizeof(TCB));
	nuevoTCB -> TID = idT;
    nuevoTCB -> PID = idP;
    nuevoTCB -> status = 'N';
    nuevoTCB -> posicionX = posX;
    nuevoTCB -> posicionY = posY;
    nuevoTCB -> instruccion_actual = malloc (strlen(tarea) + 1);
    strcpy(nuevoTCB -> instruccion_actual, tarea);
    nuevoTCB -> instruccion_actual[strlen(tarea)]='\0';
    return nuevoTCB;
}

PCB* crear_PCB(char** parametros, int conexion_RAM, int conexion_IMS)
{                        
    int cant_tripulantes = parametros[1];
    contadorPCBs++;
    PCB* nuevoPCB = malloc(sizeof(PCB));
    nuevoPCB -> PID = contadorPCBs;
    nuevoPCB -> listaTCB = list_create();
    nuevoPCB -> rutaTareas = malloc (strlen(parametros[2]) + 1);
    strcpy(nuevoPCB -> rutaTareas, parametros[2]);
    nuevoPCB -> rutaTareas[strlen(parametros[2])]='\0';
    int posX = 0;
    int posY = 0;
    bool hayParametros = true;
    for(int i = 1; i<=cant_tripulantes; i++)
    {
        if (hayParametros) {
            if (strcmp(parametros[2+i], NULL) == 0) {//Si no exite ese elemento/índice del vector, las posiciones son 0|0
                    hayParametros = false;
            }
            else {
                char** posicion = string_split(parametros[2+i], "|");
                posX = atoi(posicion[0]);
                posY = atoi(posicion[1]);
            }
        }
        int TID = (nuevoPCB -> PID) * 100 + i;
        
        int tamanioBuffer;
        void* buffer;
        t_mensaje *mensajeRecibido = malloc (sizeof(t_mensaje));
        tamanioBuffer = sizeof(int)*4 + sizeof(char);
        buffer = _serialize(tamanioBuffer, "%d","%d","%d","%d","%c", contadorPCBs, TID, posX, posY, 'N');
        _send_message(conexion_RAM, "DIS", 500, buffer, tamanioBuffer, logger);
        mensajeRecibido = _receive_message(conexion_RAM, logger);
        int comando = mensajeRecibido -> command;
        char* temp_tarea;

        if (comando == 100) {
            temp_tarea = malloc (strlen(mensajeRecibido -> payload) + 1);
            strcpy(temp_tarea, mensajeRecibido -> payload);}
        else if (comando == 401) {
            send_tareas(contadorPCBs, parametros[2]);
            //recv!
            _send_message(conexion_RAM, "DIS", 500, buffer, tamanioBuffer, logger);
            mensajeRecibido = _receive_message(conexion_RAM, logger);
            temp_tarea = malloc (strlen(mensajeRecibido -> payload) + 1);
            strcpy(temp_tarea, mensajeRecibido -> payload);}
        else if (comando == 402) {
            log_info(logger, "No hay suficiente memoria para iniciar otro tripulante");
            break;}

        TCB* nuevoTCB = crear_TCB(contadorPCBs, posX, posY, TID, temp_tarea);
        list_add (nuevoPCB -> listaTCB, (void*) nuevoTCB);
        queue_push (NEW, nuevoTCB);
        nuevoTCB -> status = 'N';
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