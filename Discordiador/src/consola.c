#include "headers/consola.h"

// crear hilo

/*void mostrarTripulante(TCB* tripulante, t_log* logger){
    log_info(logger, "Tripulante: %d\t", tripulante -> TID);
    log_info(logger, "Patota: %d\t", tripulante -> PID);
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

void mostrarListaTripulantes(t_list* patotas, t_log* logger){
    list_iterate(patotas -> listaTCB, mostrarTripulante);
}*/

void funcionConsola(t_log* logger, int conexion_RAM, int conexion_IMS) {
    int validador = 1;
    char* leido;
    char* vector_mensajes_consola[]= {"INICIAR_PLANIFICACION","PAUSAR_PLANIFICACION","INICIAR_PATOTA","LISTAR_TRIPULANTES","EXPULSAR_TRIPULANTE","OBTENER_BITACORA","SALIR"};

    while(validador) {
        while(strcmp(leido = readline("> "), "") != 0) { //mientras se ingrese algo por consola
            char** parametros = string_split(leido," "); //char**: vector de strings, cada elemento del vector es un parametro, menos el primero que es el mensaje!

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
                    while (NEW != NULL) // si hay nodos en NEW, los pasa a Ready
                    {
                        TCB* aux_TCB = malloc (sizeof(TCB));
                        aux_TCB = queue_peek(NEW);
                        queue_pop(NEW);
                        queue_push(READY, aux_TCB);
                        aux_TCB -> status = 'R';
                        free(aux_TCB);
                    }
                    break;

                case 1: //PAUSAR_PLANIFICACION
                log_info(logger, "ENTRO PAUSAR");
                //No se puede cambiar tripulantes de estado, los que están en exec se detienen, sólo se pueden iniciar patotas
                // y listar tripulantes.
                // comprobar si ya esta andando o si ya se pauso (o si nunca empezo)
                    while (EXEC != NULL) // si hay nodos en exec, los pasa a ready
                    {
                        TCB* aux_TCB = malloc (sizeof(TCB));
                        aux_TCB = queue_peek(EXEC);
                        queue_pop(EXEC);
                        queue_push(READY, aux_TCB);
                        aux_TCB -> status = 'R';
                        free(aux_TCB);
                    }
                    break;

                case 2: //INICIAR_PATOTA
                    log_info(logger, "ENTRO INICIAR PATO");

                   /* PCB* nuevoPCB = crear_PCB (parametros, contadorPCBs, conexion_RAM, conexion_IMS);
                    list_add (listaPCB, (void*) nuevoPCB);*/
                    
                //avisarle/mandarle PCB a Mi-RAM HQ
                // crear hilos
                    break;

                case 3: //LISTAR_TRIPULANTES
                    log_info(logger, "ENTRO LISTAR");
                    /*log_info(logger, "--------------------------------------------------------------------");
                    char* hora_y_fecha_actual;
                    hora_y_fecha_actual = temporal_get_string_time("%d/%m/%y %H:%M:%S");
                    log_info(logger, "Estado de la Nave: %s", hora_y_fecha_actual);
                    list_iterate(listaPCB, mostrarListaTripulantes);
                    log_info(logger, "--------------------------------------------------------------------");*/
                    break;

                case 4: //EXPULSAR_TRIPULANTE
                    log_info(logger, "ENTRO EXPULSAR");
                    if (!planificacion_pausada) {
                        tamanioBuffer = sizeof(int);
                        buffer = malloc (tamanioBuffer);
                        buffer = _serialize(tamanioBuffer, "%d", parametros[1]);
                        _send_message(conexion_RAM, "DIS", 530, buffer, tamanioBuffer, logger);
                        //mensajeRecibido = _receive_message(conexion_IMS, logger); capaz no es necesario
                        //log_info(logger, "SALIÓ %s", mensajeRecibido -> payload);
                        //sacar de la lista correspondiente al tripulante y ponerlo en exit
                        #define LONGITUD 5
                        funcion(NEW, );
                        funcion(READY);
                        //poner nombres de colas en minúscula
                        t_queue** vectorDeColas = {NEW, READY, EXEC, BLOQ_IO, BLOQ_EMER};
                        int k;
                        TCB* aux_TCB = malloc (sizeof(TCB));
                        for (k= 0; k<LONGITUD; k++) {
                            while (vectorDeColas[k]) {
                                aux_TCB = queue_peek(vectorDeColas[k]);
                                if (parametros[1] == aux_TCB -> TID) {
                                    queue_pop(vectorDeColas[k]);
                                    queue_push(EXIT, aux_TCB);
                                    aux_TCB -> status = 'X';
                                    break;
                                }
                            free(aux_TCB);
                            }
                            if (aux_TCB //lleno)
                                {break;}
                        }
                        if (k==5 && aux_TCB //vacio) {
                            log_info(logger, "El tripulante que se quiere expulsar ya tiene estado EXIT");
                        }                
                        free(parametros[0]);
                        free(parametros[1]);
                        free(parametros);
                        free(aux_TCB);
                    }
                    else{log_info(logger, "La planificación está pausada, no se puede expulsar a un tripulante");}*/
                         
                    break;

                case 5: // OBTENER_BITACORA
                    log_info(logger, "ENTRO OBTENER");
                    tamanioBuffer = sizeof(int);
                    buffer = _serialize(tamanioBuffer, "%d", parametros[1]);
                    _send_message(conexion_IMS, "DIS", 750, buffer, tamanioBuffer, logger);
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

