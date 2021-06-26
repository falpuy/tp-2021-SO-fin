#include "headers/main.h"

int main () {
    setearConfiguraciones();
    
    funcionPlanificador(logger);

    //Declaro el struct de los parametros para handler.
    parametrosServer parametros = malloc(sizeof(parametrosServer));
    parametros->puerto_DIS = string_new();
    string_append(&parametros->puertoDiscordiador,puerto_DIS);
    parametros->loggerDiscordiador = logger;


    pthread_create(&hEsperarSabotaje, NULL, servidor, parametros);
    pthread_detach(hEsperarSabotaje);

    funcionConsola (logger, conexion_RAM, conexion_IMS);

    return 0;
}

void setearConfiguraciones (){

    logger = log_create("consola.log", "consola", 1, LOG_LEVEL_INFO);
    config = config_create("archivo.config"); //Hay que ver cómo se llama el archivo.config cuando nos lo den
    
    ip_RAM = config_get_string_value(config, "IP_MI_RAM_HQ");
    puerto_RAM = config_get_string_value(config, "PUERTO_MI_RAM_HQ");
    ip_IMS = config_get_string_value(config, "IP_I_MONGO_STORE");
    puerto_IMS = config_get_string_value(config, "PUERTO_I_MONGO_STORE");
    grado_multitarea = config_get_int_value (config, "GRADO_MULTITAREA");
    algoritmo = config_get_string_value(config, "ALGORITMO");
    quantum_RR = config_get_int_value (config, "QUANTUM");
    duracion_sabotaje = config_get_int_value (config, "DURACION_SABOTAJE");
    ciclo_CPU = config_get_int_value (config, "RETARDO_CICLO_CPU");
    puerto_DIS = config_get_string_value(config, "PUERTO_DISCORDIADOR");


    /*log_info(logger, "%s, %s, %s, %s, %d, %s, %d, %d, %d", ip_RAM, puerto_RAM, ip_IMS, 
    puerto_IMS, grado_multitarea, algoritmo, quantum_RR, duracion_sabotaje, ciclo_CPU);*/


    conexion_RAM = _connect(ip_RAM, puerto_RAM, logger);
    conexion_IMS = _connect(ip_IMS, puerto_IMS, logger);

    pthread_mutex_init(&mutexNew, NULL);
    pthread_mutex_init(&mutexReady, NULL);
    pthread_mutex_init(&mutexExec, NULL);
    pthread_mutex_init(&mutexBloqIO, NULL);
    pthread_mutex_init(&mutexBloqEmer, NULL);
    pthread_mutex_init(&mutexExit, NULL);

    sem_init(&semNR, 0, 1);
    sem_init(&semRE, 0, 0);
    sem_init(&semER, 0, 0);
    sem_init(&semBLOCKIO, 0, 0);
    sem_init(&semEXIT, 0, 0);
    sem_init(&semEBIO, 0, 0);
    sem_init(&semEaX, 0, 0);
    
    semTripulantes = malloc(sizeof(sem_t)*cantidadActual);
    for(int i=cantidadVieja; i<cantidadActual; i++){
        sem_init(&semTripulantes[i], 0, 0);
    }
    
    pthread_create(&hNewaReady, NULL, (void *) funcionhNewaReady, logger);
    pthread_detach(hNewaReady);

    pthread_create(&hReadyaExec, NULL, (void *) funcionhReadyaExec, logger);
    pthread_detach(hReadyaExec);

    pthread_create(&hExecaReady, NULL, (void *) funcionhExecaReady, logger);
    pthread_detach(hExecaReady);
    
    pthread_create(&hExecaBloqIO, NULL, (void *) funcionhExecaBloqIO, logger);
    pthread_detach(hExecaBloqIO);
    
    pthread_create(&hExecaExit, NULL, (void *) funcionhExecaExit, logger);
    pthread_detach(hExecaExit);

    pthread_create(&hBloqIO, NULL, (void *) funcionhBloqIO, logger);
    pthread_detach(hBloqIO);

    pthread_create(&hExit, NULL, (void *) funcionhExit, logger);
    pthread_detach(hExit);

    pthread_create(&hExecReadyaBloqEmer, NULL, (void *) funcionhExecReadyaBloqEmer, logger);
    pthread_detach(hExecReadyaBloqEmer);

    pthread_create(&hBloqEmeraReady, NULL, (void *) funcionhBloqEmeraReady, logger);
    pthread_detach(hBloqEmeraReady);

    char* bufferAEnviar = string_new();
    string_append(&bufferAEnviar, "Aviso de inicio Discordiador");
    _send_message(conexion_IMS, "DIS", INICIO_DISCORDIADOR, bufferAEnviar, strlen(bufferAEnviar), logger);
    free(bufferAEnviar);
}