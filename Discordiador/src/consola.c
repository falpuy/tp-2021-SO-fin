#include "headers/consola.h"

// crear hilo

void funcionConsola(t_log* logger, int conexion_RAM, int conexion_IMS) {
    int validador = 1;
    char* leido;
    char* vector_mensajes_consola[]= {"INICIAR_PLANIFICACION","PAUSAR_PLANIFICACION","INICIAR_PATOTA","LISTAR_TRIPULANTES","EXPULSAR_TRIPULANTE","OBTENER_BITACORA","SALIR"};

    while(validador) {
        while(strcmp(leido = readline("> "), "") != 0) { //mientras se ingrese algo por consola
            char** parametros = string_split(leido," "); //char**: vector de strings, cada elemento del vector es un parametro, menos el primero que es el mensaje!

            free(leido);

            t_mensaje *mensajeRecibido;
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
                // si hay nodos en new, los pasa a ready 
                    break;

                case 1: //PAUSAR_PLANIFICACION
                log_info(logger, "ENTRO PAUSAR");
                //No se puede cambiar tripulantes de estado, los que están en exec se detienen, sólo se pueden iniciar patotas
                // y listar tripulantes.
                // comprobar si ya esta andando o si ya se pauso (o si nunca empezo)
                // si hay nodos en exec, los pasa a ready o a new
                    break;

                case 2: //INICIAR_PATOTA
                    // Ej: INICIAR_PATOTA 5 /home/utnso/tareas/tareasPatota5.txt 1|1 3|4 1|1 3|4 1|1 
                    //abrir y copiar en una lista o buffer parametros[2]
                    log_info(logger, "ENTRO INICIAR PATO");
                    tamanioBuffer = sizeof(int);
                    buffer = _serialize(tamanioBuffer, "%d", parametros[1]);
                    
                    FILE* archivo_tareas;
                    char* ruta_tareas = malloc (strlen(parametros[2])+1);
                    strcpy (ruta_tareas, parametros[2]);
                    archivo_tareas = fopen(ruta_tareas, "r");
                    // primero separar por \n -> luego por ; -> finalmente por espacios (en el primer miembro, el de la funcion)
                    //TAREA PARAMETRO1 PARAMETRO2 PARAMETRO3;POS X;POS Y;TIEMPO
                    //contenido adentro del string, COMO no se

                    // Cerrar archivo
                    _send_message(conexion_RAM, "DIS", 710, buffer, tamanioBuffer, logger);

                    PCB* nuevoPCB = crear_PCB (/*tareas*/, parametros, contadorPCBs);
                    list_add (listaPCB, (void*) nuevoPCB);
                    
                // crear pcb, agregarlo a nuestra lista de control, avisarle/mandarselo a Mi-RAM HQ
                // crear hilos/tripulantes, crear un TCB por hilo, enviar este TCB a NEW, agregar el TCB a nuestra lista de control
                    break;

                case 3: //LISTAR_TRIPULANTES
                    log_info(logger, "ENTRO LISTAR");
                    log_info(logger, "--------------------------------------------------------------------");
                    char* hora_y_fecha_actual;
                    hora_y_fecha_actual = temporal_get_string_time("%d/%m/%y %H:%M:%S");
                    log_info(logger, "Estado de la Nave: %s", hora_y_fecha_actual);
                    while (/*lista de tripulantes no vacía*/) {
                    log_info(logger, "Tripulante: %d", /*id del nodo*/);
                    log_info(logger, "Patota: %d", /*patota del nodo*/);
                    log_info(logger, "Status: %s", /*status del nodo*/);
                    }
                    log_info(logger, "--------------------------------------------------------------------");
                    break;

                case 4: //EXPULSAR_TRIPULANTE
                    log_info(logger, "ENTRO EXPULSAR");
                    tamanioBuffer = sizeof(int);
                    buffer = malloc (tamanioBuffer);
                    buffer = _serialize(tamanioBuffer, "%d", parametros[1]);
                    _send_message(conexion_RAM, "DIS", 530, buffer, tamanioBuffer, logger);
                    //mensajeRecibido = _receive_message(conexion_IMS, logger); capaz no es necesario
                    //log_info(logger, "SALIÓ %s", mensajeRecibido -> payload);
                    //sacar de la lista correspondiente al tripulante y ponerlo en exit
                    free(parametros[0]);
                    free(parametros[1]);
                    free(parametros);
                    break;

                case 5: // OBTENER_BITACORA
                    log_info(logger, "ENTRO OBTENER");
                    tamanioBuffer = sizeof(int);
                    buffer = malloc (tamanioBuffer);
                    buffer = _serialize(tamanioBuffer, "%d", parametros[1]);
                    _send_message(conexion_IMS, "DIS", 750, buffer, tamanioBuffer, logger);
                    mensajeRecibido = _receive_message(conexion_IMS, logger);
                    log_info(logger, "BITACORA: %s", mensajeRecibido -> payload);
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

