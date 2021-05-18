#include <stdio.h>
#include <stdlib.h>
#include <commons/log.h>
#include <unnamed/socket.h>
#include <signal.h>
#include <pthread.h>

t_log *logger;
int validator = 1;

typedef struct {
    int socket;
    t_log *logger;
    void (*func)();    
} t_data;

t_data *connection;

pthread_t *_threads;

#define MAX_CON 5
#define STRING_COMMAND 999

void liberar_memoria(int num) {
  validator = 0;

    log_destroy(logger);
    // free(connection);

  exit(EXIT_FAILURE);
}

void _start_server(char *port, void (*callback)(), t_log *logger);
void _thread_function(t_data *connection);
t_data *_create_metadata(int new_socket, t_log *logger, void (* callback)());

void handler(int socket, char *id, int comando, void *buffer, t_log *logger) {
    log_info(logger, "HERE: %d, %s, %d", socket, id, comando);

}

int main() {

    logger = log_create("../logs/test.log", "TEST", 1, LOG_LEVEL_TRACE);

    signal(SIGINT, liberar_memoria);

    log_info(logger,"Trying to start the server...");

    _start_server("9000", handler, logger);

    return 0;
}

void _start_server(char *port, void (*callback)(), t_log *logger) {
    // _threads = malloc(_pool_size * sizeof(pthread_t));
    int addrlen, new_socket;
    struct sockaddr_in address;

    int lsocket = _create_socket_listenner(port, logger);
	
    // Try to specify maximum of n pending connections for the master socket
    if (_listen(lsocket, MAX_CON, logger) < 0) {
        log_error(logger, "[Shared Library]: Ocurrio un error en el listen \n (╯°o°）╯︵ ┻━┻");
        exit(EXIT_FAILURE);
    }

    addrlen = sizeof(address);

    while (validator) {
        if ((new_socket = accept(lsocket, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            log_error(logger, "[Shared Library]: Ocurrio un error al aceptar una conexion \n (╯°o°）╯︵ ┻━┻");
            exit(EXIT_FAILURE);
        }

        log_info(logger, "[Shared Library]: Nueva conexion ID: %d", new_socket);

        connection = _create_metadata(new_socket, logger, callback);

        log_info(connection -> logger, "Trying to create a new thread..");

        pthread_t socket_thread;
	    pthread_create(&socket_thread, NULL, (void *) _thread_function, connection);
	    pthread_detach(socket_thread);

    }

}

t_data *_create_metadata(int new_socket, t_log *logger, void (* callback)()) {
        t_data *connection = malloc(sizeof(t_data));
        connection -> socket = new_socket;
        connection -> logger = logger;
        connection -> func = callback;
        return connection;
}

void _thread_function(t_data *connection) {
    char *id;
    int comando, b_size;
    void *buffer;
    int canRun = 1;
    log_info(connection -> logger, "Starting new thread for connection handling..");
    while (canRun) {
        id = malloc(4);
        if (recv( connection -> socket, id, 3, 0) <= 0) {
            log_info(connection -> logger, "El cliente %d se desconecto.", connection -> socket);
            free(id);
            close (connection -> socket);
            free(connection);
            canRun = 0;
        } else {
            // Agrego un \0 al final
            id [3] = '\0';
            log_info(connection -> logger, "Mensaje recibido de: %s", id);
            
            /* -------------------------------- Recibi el id de proceso -------------------------------- */
                    
            // Obtengo el codigo de operacion
            if (recv( connection -> socket, &comando, sizeof(int), 0) > 0) {
                        
                // Obtengo el tamanio del buffer que contiene los datos del struct enviado
                if (recv( connection -> socket, &b_size, sizeof(int), 0) > 0) {
                                    
                    // SI EL COMANDO ES IGUAL A STRING_COMMAND SIGNIFICA QUE SE ENVIO UN STRING
                    // Creo el buffer para guardar los datos
                    buffer = malloc(comando == STRING_COMMAND ? b_size + 1 : b_size);
                    // Obtengo el buffer
                    if (recv( connection -> socket, buffer, b_size, 0) > 0) {

                        // If code 999, set \0
                        if(comando == STRING_COMMAND) {
                            memset(buffer + b_size, '\0', 1);
                        }

                        // Calling external function with the current file descriptor, process id, operation code and payload
                        connection -> func(connection -> socket, id, comando, buffer, logger);
                        free(buffer);
                        free(id);
                    }
                }
            }
        }
    }
}