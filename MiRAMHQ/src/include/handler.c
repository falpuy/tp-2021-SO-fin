#include "./handler.h"

void handler(int fd, char *id, int opcode, char *buffer, t_log *logger) {
    log_info(logger, "Recibi la siguiente operacion de %s: %d", id, opcode);

    switch (opcode){
        case INICIAR_TRIPULANTE:
        case RECIBIR_TAREAS_PATOTA:
        case RECIBIR_UBICACION_TRIPULANTE:
        case ENVIAR_PROXIMA_TAREA:
        case EXPULSAR_TRIPULANTE:
            break;
        case MENSAJE:
            log_info(logger, "Message: %s", buffer);
            _send_message(fd, "RAM", 999, "ok", strlen("ok"), logger);
        break;
        
        default:
            log_info(logger, "No se pudo realizar la operacion %d", opcode);
            break;
    }
}