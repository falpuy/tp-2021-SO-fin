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
    
    // pthread_create(&sync_blocks,NULL,(void*) actualizarArchivoBlocks, logger);
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
    printf("\n\n%s\n", datosConfig->puerto);
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
