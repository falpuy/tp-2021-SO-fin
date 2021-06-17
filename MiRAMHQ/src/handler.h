#ifndef RAM_HANDLER_H
#define RAM_HANDLER_H

#include <stdio.h>
#include <commons/log.h>
#include <string.h>
#include <unnamed/socket.h>
#include "memory.h"

enum comandos {
    INICIAR_TRIPULANTE = 500,
    RECIBIR_UBICACION_TRIPULANTE=510,
    ENVIAR_PROXIMA_TAREA = 520,
    ENVIAR_TAREA = 525,
    EXPULSAR_TRIPULANTE = 530,
    SUCCESS = 200,
    ERROR_CANTIDAD_TRIPULANTES = 554,
    ERROR_POR_FALTA_DE_MEMORIA = 555,
    ERROR_NO_HAY_TAREAS = 560,
    MENSAJE = 999
};

void handler (int fd, char *id, int opcode, void *buffer, t_log *logger);

#endif