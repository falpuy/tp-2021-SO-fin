#include <stdio.h>
#include <unnamed/socket.h>
#include <commons/log.h>

int main() {

    printf("Enviando mensaje...");

    t_log *logger = log_create("../logs/test.log", "TEST", 1, LOG_LEVEL_TRACE); 

    int socket = _connect("127.0.0.1", "9000", logger);

    _send_message(socket, "TST", 999, "HOLA", 4, logger);

    log_destroy(logger);

    return 0;
}
