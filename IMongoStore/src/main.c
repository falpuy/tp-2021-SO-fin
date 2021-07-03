#include "headers/main.h"

int main() {
    setearConfiguraciones();
    inicializacionFS(logger);

    signal(SIGINT, finalizarProceso);
    signal(SIGUSR1, sabotaje);

    log_info(logger, "Creando servidor......");
    sleep(1);
    
    pthread_create(&sync_blocks,NULL,(void*) actualizarArchivoBlocks, logger);
    _start_server(datosConfig->puerto,handler,logger);

    return 0;
}


void setearConfiguraciones(){

    socketDiscordiador = 0;
    testeoIDTripulante = 0;

    config = config_create(CONFIG_PATH);
    logger = log_create(LOG_PATH,"IMS",1,LOG_LEVEL_INFO);

    datosConfig = malloc(sizeof(configIMS));
    datosConfig->puntoMontaje = config_get_string_value(config,"PUNTO_MONTAJE");
    datosConfig->puerto = config_get_string_value(config,"PUERTO");
    datosConfig->tiempoSincronizacion = config_get_int_value(config,"TIEMPO_SINCRONIZACION");
    // datosConfig->posicionesSabotaje = config_get_array_value(config,"POSICIONES_SABOTAJE");

    pthread_mutex_init(&blocks_bitmap, NULL); 
    pthread_mutex_init(&m_superBloque, NULL); 
    pthread_mutex_init(&m_metadata, NULL); 
    pthread_mutex_init(&discordiador, NULL); 
    pthread_mutex_init(&validador, NULL); 

    pthread_mutex_lock(&validador);
    flagEnd = 1;
    pthread_mutex_unlock(&validador);

}

void finalizarProceso(){
    pthread_mutex_lock(&validador);
    flagEnd = 0;
    pthread_mutex_unlock(&validador);
    pthread_join(sync_blocks,NULL);

    config_destroy(config);
    log_destroy(logger);

    free(copiaBlocks);
    free(copiaSB);
    bitarray_destroy(bitmap);
    free(memBitmap);
    free(datosConfig);

    pthread_mutex_destroy(&blocks_bitmap); 
    pthread_mutex_destroy(&m_superBloque); 
    pthread_mutex_destroy(&m_metadata);
    pthread_mutex_destroy(&discordiador);
    pthread_mutex_destroy(&validador);

    exit(EXIT_SUCCESS);
}






// void signal_handler(int sig_number) {

//   switch(sig_number) {
//     case SIGINT:
//         pthread_mutex_lock(&validador);
//         flagEnd = 0;
//         pthread_mutex_unlock(&validador);
        
//         pthread_join(sync_blocks,NULL);
        
//         config_destroy(config);
//         log_destroy(logger);

//         free(copiaBlocks);
//         free(copiaSB);
//         bitarray_destroy(bitmap);
//         free(memBitmap);
//         free(datosConfig);

//         pthread_mutex_destroy(&blocks_bitmap); 
//         pthread_mutex_destroy(&m_superBloque); 
//         pthread_mutex_destroy(&m_metadata);
//         pthread_mutex_destroy(&discordiador);
//         pthread_mutex_destroy(&validador);


//         exit(EXIT_SUCCESS);
//         break;
//     case SIGUSR1:
//         sabotaje();
//         break;
//     }

// }