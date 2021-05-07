#ifndef RAM_HANDLER_H
#define RAM_HANDLER_H

#include <stdio.h>
#include <commons/log.h>
#include <string.h>
#include <unnamed/socket.h>
#include "memory.h"

enum comandos {
    INICIAR_TRIPULANTE = 500,
    RECIBIR_TAREAS_PATOTA = 510,
    RECIBIR_UBICACION_TRIPULANTE,
    ENVIAR_PROXIMA_TAREA = 520,
    EXPULSAR_TRIPULANTE = 530,
    MENSAJE = 999
};

//pcb para testear porque no se que es void* tasks
typedef struct {
    int idPat;
    char* tripulantes[];
}falsoPCB;

void handler (int fd, char *id, int opcode, void *buffer, t_log *logger);

#endif