#include<stdio.h>
#include<stdlib.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include<unnamed/server.h>
#include<unnamed/socket.h>

int main ()
{
    t_log* logger;
    logger = log_create("test.log", "test", 1, LOG_LEVEL_INFO);
    
    t_config* config;
    config = config_create("test.config");
    char* puerto_RAM;
    puerto_RAM = config_get_string_value(config, "PUERTO");
    int conexion_RAM;
    char* ip = "127.0.0.1";
    conexion_RAM = _connect(ip, puerto_RAM, logger);

    return 0;
}