#include "headers/main.h"

int main() {
    pthread_t sync_blocks;
    //t_log* log;

    setearConfiguraciones();
    inicializacionFS(log);
    signal(SIGINT,finalizarProceso);


    log_info(log, "------------------------------------------------");
    log_info(log, "Creando servidor......");
    log_info(log, "------------------------------------------------");
    sleep(1);
    
    pthread_create(&sync_blocks,NULL,(void*) actualizarArchivoBlocks, log);
    pthread_detach(sync_blocks);

    _start_server(datosConfig->puerto,handler, log);
    finalizarProceso(log);
    return 0;
}


void setearConfiguraciones(){
    flagEnd = 1;
    config = config_create(CONFIG_PATH);
    log = log_create(LOG_PATH,"IMS",1,LOG_LEVEL_INFO);

    datosConfig = malloc(sizeof(configIMS));
    datosConfig->puntoMontaje = config_get_string_value(config,"PUNTO_MONTAJE");
    datosConfig->puerto = config_get_string_value(config,"PUERTO");
    datosConfig->tiempoSincronizacion = config_get_int_value(config,"TIEMPO_SINCRONIZACION");

    pthread_mutex_init(&m_blocks, NULL); 
    pthread_mutex_init(&m_superBloque, NULL); 
    pthread_mutex_init(&m_bitmap, NULL);
    pthread_mutex_init(&m_metadata, NULL); 
}

void finalizarProceso(){
    flagEnd = 0;

    free(copiaBlocks);
    free(copiaSB);
    free(memBitmap);
    free(datosConfig);
    bitarray_destroy(bitmap);

    log_destroy(log);
    config_destroy(config);

    pthread_mutex_destroy(&m_blocks); 
    pthread_mutex_destroy(&m_superBloque); 
    pthread_mutex_destroy(&m_bitmap); 
    pthread_mutex_destroy(&m_metadata);
    exit(EXIT_SUCCESS);
}
