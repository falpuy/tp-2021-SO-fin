#include "headers/main.h"

int main() {
    setearConfiguraciones();
    inicializacionFS(logger);

    signal(SIGINT, signal_handler);
    signal(SIGUSR1, signal_handler);

    log_info(logger, "Creando servidor......");
    sleep(1);
    
    pthread_create(&sync_blocks,NULL,(void*) actualizarArchivoBlocks, logger);
    printf("%s\n", datosConfig->puerto);
    _start_server(datosConfig->puerto,handler,logger);

    return 0;
}


void setearConfiguraciones(){
    flagEnd = 1;
    contador = 0;
    socketDiscordiador = 0;
    testeoIDTripulante = 0;

    config = config_create(CONFIG_PATH);
    logger = log_create(LOG_PATH,"IMS",1,LOG_LEVEL_INFO);

    datosConfig = malloc(sizeof(configIMS));
    datosConfig->puntoMontaje = config_get_string_value(config,"PUNTO_MONTAJE");
    datosConfig->puerto = config_get_string_value(config,"PUERTO");
    datosConfig->tiempoSincronizacion = config_get_int_value(config,"TIEMPO_SINCRONIZACION");
    datosConfig->posicionesSabotaje = config_get_array_value(config,"LISTA_POSICIONES");

    pthread_mutex_init(&blocks_bitmap, NULL); 
    pthread_mutex_init(&m_superBloque, NULL); 
    pthread_mutex_init(&m_metadata, NULL); 
    pthread_mutex_init(&discordiador, NULL); 

}


void signal_handler(int sig_number) {

  switch(sig_number) {
    case SIGINT:
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
        pthread_mutex_destroy(&discordiador);


        exit(EXIT_SUCCESS);
        break;
    case SIGUSR1:
        sabotaje();
        break;
    }

}