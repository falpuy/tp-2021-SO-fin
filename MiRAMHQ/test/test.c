#include <stdio.h>
#include <stdlib.h>
#include <unnamed/socket.h>
#include <unnamed/serialization.h>
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

    t_log *logger = log_create("../logs/test.log", "TEST", 1, LOG_LEVEL_TRACE);


    // ---------------- TEST ARCHIVOS ----------------- //


    FILE * fp;
    char * line = NULL;
    size_t len = 0;
    ssize_t read;

    void *buffer;
    int b_size = 0;
    int offset = 0;
    int new_size;
    void *temp;

    fp = fopen("./tareas.txt", "r");
    if (fp == NULL)
        exit(EXIT_FAILURE);

    while ((read = getline(&line, &len, fp)) != -1) {

        // printf("Length: %d - String: %s", read, line);

        if (line[ read - 1 ] == '\n') {
            read--;
            memset(line + read, 0, 1);
        }

        new_size = sizeof(int) + read;
        
        temp = _serialize(new_size, "%s", line);

        if (!b_size) {

            b_size = new_size;
            buffer = malloc(b_size);
        } else {

            b_size += new_size;
            buffer = realloc(buffer, b_size);
        }
        
        memcpy(buffer + offset, temp, new_size);
        offset += new_size;

        free(temp);
    }

    fclose(fp);
    if (line)
        free(line);

    // Agregar id de pcb al que pertenece
    // _send(buffer, offset);

    // Prueba Deserializacion

    char *tarea;
    int size_tarea;
    int off = 0;
    int i = 0;

    while (off < offset) {

        memcpy(&size_tarea, buffer + off, sizeof(int));
        off += sizeof(int);

        tarea = malloc(size_tarea + 1);
        memcpy(tarea, buffer + off, size_tarea);
        off += size_tarea;
        tarea[size_tarea] = '\0';

        log_info(logger, "Tarea %d - len %d: %s", i++, size_tarea, tarea);
        free(tarea);

    }

    free(buffer);


    // -------------- TEST SERIALIZACION -------------- //
    

    // void *buffer;
    // int b_size;

    // char *str = "HOLA!!!";
    // b_size = sizeof(int) + strlen(str);
    // buffer = _serialize(b_size, "%s", str);

    // char *str2 = "CHAU!!!";
    // int b_size2 = sizeof(int) + strlen(str2);
    // buffer = realloc(buffer, b_size + b_size2);
    // void * temp = _serialize(b_size2, "%s", str2);
    // memcpy(buffer + b_size, temp, b_size2);
    

    // log_info(logger, "Deserializando...");

    // char *otroBuffer;
    // int offset = 0;
    // int size;

    // memcpy(&size, buffer + offset, sizeof(int));
    // offset += sizeof(int);
    // log_info(logger, "Size primer string: %d", size);

    // otroBuffer = malloc(size + 1);
    // memcpy(otroBuffer, buffer + offset, size);
    // offset += size;
    // otroBuffer[offset] = '\0';

    // log_info(logger, "String: %s", otroBuffer);
    // free(otroBuffer);

    // memcpy(&size, buffer + offset, sizeof(int));
    // log_info(logger, "Size 2 string: %d - %d|%d", size, offset, b_size);
    // offset += sizeof(int);

    // otroBuffer = malloc(size + 1);
    // memcpy(otroBuffer, buffer + offset, size);
    // offset += size;
    // otroBuffer[offset] = '\0';

    // log_info(logger, "String 2: %s", otroBuffer);
    // free(otroBuffer);

    // free (temp);
    // free(buffer);
    
    //  ------------------------- TEST CASES

    // void *buffer;
    // int b_size;

    // char *str = "HOLA!!!";
    // b_size = sizeof(int) + strlen(str);
    // buffer = _serialize(b_size, "%s", str);
    // free(buffer);

    // b_size = sizeof(int);
    // buffer = _serialize(b_size, "%d", 25);
    // free(buffer);

    // b_size = sizeof(char);
    // buffer = _serialize(b_size, "%c", 'Z');
    // free(buffer);

    // b_size = sizeof(double);
    // buffer = _serialize(b_size, "%f", 100.33494);
    // free(buffer);

    // b_size = sizeof(uint32_t);
    // buffer = _serialize(b_size, "%u", 32);
    // free(buffer);

    // // Error de formato
    // b_size = sizeof(uint32_t);
    // buffer = _serialize(b_size, "%d%s%p%k", 32);
    // free(buffer);

    

    // ---------------- TEST CONEXION ----------------- //

    // int socket_memoria =  _connect("127.0.0.1", "5001", logger);

    // _send_message(socket_memoria, "DIS", 999, "asdasdasdasd", strlen("asdasdasdasd"), logger);

    // t_mensaje *mensaje = _receive_message(socket_memoria, logger);

    // free(mensaje -> identifier);
    // free(mensaje -> payload);
    // free(mensaje);

    // close(socket_memoria);

    // -------------------------------------------- //

    log_destroy(logger);

    return 0;
}

// int main()
// {
//     // char str[] = "comando blabla [1,2,3,4,5] bla bla";
//     // remove_all_chars(str, '[');
//     // remove_all_chars(str, ']');
//     void *str = malloc(strlen("STRING DE PRUEBBA\0"));

//     memcpy(str, "STRING DE PRUEBBA\0", strlen("STRING DE PRUEBBA\0"));

//     printf("str '%s'\n", str);
    
//     int start = 3;
//     int end = 6;

//     // Delete space allocated by data in hq memory
//     memset(str + start, '\0', end - start);

//     printf("str '%s'\n", str + end);

//     for(int i = 0; i < strlen("STRING DE PRUEBBA\0"); i ++) {
//         if (memcmp(str + i, "\0", 1)) {
//             printf("CHAR: %s\n", (char*)&(str[i]));
//         } else {
//             printf("EMPTY: %s\n", (char*)&(str[i]));
//         }
//     }

//     // Search for free spaces
//     // for -> primero elemento vacio
//     // for -> validar qe haya elementos vacios desde el primero q matchea
//     // hhasta el tamanio necesario
//     // si se encuentra una valor distinto de vacio en el medio de la segunda busqueda
//     // colocar el primer for a partir de este valor y seguir buscando hhasta finalizar

//     // Trim all values in memory
//     // Use segment table to iterate the memory
//     // Copy all values inside a new buffer next to each other
//     // delete the memory buffer and set the new values

//     free(str);
//     return 0;
// }