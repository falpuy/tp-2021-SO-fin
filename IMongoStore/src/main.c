#include "headers/main.h"

int main() {
    //t_log* log;

    setearConfiguraciones();
    inicializacionFS(logger);
    signal(SIGINT,finalizarProceso);


    log_info(logger, "------------------------------------------------");
    log_info(logger, "Creando servidor......");
    log_info(logger, "------------------------------------------------");
    sleep(1);
    
    pthread_create(&sync_blocks,NULL,(void*) actualizarArchivoBlocks, logger);
    //pthread_detach(sync_blocks);

    _start_server(datosConfig->puerto,handler,logger);
    finalizarProceso(log);
    return 0;
}


void setearConfiguraciones(){
    flagEnd = 1;
    config = config_create(CONFIG_PATH);
    logger = log_create(LOG_PATH,"IMS",1,LOG_LEVEL_INFO);

    datosConfig = malloc(sizeof(configIMS));
    datosConfig->puntoMontaje = config_get_string_value(config,"PUNTO_MONTAJE");
    datosConfig->puerto = config_get_string_value(config,"PUERTO");
    datosConfig->tiempoSincronizacion = config_get_int_value(config,"TIEMPO_SINCRONIZACION");

    pthread_mutex_init(&blocks_bitmap, NULL); 
    pthread_mutex_init(&m_superBloque, NULL); 
    pthread_mutex_init(&m_metadata, NULL); 
}

void finalizarProceso(){
    flagEnd = 0;
    pthread_join(sync_blocks,NULL);
    
    config_destroy(config);
    log_destroy(logger);

    // free(datosConfig->puntoMontaje);
    free(copiaBlocks);
    free(copiaSB);
    bitarray_destroy(bitmap);
    free(memBitmap);
    // free(datosConfig->puerto);
    free(datosConfig);


    pthread_mutex_destroy(&blocks_bitmap); 
    pthread_mutex_destroy(&m_superBloque); 
    pthread_mutex_destroy(&m_metadata);

    exit(EXIT_SUCCESS);
}

// void _start_server_ims(char *port, void (*callback)()) {
//     t_data *connection;

//     int addrlen, new_socket;
    
//     struct sockaddr_in address;

//     int lsocket = _create_socket_listenner(port, log);
	
//     // Try to specify maximum of n pending connections for the master socket
//     if (listen(lsocket, MAX_CON) < 0) {
//         log_error(log, "[Shared Library]: Ocurrio un error en el listen \n (╯°o°）╯︵ ┻━┻");
//         exit(EXIT_FAILURE);
//     }

//     log_info(log, "[Shared Library]: Esperando conexiones en puerto: %s", port);

//     addrlen = sizeof(address);

//     while (validator) {
//         if ((new_socket = accept(lsocket, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
//             log_error(log, "[Shared Library]: Ocurrio un error al aceptar una conexion \n (╯°o°）╯︵ ┻━┻");
//             exit(EXIT_FAILURE);
//         }

//         log_info(log, "[Shared Library]: Nueva conexion ID: %d", new_socket);

//         connection = _create_metadata(new_socket, log, callback);

//         pthread_t socket_thread;
// 	    pthread_create(&socket_thread, NULL, (void *) _thread_function, connection);
// 	    pthread_detach(socket_thread);

//     }

// }