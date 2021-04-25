#include <stdio.h>
#include <unnamed/socket.h>
#include <commons/log.h>

int status = 1;
int recvCounter = 0;

// void *handler(int socket) {
//   char *proceso;

//   log_info(logger, "Esperando mensajes..");
//   while(status) {
//     while (recvCounter--) {
//         proceso = malloc(3);
//         if (recv(socket, proceso, 3, 0) > 0) {
//             proceso[3] = '\0';
//         }
//     }
//   }
// }

int main() {

    printf("Enviando mensaje...");

    t_log *logger = log_create("../logs/test.log", "TEST", 1, LOG_LEVEL_TRACE);

    int socket_memoria =  _connect("127.0.0.1", "9000", logger);

    _send_message(socket_memoria, "DIS", 999, "asdasdasdasd", strlen("asdasdasdasd"), logger);

    t_mensaje *mensaje = _receive_message(socket_memoria, logger);

    log_destroy(logger);

    return 0;
}
