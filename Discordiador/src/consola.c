#include "headers/consola.h"

// crear hilo

void mostrarTripulante(void* elemento){
    tcb* tripulante = (tcb*) elemento;
    log_info(logger, "Tripulante: %d\t", tripulante -> tid);
    log_info(logger, "Patota: %d\t", tripulante -> pid);
    switch(tripulante -> status){
        case 'N':
            log_info(logger, "Status: NEW\n");
            break;
        case 'R':
            log_info(logger, "Status: READY\n");
            break;
        case 'E':
            log_info(logger, "Status: EXEC\n");
            break;
        case 'I':
            log_info(logger, "Status: BLOQ IO\n");
            break;
        case 'M':
            log_info(logger, "Status: BLOQ EMERGENCIA\n");
            break;
        case 'X':
            log_info(logger, "Status: EXIT\n");
            break;
    }
}

void mostrarListaTripulantes(void* elemento){
    pcb* patotas = (pcb*) elemento;
    list_iterate(patotas -> listaTCB, mostrarTripulante);
}

bool buscarTripulante (void* elemento){
    tcb* tripulante = (tcb*) elemento;
    return tripulante -> tid == atoi(parametros[1]);
}

void expulsarNodo (t_queue* cola, char* nombre_cola){
    if (loEncontro==0) {
        tcb* tripulanteAExpulsar = malloc (sizeof(tcb));
        tripulanteAExpulsar = list_find (cola->elements, buscarTripulante);
        if (tripulanteAExpulsar != NULL) {
            log_info(logger, "se expulsó el tripulante %d de la cola %s", tripulanteAExpulsar -> tid, nombre_cola);
            //pasarlo a exit
            loEncontro=1;
            free(tripulanteAExpulsar);
        }
        else{
            log_info(logger, "No se encontro el tripulante en la cola %s", nombre_cola);
            free(tripulanteAExpulsar);
        }
    }
}

void funcionConsola(t_log* logger, int conexion_RAM, int conexion_IMS) {
    contadorPCBs = 0;
    contNew = 0;
    contReady = 0;
    contExec = 0;
    contBloqIO = 0;
    contBloqEmer = 0;
    contExit = 0;
    validador = 1;
    planificacion_pausada = 1;
    char* leido;
    char* vector_mensajes_consola[]= {"INICIAR_PLANIFICACION","PAUSAR_PLANIFICACION","INICIAR_PATOTA","LISTAR_TRIPULANTES","EXPULSAR_TRIPULANTE","OBTENER_BITACORA","SALIR"};

    while(validador) {
        while(strcmp(leido = readline("> "), "") != 0) { //mientras se ingrese algo por consola
            parametros = string_split(leido," "); //char**: vector de strings, cada elemento del vector es un parametro, menos el primero que es el mensaje!

            free(leido);

            t_mensaje *mensajeRecibido = malloc (sizeof(t_mensaje));
            int tamanioBuffer;
            void* buffer;
            int instruccion_consola = 8;
            int i;

            for(i=0; i<LARGO; i++) {
                if (!strcmp(parametros[0], vector_mensajes_consola[i])) {
                    instruccion_consola = i;
                    break;
                }
            }

            switch (instruccion_consola) {
                case 0: //INICIAR_PLANIFICACION
                    log_info(logger, "ENTRO INICIAR PLANI");
                    planificacion_pausada = 0;
                    log_info(logger, "Planificacion pausada = %d", planificacion_pausada);
                    break;

                case 1: //PAUSAR_PLANIFICACION
                log_info(logger, "ENTRO PAUSAR");
                planificacion_pausada = 1;
                //No se puede cambiar tripulantes de estado, los que están en exec se detienen, sólo se pueden iniciar patotas
                // y listar tripulantes.
                    /*while (exec != NULL) // si hay nodos en exec, los pasa a ready
                    {
                        TCB* aux_TCB = malloc (sizeof(TCB));
                        aux_TCB = queue_peek(exec);
                        queue_pop(exec);
                        queue_push(ready, aux_TCB);
                        aux_TCB -> status = 'R';
                        free(aux_TCB);
                    }*/
                    break;

                case 2: //INICIAR_PATOTA
                    log_info(logger, "ENTRO INICIAR PATO");

                    pcb* nuevoPCB = crear_PCB (parametros, conexion_RAM, logger);
                    list_add (listaPCB, (void*) nuevoPCB);
                    
                //avisarle/mandarle PCB a Mi-RAM HQ
                // crear hilos
                    break;

                case 3: //LISTAR_TRIPULANTES
                    log_info(logger, "ENTRO LISTAR");
                    log_info(logger, "--------------------------------------------------------------------");
                    char* hora_y_fecha_actual;
                    hora_y_fecha_actual = temporal_get_string_time("%d/%m/%y %H:%M:%S");
                    log_info(logger, "Estado de la Nave: %s", hora_y_fecha_actual);
                    list_iterate(listaPCB, mostrarListaTripulantes);
                    log_info(logger, "--------------------------------------------------------------------");
                    break;

                case 4: //EXPULSAR_TRIPULANTE
                    log_info(logger, "ENTRO EXPULSAR");
                    if (!planificacion_pausada) {
                        loEncontro=0;
                        tamanioBuffer = sizeof(int);
                        buffer = malloc (tamanioBuffer);
                        buffer = _serialize(tamanioBuffer, "%d", parametros[1]);
                        _send_message(conexion_RAM, "DIS", 530, buffer, tamanioBuffer, logger);
                        //mensajeRecibido = _receive_message(conexion_IMS, logger); capaz no es necesario
                        //log_info(logger, "SALIÓ %s", mensajeRecibido -> payload);

                        expulsarNodo(cola_new, "New");
                        expulsarNodo(ready, "Ready");
                        expulsarNodo(exec, "Exec");
                        expulsarNodo(bloq_io, "Bloqueado por IO");
                        expulsarNodo(bloq_emer, "Bloqueado por emergencia");
                        if (loEncontro==0)
                            log_info(logger, "El tripulante que se quiere expulsar ya esta en estado EXIT o el ID ingresado es incorrecto");

                        free(parametros[0]);
                        free(parametros[1]);
                        free(parametros);
                    }
                    else{log_info(logger, "La planificación está pausada, no se puede expulsar a un tripulante");}
                    break;

                case 5: // OBTENER_BITACORA
                    log_info(logger, "ENTRO OBTENER");
                    tamanioBuffer = sizeof(int);
                    buffer = _serialize(tamanioBuffer, "%d", parametros[1]);
                    _send_message(conexion_RAM, "DIS", 750, buffer, tamanioBuffer, logger);
                    //mensajeRecibido = _receive_message(conexion_IMS, logger);
                    //log_info(logger, "BITACORA: %s", mensajeRecibido -> payload);
                    // después vemos qué mostramos con Delfi
                    free(parametros[0]);
                    free(parametros[1]);
                    free(parametros);
                    break;
                
                case 6: // SALIR
                    log_info(logger, "Salimos de la consola");
                    validador = 0;
                    //hacer los destroy de los config, logs, listas, colas, todo
                    break;

                default:
                    log_info(logger, "El mensaje ingresado no corresponde a una acción propia del Discordiador");
                    break;
            }
            if (!validador) {
                break;
            }
        }
    }
}

