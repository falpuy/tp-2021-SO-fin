#include "main.h"

int main() {

    char *logger_path;
    t_log *logger;
    t_config * config = config_create(CONFIG_PATH);
    int socket_ds;
    char *ip_ds;
    char *port_ds;

    if (config_has_property(config, "ARCHIVO_LOG")) {
        logger_path = config_get_string_value(config, "ARCHIVO_LOG");
        logger = log_create(logger_path, PROGRAM, 1, LOG_LEVEL_TRACE); 

        log_info(logger, "(╯°o°)ᕗ I Mongo Store ejecutando correctamente..");

        ip_ds = config_get_string_value(config, "IP_DISCORD");
        port_ds = config_get_string_value(config, "PUERTO_DISCORD");

        socket_ds = _connect(ip_ds, port_ds, logger);
        
        if ( socket_ds < 0 ) {
            log_error(logger, "Finalizando proceso..");

            config_destroy(config);
            log_destroy(logger);

            exit(EXIT_FAILURE);
        } else {
            _send_message(socket_ds, PROGRAM, 999, "HOLA", 4, logger);
        }

    } else {
        perror("Error en archivo de configuracion..");
    }

    config_destroy(config);
    log_destroy(logger);

    return 0;
}