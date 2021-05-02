#include<stdio.h>
#include<stdlib.h>
#include<ctype.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include<readline/readline.h>
#include<sys/socket.h>

int conexion_RAM;
int conexion_IMS;
char* ip_RAM;
char* puerto_RAM;
char* ip_IMS;
char* puerto_IMS;

t_log* logger;
logger = log_create("consola.log", "consola", 1, LOG_LEVEL_INFO);

t_config* config;
config = config_create("./archivo.config"); //Hay que ver cómo se llama el archivo.config cuando nos lo den
ip_RAM = config_get_string_value(config, "IP_MI_RAM_HQ");
puerto_RAM = config_get_string_value(config, "PUERTO_MI_RAM_HQ");
ip_IMS = config_get_string_value(config, "IP_I_MONGO_STORE");
puerto_IMS = config_get_string_value(config, "PUERTO_I_MONGO_STORE");

conexion_RAM = _connect(ip_RAM, puerto_RAM, logger);
conexion_IMS = _connect(ip_IMS, puerto_IMS, logger);

char* leido;
leido = readline(">");

const int largo = 6;
char vector_mensajes_consola[largo] = {"INICIAR_PLANIFICACION", "PAUSAR_PLANIFICACION", "INICIAR_PATOTA",
"LISTAR_TRIPULANTES", "EXPULSAR_TRIPULANTE", "OBTENER_BITACORA" /*"SALIR"*/};

char* principio_leido;
int p_id_tripulante;
int p_cant_tripulantes;
char* p_camino_archivo;
int largo_archivo;
char* posiciones_a_RAM; //capaz es void*??
int instruccion_consola = 7;
int i;
for(i=0; i<largo; i++) {
    memcpy(principio_leido, leido, strlen(vector_mensajes_consola[i]));
    if(strcmp(principio_leido, vector_mensajes_consola[i]) == 0) {
        instruccion_consola = i;
        break;
    }
}
free(leido);

switch (instruccion_consola) {
    case 0: //INICIAR_PLANIFICACION
    // tiene que estar activo todo el tiempo, entonces, qué hace?
    // necesitamos hacer un hilo para planificar y comprobar si esta andando o no
        break;

    case 1: //PAUSAR_PLANIFICACION
    //No se puede cambiar tripulantes de estado, los que están en exec se detienen, sólo se pueden iniciar patotas
    // y listar tripulantes.
    // tiene que estar activo todo el tiempo, entonces, qué hace?
    // comprobar si ya esta andando o si ya se pauso (o si nunca empezo)
        break;

    case 2: //INICIAR_PATOTA
    //parametros
        int offset = strlen("INICIAR_PATOTA ");
        memcpy(p_cant_tripulantes, leido+offset, sizeof(int));
        offset += sizeof(int) + 1;

        //INICIAR_PATOTA 5 /hsduaisjdfuk.txt 289|377777 5|7 3|1

        int j = offset;
        while (leido[j]!= "." && j<strlen(leido)) {
            j++;
        }
        if (leido[j]== "." && leido[j+1]=="t" && leido[j+2]=="x" && leido[j+3]=="t" && leido[j+4]==" ") {
            largo_archivo = j + 3 - offset;
            memcpy(p_camino_archivo, leido+offset, largo_archivo);
        }
        int offset += largo_archivo+1;
        int k = offset;
        int cant_digitos=0;
        while (k<strlen(leido) && p_cant_tripulantes!=0) {
            while (leido[k]!= "|" && isdigit(leido[k])) {
                cant_digitos++;
                k++; 
            }
            if(!isdigit(leido[k]) && leido[k]!= "|") {
                printf("ERROR");
            }
            memcpy(posiciones_a_RAM, leido+offset, sizeof(int)*cant_digitos);
            p_cant_tripulantes--;
        }

    //buffer = 289-3777777-5-7-3-1
        
    // crear pcb, agregarlo a nuestra lista de control, avisarle/mandarselo a Mi-RAM HQ
    // crear hilos/tripulantes, crear un TCB por hilo, enviar este TCB a NEW, agregar el TCB a nuestra lista de control
        break;

    case 3: //LISTAR_TRIPULANTES
        log_info(logger, "--------------------------------------------------------------------");
        log_info(logger, "Estado de la Nave: ");
        //fecha y hora actual
        while (/*lista de tripulantes no vacía*/) {
	    log_info(logger, "Tripulante: ");
        log_info(logger, "%d", /*id del nodo*/);
        log_info(logger, "Patota: ");
        log_info(logger, "%d", /*patota del nodo*/);
        log_info(logger, "Status: ");
        log_info(logger, "%s", /*status del nodo*/);
        }
        log_info(logger, "--------------------------------------------------------------------");
        break;

    case 4: //EXPULSAR_TRIPULANTE
        int offset = strlen("EXPULSAR_TRIPULANTE ");
        memcpy(p_id_tripulante, leido+offset, sizeof(int));
        _send_message(conexion_RAM, "RAM", 530, PAYLOAD, PAY_LEN, logger);
        // en el payload, hay que avisar qué tripulante es (p_id_tripulante)
        // Acá hay recv??
        break;

    case 5: // OBTENER_BITACORA
        int offset = strlen("OBTENER_BITACORA ");
        memcpy(p_id_tripulante, leido+offset, sizeof(int));
         _send_message(conexion_IMS, "IMS", 750, PAYLOAD, PAY_LEN, logger);
        // en el payload, hay que avisar qué tripulante es (p_id_tripulante)
        // se muestra la bitácora? cómo la recibimos si no somos servidor? por el recv?
        break;

    default:
        log_info(logger, "El mensaje ingresado no corresponde a una acción propia del Discordiador");
}


