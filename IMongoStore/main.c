#include "headers/main.h"
#include <commons/process.h>

int main() {
    setearConfiguraciones();
    inicializacionFS(logger);
    
    unsigned int pid = process_getpid();
    log_info(logger,"-----------------------------------------------------");
    log_info(logger, "N° Proceso es:%d",pid);
    log_info(logger,"-----------------------------------------------------");

    signal(SIGINT, finalizarProceso);
    signal(SIGUSR1, sabotaje);

    log_info(logger, "Creando servidor......");
    
    pthread_create(&sync_blocks,NULL,(void*) actualizarArchivoBlocks, logger);
    // pthread_detach(sync_blocks);
    _start_server(datosConfig->puerto,handler,logger);

    return 0;
}


void setearConfiguraciones(){

    socketDiscordiador = 0;
    testeoIDTripulante = 0;
    contadorListaSabotajes = 0;
    contadorSabotajeLeido = 0;

    config = config_create(CONFIG_PATH);
    logger = log_create(LOG_PATH,"IMS",1,LOG_LEVEL_INFO);

    datosConfig = malloc(sizeof(configIMS));
    datosConfig->puntoMontaje = config_get_string_value(config,"PUNTO_MONTAJE");
    datosConfig->puerto = config_get_string_value(config,"PUERTO");
    datosConfig->tiempoSincronizacion = config_get_int_value(config,"TIEMPO_SINCRONIZACION");
    posicionesSabotajes = config_get_array_value(config,"POSICIONES_SABOTAJE");
    ipDiscordiador = config_get_string_value(config,"IP_DISCORDIADOR");
    puertoDiscordiador = config_get_string_value(config,"PUERTO_DISCORDIADOR");

    for(int i = 0; posicionesSabotajes[contadorListaSabotajes] != NULL; i++){
        contadorListaSabotajes++;
    }

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
    int contador_temp = 0;

    for(int i = 0; i <= contadorListaSabotajes; i++){
        free(posicionesSabotajes[contador_temp]);
        contador_temp++;
    }
    free(posicionesSabotajes);
    config_destroy(config);
    log_destroy(logger);

    free(copiaBlocks);
    // free(copiaSB);

    // bitarray_destroy(bitmap);
    // free(memBitmap);
    munmap(sb_memoria,sizeof(uint32_t) * 2 + cantidadBloques/8);
    
    free(datosConfig);

    pthread_mutex_destroy(&blocks_bitmap); 
    pthread_mutex_destroy(&m_superBloque); 
    pthread_mutex_destroy(&m_metadata);
    pthread_mutex_destroy(&discordiador);
    pthread_mutex_destroy(&validador);

    exit(EXIT_SUCCESS);
}