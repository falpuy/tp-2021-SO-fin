#ifndef RAM_HANDLER_H
#define RAM_HANDLER_H

#include <stdio.h>
#include <commons/log.h>
#include <string.h>
#include <unnamed/socket.h>
#include "memory.h"

enum comandos {
    INICIAR_TRIPULANTE = 500,
    RECIBIR_UBICACION_TRIPULANTE,
    ENVIAR_PROXIMA_TAREA = 520,
    EXPULSAR_TRIPULANTE = 530,
    SUCCESS = 200
    ERROR_GUARDAR_TCB = 556
    ERROR_POR_FALTA_DE_MEMORIA = 555
    MENSAJE = 999
};

void handler (int fd, char *id, int opcode, void *buffer, t_log *logger);

#endif