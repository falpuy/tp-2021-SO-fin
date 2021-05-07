#ifndef MIRAMHQ_LIB
#define MIRAMHQ_LIB

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <commons/config.h>
#include <commons/log.h>
#include <unnamed/server.h>
#include <unnamed/validation.h>
#include "handler.h"
//#include "./include/level.h"

#define CONFIG_PATH "settings.config"
#define ARCHIVO_LOG "info.log"
#define PROGRAM "RAM" // Se utiliza para el protocolo

char *KEYS[] = {
    "TAMANIO_MEMORIA",
    "ESQUEMA_MEMORIA",
    "TAMANIO_PAGINA",
    "TAMANIO_SWAP",
    "PATH_SWAP",
    "ALGORITMO_REEMPLAZO",
    "PUERTO"
};

// MEMORIA
void *memory;
void *virtualMemory;

// SEGMENTACION
int tasks_size;
t_list *segmentTable;

t_log *logger;
t_config * config;

void liberar_memoria(int num);

#endif