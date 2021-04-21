#ifndef MIRAMHQ_LIB
#define MIRAMHQ_LIB

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <commons/config.h>
#include <commons/log.h>
#include <unnamed/select.h>
#include <unnamed/validation.h>
#include "./include/handler.h"
#include "./include/level.h"

#define CONFIG_PATH "./src/settings.config"
#define ARCHIVO_LOG "./logs/info.log"
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

t_log *logger;
t_config * config;

void liberar_memoria(int num);

#endif