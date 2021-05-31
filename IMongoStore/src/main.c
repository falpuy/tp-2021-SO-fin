#include "headers/main.h"

int main() {
    pthread_t sync_blocks;


    setearConfiguraciones();
    inicializacionFS(logIMS);
    signal(SIGINT,finalizarProceso);

    

    //pthread_create(&sync_blocks,NULL,(void *) actualizarArchivo,logIMS);
    //pthread_join(sync_blocks,NULL);

    log_info(logIMS, "------------------------------------------------");
    log_info(logIMS, "Se crea servidor......");
    sleep(1);
    _start_server(datosConfig->puerto,handler, logIMS);
    finalizarProceso();
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

    pthread_mutex_init(&mutexBlocks, NULL); 
    pthread_mutex_init(&mutexSuperBloque, NULL); 

}

void finalizarProceso(){
    flagEnd = 0;
    bitarray_destroy(bitmap);

    err = munmap(blocks_memory, (tamanioBloque * cantidadBloques));
    if (err == -1){
        log_error(logIMS, "[Blocks] Error al liberal la memoria mapeada de tamañoBloque y cantidadBloque");
    }
    log_destroy(logIMS);
    config_destroy(arch_config);
    free(datosConfig);

    pthread_mutex_destroy(&mutexBlocks); 
    pthread_mutex_destroy(&mutexSuperBloque); 

    exit(EXIT_SUCCESS);
}
