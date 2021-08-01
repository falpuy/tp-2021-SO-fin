#ifndef MIRAMHQ_LIB
#define MIRAMHQ_LIB

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#include <commons/log.h>
#include <commons/process.h>
#include <unnamed/server.h>
#include <unnamed/validation.h>
#include "memory.h"

#define CONFIG_PATH "settings.config"
#define ARCHIVO_LOG "info.log"
#define ARCHIVO_LOG_MAP "map.log"
#define PROGRAM "RAM" // Se utiliza para el protocolo

char *KEYS[] = {
    "TAMANIO_MEMORIA",
    "ESQUEMA_MEMORIA",
    "TAMANIO_PAGINA",
    "TAMANIO_SWAP",
    "PATH_SWAP",
    "ALGORITMO_REEMPLAZO",
    "CRITERIO_SELECCION",
    "PUERTO"
};

void signal_handler(int sig_number);

// --------------------- HANDLERS ----------------------- //

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

void pagination_handler (int fd, char *id, int opcode, void *buffer, t_log *logger);
void segmentation_handler (int fd, char *id, int opcode, void *buffer, t_log *logger);

// --------------------- END HANDLERS ----------------------- //

void admin_destroyer(void *item);
void setup_pagination(void *memory, char *path, int page_size, int real_size, int v_size, t_log *logger);

#endif