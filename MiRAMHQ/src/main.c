#include "main.h"

int main() {

    char *logger_path;
    t_log *logger;
    t_config * config = config_create(CONFIG_PATH);

    if (config_has_property(config, "ARCHIVO_LOG")) {
        logger_path = config_get_string_value(config, "ARCHIVO_LOG");
        logger = log_create(logger_path, PROGRAM, 1, LOG_LEVEL_TRACE); 

        log_info(logger, "(╯°o°)ᕗ Mi RAM HQ ejecutando correctamente..");
    } else {
        perror("Error en archivo de configuracion..");
    }

    config_destroy(config);
    log_destroy(logger);

    return 0;
}