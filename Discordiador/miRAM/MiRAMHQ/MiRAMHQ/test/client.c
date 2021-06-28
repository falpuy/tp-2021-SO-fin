#include <stdio.h>
#include <stdlib.h>
#include <unnamed/socket.h>
#include <commons/log.h>
#include <commons/string.h>
#include <unnamed/serialization.h>
#include <pthread.h>
#include <unnamed/server.h>

// enum COMANDOS{
//     OBTENER_BITACORA = 760 ,
//     MOVER_TRIPULANTE ,
//     COMIENZA_EJECUCION_TAREA,
//     FINALIZA_TAREA,
//     ATENDER_SABOTAJE,
//     RESUELTO_SABOTAJE,
//     RESPUESTA_BITACORA
// };

enum comandos {
    INICIAR_PATOTA = 500,
    RECIBIR_ESTADO_TRIPULANTE,
    RECIBIR_UBICACION_TRIPULANTE,
    ENVIAR_TAREA = 520,
    EXPULSAR_TRIPULANTE = 530,
    ELIMINAR_PATOTA = 599,
    SUCCESS = 200,
    ERROR_CANTIDAD_TRIPULANTES = 554,
    ERROR_POR_FALTA_DE_MEMORIA = 555,
    ERROR_GUARDAR_TCB = 556,
    ERROR_NO_HAY_TAREAS = 560,
    MENSAJE = 999
};

typedef struct {
    uint32_t pid;
    uint32_t tasks;
} pcb_t;

typedef struct {
    uint32_t tid;
    uint32_t pid;
    char status;
    uint32_t xpos;
    uint32_t ypos;
    uint32_t next;
} tcb_t;

int main(){

   
    t_log* logger = log_create("cliente.log","clienteTest", 1,  LOG_LEVEL_INFO);
    int socket = _connect("127.0.0.1", "5001", logger);

    log_info(logger, "INICIANDO PATOTA....");

    tcb_t *temp = malloc(sizeof(tcb_t));

    temp -> tid = 1;
    temp -> pid = 1;
    temp -> status = 'N';
    temp -> xpos = 3;
    temp -> ypos = 4;

    tcb_t *temp2 = malloc(sizeof(tcb_t));

    temp2 -> tid = 2;
    temp2 -> pid = 1;
    temp2 -> status = 'N';
    temp2 -> xpos = 3;
    temp2 -> ypos = 4;

    tcb_t *temp3 = malloc(sizeof(tcb_t));

    temp3 -> tid = 3;
    temp3 -> pid = 1;
    temp3 -> status = 'N';
    temp3 -> xpos = 3;
    temp3 -> ypos = 4;

    tcb_t *temp4 = malloc(sizeof(tcb_t));

    temp4 -> tid = 4;
    temp4 -> pid = 1;
    temp4 -> status = 'N';
    temp4 -> xpos = 3;
    temp4 -> ypos = 4;

    tcb_t *temp5 = malloc(sizeof(tcb_t));

    temp5 -> tid = 5;
    temp5 -> pid = 1;
    temp5 -> status = 'N';
    temp5 -> xpos = 3;
    temp5 -> ypos = 4;

    tcb_t *temp6 = malloc(sizeof(tcb_t));

    temp6 -> tid = 6;
    temp6 -> pid = 1;
    temp6 -> status = 'N';
    temp6 -> xpos = 3;
    temp6 -> ypos = 4;

    tcb_t *temp7 = malloc(sizeof(tcb_t));

    temp7 -> tid = 7;
    temp7 -> pid = 1;
    temp7 -> status = 'N';
    temp7 -> xpos = 3;
    temp7 -> ypos = 4;

    tcb_t *temp8 = malloc(sizeof(tcb_t));

    temp8 -> tid = 8;
    temp8 -> pid = 1;
    temp8 -> status = 'N';
    temp8 -> xpos = 3;
    temp8 -> ypos = 4;

    tcb_t *temp9 = malloc(sizeof(tcb_t));

    temp9 -> tid = 9;
    temp9 -> pid = 1;
    temp9 -> status = 'N';
    temp9 -> xpos = 3;
    temp9 -> ypos = 4;

    tcb_t *temp10 = malloc(sizeof(tcb_t));

    temp10 -> tid = 10;
    temp10 -> pid = 1;
    temp10 -> status = 'N';
    temp10 -> xpos = 3;
    temp10 -> ypos = 4;

    pcb_t *patota = malloc(sizeof(pcb_t));

    patota -> pid = 1;

    char *tareas = "GENERAR_OXIGENO 12;3;2;5CONSUMIR_OXIGENO 120;2;3;1";

    int total_size = (17 * 10) + sizeof(int) * 3 + strlen(tareas);

    void *buffer = _serialize(
        total_size,
        "%d%s%d%d%d%c%d%d%d%d%c%d%d%d%d%c%d%d%d%d%c%d%d%d%d%c%d%d%d%d%c%d%d%d%d%c%d%d%d%d%c%d%d%d%d%c%d%d%d%d%c%d%d",
        patota -> pid,
        tareas,
        10, // Cantidad tcbs
        temp -> tid,
        temp -> pid,
        temp -> status,
        temp -> xpos,
        temp -> ypos,

        temp2 -> tid,
        temp2 -> pid,
        temp2 -> status,
        temp2 -> xpos,
        temp2 -> ypos,

        temp3 -> tid,
        temp3 -> pid,
        temp3 -> status,
        temp3 -> xpos,
        temp3 -> ypos,

        temp4 -> tid,
        temp4 -> pid,
        temp4 -> status,
        temp4 -> xpos,
        temp4 -> ypos,

        temp5 -> tid,
        temp5 -> pid,
        temp5 -> status,
        temp5 -> xpos,
        temp5 -> ypos,

        temp6 -> tid,
        temp6 -> pid,
        temp6 -> status,
        temp6 -> xpos,
        temp6 -> ypos,

        temp7 -> tid,
        temp7 -> pid,
        temp7 -> status,
        temp7 -> xpos,
        temp7 -> ypos,

        temp8 -> tid,
        temp8 -> pid,
        temp8 -> status,
        temp8 -> xpos,
        temp8 -> ypos,

        temp9 -> tid,
        temp9 -> pid,
        temp9 -> status,
        temp9 -> xpos,
        temp9 -> ypos,

        temp10 -> tid,
        temp10 -> pid,
        temp10 -> status,
        temp10 -> xpos,
        temp10 -> ypos
    );

    _send_message(socket, "DIS",INICIAR_PATOTA, buffer, total_size , logger);

    t_mensaje *mensaje = _receive_message(socket, logger);

    log_info(logger, "RESPUESTA: %s - %s",
        mensaje -> identifier,
        mensaje -> command == 200 ? "Success" : "Error"
    );

    free(buffer);

    if (mensaje -> command == 200) {

        log_info(logger, "ELIMINANDO TRIPULANTE %d....", temp4 -> tid);

        total_size = sizeof(int) * 2;
        buffer = _serialize(total_size, "%d%d", patota -> pid, temp4 -> tid);

        _send_message(socket, "DIS",EXPULSAR_TRIPULANTE, buffer, total_size , logger);

        t_mensaje *mensaje2 = _receive_message(socket, logger);

        log_info(logger, "RESPUESTA: %s - %s",
            mensaje2 -> identifier,
            mensaje2 -> command == 200 ? "Success" : "Error"
        );

        free(buffer);

        log_info(logger, "ELIMINANDO TRIPULANTE %d....", temp5 -> tid);

        total_size = sizeof(int) * 2;
        buffer = _serialize(total_size, "%d%d", patota -> pid, temp5 -> tid);

        _send_message(socket, "DIS",EXPULSAR_TRIPULANTE, buffer, total_size , logger);

        t_mensaje *mensaje4 = _receive_message(socket, logger);

        log_info(logger, "RESPUESTA: %s - %s",
            mensaje4 -> identifier,
            mensaje4 -> command == 200 ? "Success" : "Error"
        );

        free(buffer);

        log_info(logger, "ELIMINANDO TRIPULANTE %d....", temp6 -> tid);

        total_size = sizeof(int) * 2;
        buffer = _serialize(total_size, "%d%d", patota -> pid, temp6 -> tid);

        _send_message(socket, "DIS",EXPULSAR_TRIPULANTE, buffer, total_size , logger);

        t_mensaje *mensaje5 = _receive_message(socket, logger);

        log_info(logger, "RESPUESTA: %s - %s",
            mensaje5 -> identifier,
            mensaje5 -> command == 200 ? "Success" : "Error"
        );

        free(buffer);

        log_info(logger, "ELIMINANDO PATOTA....");

        total_size = sizeof(int);
        buffer = _serialize(total_size, "%d", patota -> pid);

        _send_message(socket, "DIS",ELIMINAR_PATOTA, buffer, total_size , logger);

        t_mensaje *mensaje3 = _receive_message(socket, logger);

        log_info(logger, "RESPUESTA: %s - %s",
            mensaje3 -> identifier,
            mensaje3 -> command == 200 ? "Success" : "Error"
        );

        free(buffer);
    } else {
        log_error(logger, "No hay espacio en memoria.");
    }

    log_destroy(logger);

    return 0;
}