#include "main.h"

int main() {

    // Inicializando
   
    config = config_create(CONFIG_PATH);
    logger = log_create(ARCHIVO_LOG, PROGRAM, 1, LOG_LEVEL_TRACE);
   
    signal(SIGINT, liberar_memoria);
    //

    //
    if (!_check_config(config, KEYS)) {
        log_error(logger, "Error en el archivo de configuracion..");
        liberar_memoria(0);
    }

    log_info(logger, "Mi RAM HQ ejecutando correctamente..");
    //

    // Inicializo la memoria
    memory = memory_init(config_get_string_value(config, "TAMANIO_MEMORIA"));

    // Creo el mapa
    // create_map(logger);

    // Creo el server
    _select(config_get_string_value(config, "PUERTO"), handler, logger);

    return EXIT_SUCCESS;
}

void liberar_memoria(int num) {
  log_destroy(logger);
  config_destroy(config); 

 