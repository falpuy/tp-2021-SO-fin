#include<stdio.h>
#include<stdlib.h>
#include<ctype.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include<readline/readline.h>
#include<sys/socket.h>

void funcionConsola(t_log* logger) {
    int validador = 1;
    char* leido;

    while(validador) {
        while(strcmp(leido = readline("> "), "") != 0) { //mientras se ingrese algo por consola
            char** parametros = string_split(leido," "); //char**: vector de strings, cada elemento del vector es un parametro, menos el primero que es el mensaje!

free(leido);

const int largo = 7;
char vector_mensajes_consola[largo] = {"INICIAR_PLANIFICACION", "PAUSAR_PLANIFICACION", "INICIAR_PATOTA",
"LISTAR_TRIPULANTES", "EXPULSAR_TRIPULANTE", "OBTENER_BITACORA", "SALIR"};

t_mensaje *mensajeRecibido;
int tamanioBuffer;
void* buffer = malloc (tamanioBuffer);
int instruccion_consola = 7;
int i;

for(i=0; i<largo; i++) {
   if (strcmp(parametros[0], vector_mensajes_consola[i])==0) {
        instruccion_consola = i;
        break; // sale del for?
   }
}

switch (instruccion_consola) {
    case 0: //INICIAR_PLANIFICACION
    // si hay nodos en new, los pasa a ready 
        break;

    case 1: //PAUSAR_PLANIFICACION
    //No se puede cambiar tripulantes de estado, los que están en exec se detienen, sólo se pueden iniciar patotas
    // y listar tripulantes.
    // comprobar si ya esta andando o si ya se pauso (o si nunca empezo)
    // si hay nodos en exec, los pasa a ready o a new
        break;

    case 2: //INICIAR_PATOTA
        //abrir y copiar en una lista parametros[2]
        _serialize(sizeof(int), "%d", parametros[1]);
        for (int j=3; j</*tamaño del vector parametros*/; j++) {
            parametros[3] al buffer

        }
        _send_message(conexion_RAM, "DIS", 710, buffer, tamanioBuffer, logger);
        mensajeRecibido = _receive_message(conexion_IMS, logger);
        log_info(logger, "SALIÓ %s", mensajeRecibido -> payload);
        
    // crear pcb, agregarlo a nuestra lista de control, avisarle/mandarselo a Mi-RAM HQ
    // crear hilos/tripulantes, crear un TCB por hilo, enviar este TCB a NEW, agregar el TCB a nuestra lista de control
        break;

    case 3: //LISTAR_TRIPULANTES
        log_info(logger, "--------------------------------------------------------------------");
        log_info(logger, "Estado de la Nave: ");
        //fecha y hora actual
        while (/*lista de tripulantes no vacía*/) {
	    log_info(logger, "Tripulante: %d", /*id del nodo*/);
        log_info(logger, "Patota: %d", /*patota del nodo*/);
        log_info(logger, "Status: %s", /*status del nodo*/);
        }
        log_info(logger, "--------------------------------------------------------------------");
        break;

    case 4: //EXPULSAR_TRIPULANTE
        _send_message(conexion_RAM, "DIS", 530, parametros[1], sizeof(int), logger);
        mensajeRecibido = _receive_message(conexion_IMS, logger);
        log_info(logger, "SALIÓ %s", mensajeRecibido -> payload);
        free(parametros[0]);
        free(parametros[1]);
        free(parametros);
        break;

    case 5: // OBTENER_BITACORA
        _send_message(conexion_IMS, "DIS", 750, parametros[1], sizeof(int), logger);
        mensajeRecibido = _receive_message(conexion_IMS, logger);
        log_info(logger, "BITACORA: %s", mensajeRecibido -> payload);
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
}


