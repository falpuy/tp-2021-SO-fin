#include "main.h"

int main() {

    char *logger_path;
    config = config_create(CONFIG_PATH);

    // Esta funcion se llama al presionar ctrl+c en la terminal.
    // Permite eliminar con la funcion "liberar_memoria" la memoria de los procesos que no finalizan correctamente
    signal(SIGINT, liberar_memoria);

    if (config_has_property(config, "ARCHIVO_LOG")) {
        logger_path = config_get_string_value(config, "ARCHIVO_LOG");
        logger = log_create(logger_path, PROGRAM, 1, LOG_LEVEL_TRACE); 

        log_info(logger, "(╯°o°)ᕗ Proceso ejecutado correctamente..");
        _select("9000", handler, logger);
        
    } else {
        perror("Error en archivo de configuracion..");
    }

    config_destroy(config);
    log_destroy(logger);

    return EXIT_SUCCESS;
}

void liberar_memoria(int num) {
  log_destroy(logger);
  config_destroy(config);
  exit(EXIT_FAILURE);
}