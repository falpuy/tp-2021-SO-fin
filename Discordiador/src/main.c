#include "headers/main.h"

int main () {
    int conexion_RAM;
    int conexion_IMS;
    char* ip_RAM;
    char* puerto_RAM;
    char* ip_IMS;
    char* puerto_IMS;
    int grado_multitarea;
    char* algoritmo;
    int quantum_RR;
    int duracion_sabotaje;
    int ciclo_CPU;

    t_log* logger;
    logger = log_create("consola.log", "consola", 1, LOG_LEVEL_INFO);

    t_config* config;
    config = config_create("archivo.config"); //Hay que ver cómo se llama el archivo.config cuando nos lo den
    ip_RAM = config_get_string_value(config, "IP_MI_RAM_HQ");
    puerto_RAM = config_get_string_value(config, "PUERTO_MI_RAM_HQ");
    ip_IMS = config_get_string_value(config, "IP_I_MONGO_STORE");
    puerto_IMS = config_get_string_value(config, "PUERTO_I_MONGO_STORE");
    grado_multitarea = config_get_int_value (config, "GRADO_MULTITAREA");
    algoritmo = config_get_string_value(config, "ALGORITMO");
    quantum_RR = config_get_int_value (config, "QUANTUM");
    duracion_sabotaje = config_get_int_value (config, "DURACION_SABOTAJE");
    ciclo_CPU = config_get_int_value (config, "RETARDO_CICLO_CPU");
    log_info(logger, "%s, %s, %s, %s", ip_RAM, puerto_RAM, ip_IMS, puerto_IMS);

    //conexion_RAM = _connect(ip_RAM, puerto_RAM, logger);
    //conexion_IMS = _connect(ip_IMS, puerto_IMS, logger);

    //funcionConsola (logger);
    return 0;
}


