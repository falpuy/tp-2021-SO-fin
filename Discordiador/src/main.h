#ifndef DISCORD_LIB
#define DISCORD_LIB

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <commons/config.h>
#include <commons/log.h>
#include <unnamed/select.h>
#include "include/handler.h"

#define CONFIG_PATH "./src/settings.config"
#define PROGRAM "DIS" // Se utiliza para el protocolo

t_log *logger;
t_config * config;

void liberar_memoria(int num);

#endif