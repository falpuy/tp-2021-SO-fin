#include "headers/main.h"

int main () {
    setearConfiguraciones();
    signal(SIGINT,liberarMemoria);
    funcionPlanificador(logger);
    // pthread_create(&hEsperarSabotaje, NULL,(void*) servidor, parametros);
    // pthread_detach(hEsperarSabotaje);
    //

    funcionConsola();

    return 0;
}

void setearConfiguraciones (){

    logger = log_create("discordiador.log", "discordiador", 1, LOG_LEVEL_INFO);
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


    log_info(logger, "%s, %s, %s, %s, %d, %s, %d, %d, %d", ip_RAM, puerto_RAM, ip_IMS, 
    puerto_IMS, grado_multitarea, algoritmo, quantum_RR, duracion_sabotaje, ciclo_CPU);

    cantidadVieja = 0;
    cantidadActual = 0;
    contadorPCBs = 0;
    contadorSemGlobal = 0;
    cantidadTCBTotales = 0;
    
    validador = 1;
    planificacion_viva = 1;
    sabotaje_activado = 0;

    conexion_RAM = _connect(ip_RAM, puerto_RAM, logger);
    conexion_IMS = _connect(ip_IMS, puerto_IMS, logger);

    pthread_mutex_init(&mutexNew, NULL);
    pthread_mutex_init(&mutexReady, NULL);
    pthread_mutex_init(&mutexExec, NULL);
    pthread_mutex_init(&mutexBloqIO, NULL);
    pthread_mutex_init(&mutexBloqEmer, NULL);
    pthread_mutex_init(&mutexBloqEmerSorted, NULL);
    pthread_mutex_init(&mutexExit, NULL);
    pthread_mutex_init(&mutexValidador, NULL);
    pthread_mutex_init(&mutexListaPCB, NULL);
    pthread_mutex_init(&mutex_cantidadVieja,NULL);
    pthread_mutex_init(&mutex_cantidadActual,NULL);

    sem_init(&semNR, 0, 0);
    sem_init(&semRE, 0, 0);
    sem_init(&semER, 0, 0);
    sem_init(&semBLOCKIO, 0, 0);
    sem_init(&semEXIT, 0, 0);
    sem_init(&semEBIO, 0, 0);
    sem_init(&semEaX, 0, 0);
    sem_init(&semERM, 0, 0);
    sem_init(&semMR, 0, 0);
    sem_init(&semFMR, 0, 0);
    
    pthread_create(&hNewaReady, NULL, (void *) funcionhNewaReady, logger);
    pthread_create(&hReadyaExec, NULL, (void *) funcionhReadyaExec, logger);
    pthread_create(&hExecaReady, NULL, (void *) funcionhExecaReady, logger);
    pthread_create(&hExecaBloqIO, NULL, (void *) funcionhExecaBloqIO, logger);
    pthread_create(&hExecaExit, NULL, (void *) funcionhExecaExit, logger);
    pthread_create(&hBloqIO, NULL, (void *) funcionhBloqIO, logger);
    pthread_create(&hExit, NULL, (void *) funcionhExit, logger);

    pthread_create(&hExecReadyaBloqEmer, NULL, (void *) funcionhExecReadyaBloqEmer, logger);
    pthread_create(&hFixerdeEmeraReady, NULL, (void *) funcionhFixerdeEmeraReady, logger);
    pthread_create(&hBloqEmeraReady, NULL, (void *) funcionhBloqEmeraReady, logger);

    pthread_detach(hNewaReady);
    pthread_detach(hReadyaExec);
    pthread_detach(hExecaReady);
    pthread_detach(hExecaBloqIO);
    pthread_detach(hExecaExit);
    pthread_detach(hBloqIO);
    pthread_detach(hExit);
    pthread_detach(hExecReadyaBloqEmer);
    pthread_detach(hBloqEmeraReady);
    pthread_detach(hFixerdeEmeraReady);

    // char* bufferAEnviar = string_new();
    // string_append(&bufferAEnviar, "Aviso de inicio Discordiador");
    // _send_message(conexion_IMS, "DIS", INICIO_DISCORDIADOR, bufferAEnviar, strlen(bufferAEnviar), logger);
    // free(bufferAEnviar);
}

void servidor(parametrosServer* parametros){
    _start_server(parametros->puertoDiscordiador, handler, parametros->loggerDiscordiador);
}

