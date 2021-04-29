#include "main.h"

int main() {

    config = config_create(CONFIG_PATH);
    logger = log_create(ARCHIVO_LOG, PROGRAM, 1, LOG_LEVEL_TRACE);

    signal(SIGINT, liberar_memoria);

    if (!_check_config(config, KEYS)) {
        log_error(logger, "Error en archivo de configuracion..");
        liberar_memoria(0);
    }

    log_info(logger, "(╯°o°)ᕗ Mi RAM HQ ejecutando correctamente..");

    // Creo el mapa
    // create_map(logger);

    // Inicializo la memoria
    memory = memory_init(config_get_string_value(config, "TAMANIO_MEMORIA"));

    // Creo el server
    _select(config_get_string_value(config, "PUERTO"), handler, logger);

    return EXIT_SUCCESS;
}

void liberar_memoria(int num) {
  log_destroy(logger);
  config_destroy(config);

  exit(EXIT_FAILURE);
}