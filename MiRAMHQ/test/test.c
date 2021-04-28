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

// int main() {

//     printf("Enviando mensaje...");

//     t_log *logger = log_create("../logs/test.log", "TEST", 1, LOG_LEVEL_TRACE);

//     int socket_memoria =  _connect("127.0.0.1", "9000", logger);

//     _send_message(socket_memoria, "DIS", 999, "asdasdasdasd", strlen("asdasdasdasd"), logger);

//     t_mensaje *mensaje = _receive_message(socket_memoria, logger);

//     log_destroy(logger);

//     return 0;
// }

int main()
{
    // char str[] = "comando blabla [1,2,3,4,5] bla bla";
    // remove_all_chars(str, '[');
    // remove_all_chars(str, ']');
    void *str = malloc(strlen("STRING DE PRUEBBA\0"));

    memcpy(str, "STRING DE PRUEBBA\0", strlen("STRING DE PRUEBBA\0"));

    printf("str '%s'\n", str);
    
    int start = 3;
    int end = 6;

    memset(str + start, '\0', end - start);

    printf("str '%s'\n", str + end);

    for(int i = 0; i < strlen("STRING DE PRUEBBA\0"); i ++) {
        if (memcmp(str + i, "\0", 1)) {
            printf("CHAR: %s\n", (char*)&(str[i]));
        } else {
            printf("EMPTY: %s\n", (char*)&(str[i]));
        }
    }

    // Search for free spaces
    // for -> primero elemento vacio
    // for -> validar qe haya elementos vacios desde el primero q matchea
    // hhasta el tamanio necesario
    // si se encuentra una valor distinto de vacio en el medio de la segunda busqueda
    // colocar el primer for a partir de este valor y seguir buscando hhasta finalizar

    // Trim all values in memory
    // Use segment table to iterate the memory
    // Copy all values inside a new buffer next to each other
    // delete the memory buffer and set the new values

    free(str);
    return 0;
}