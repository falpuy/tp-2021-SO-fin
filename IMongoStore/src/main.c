#include "headers/main.h"

int main() {
    // pthread_t sync_blocks;

    setearConfiguraciones();
    //inicializacionFS(logIMS);

    // pthread_create(&sync_blocks,NULL,(void *) actualizarArchivo,logIMS);
    // pthread_join(sync_blocks,NULL);
    //_start_server(datosConfig->puerto,handler, logIMS);
   
    // finalizarProceso();
    return 0;
}


void setearConfiguraciones(){
    flagEnd = 1;
    arch_config = config_create(CONFIG_PATH);
    logIMS = log_create(LOG_PATH,"IMS",1,LOG_LEVEL_INFO);
    datosConfig = malloc(sizeof(configIMS));

    datosConfig->puntoMontaje = config_get_string_value(arch_config,"PUNTO_MONTAJE");
    datosConfig->puerto = config_get_string_value(arch_config,"PUERTO");
    datosConfig->tiempoSincronizacion = config_get_int_value(arch_config,"TIEMPO_SINCRONIZACION");
    datosConfig->tamanioBloque = config_get_int_value(arch_config,"TAMANIO_BLOQUE");
    datosConfig->cantidadBloques = config_get_int_value(arch_config,"CANTIDAD_BLOQUES");

    pthread_mutex_init(&mutexBlocks, NULL); 
    pthread_mutex_init(&mutexSuperBloque, NULL); 

}

void finalizarProceso(){
    flagEnd = 0;
    log_destroy(logIMS);
    config_destroy(arch_config);

    free(datosConfig->puntoMontaje);
    free(datosConfig->puerto);
    free(datosConfig);
    //free(arch_bloques);
    pthread_mutex_destroy(&mutexBlocks); 
    pthread_mutex_destroy(&mutexSuperBloque); 

    //rmdir("/home/utnso/Escritorio/TP_Operativos/tp-2021-1c-Unnamed-Group/IMongoStore/Filesystem/Bitacoras");
    exit(EXIT_SUCCESS);
}