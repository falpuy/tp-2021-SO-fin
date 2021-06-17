#ifndef MIRAMHQ_LIB
#define MIRAMHQ_LIB

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <commons/config.h>
#include <commons/log.h>
#include <unnamed/server.h>
#include <unnamed/validation.h>
#include "memory.h"
//#include "./include/level.h"

#define CONFIG_PATH "settings.config"
#define ARCHIVO_LOG "../logs/info.log"
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

t_dictionary *table_collection;

char *esquema;

t_log *logger;
t_config * config;

void *memory;
int mem_size;
int page_size;

void *virtual_memory;
int virtual_size;

t_bitarray *bitmap;
t_bitarray *virtual_bitmap;

int frames_memory;
int frames_virtual;

void signal_handler(int sig_number);

// --------------------- HANDLERS ----------------------- //

enum comandos {
    INICIAR_PATOTA = 500,
    RECIBIR_ESTADO_TRIPULANTE,
    RECIBIR_UBICACION_TRIPULANTE,
    ENVIAR_TAREA = 520,
    EXPULSAR_TRIPULANTE = 530,
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

void setup_pagination(void *memory, char *path, int page_size, int real_size, int v_size, t_log *logger);

#endif