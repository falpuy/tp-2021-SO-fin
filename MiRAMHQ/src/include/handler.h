#ifndef RAM_HANDLER_H
#define RAM_HANDLER_H

#include <stdio.h>
#include <commons/log.h>

enum comandos {
    INICIAR_TRIPULANTE = 500,
    RECIBIR_TAREAS_PATOTA = 510,
    RECIBIR_UBICACION_TRIPULANTE,
    ENVIAR_PROXIMA_TAREA = 520,
    EXPULSAR_TRIPULANTE = 530,
    MENSAJE = 999

};

void handler (int fd, char *id, int opcode, char *buffer, t_log *logger);

#endif