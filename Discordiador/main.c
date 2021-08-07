#include "headers/main.h"
#include <commons/process.h>

int main () {
    setearConfiguraciones();

    unsigned int pid = process_getpid();
    log_info(logger, "N° proceso: %d", pid);

    signal(SIGINT,liberarMemoria);

    funcionPlanificador(logger);

    pthread_create(&hEsperarSabotaje, NULL,(void*) servidor, parametros);
    pthread_detach(hEsperarSabotaje);
    
    funcionConsola();
    return 0;
}

void setearConfiguraciones (){

    logger = log_create("discordiadorPruebaDiscordiadorCPUFIFO.log", "discordiador", 1, LOG_LEVEL_INFO);
    config = config_create("archivo.config");
    
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

    lista_parametros = list_create();

    pthread_mutex_init(&mutex_cantidadVieja, NULL);
    pthread_mutex_init(&mutex_cantidadActual, NULL);
    pthread_mutex_init(&mutexValidador, NULL);
    pthread_mutex_init(&mutexPlanificacionViva, NULL);
    pthread_mutex_init(&mutexSabotajeActivado, NULL);
    pthread_mutex_init(&mutex_cantidadTCB, NULL);
    pthread_mutex_init(&mutex_contadorSemGlobal, NULL);
    pthread_mutex_init(&mutexValidacionPos, NULL);

    pthread_mutex_lock(&mutex_cantidadVieja);
    cantidadVieja = 0;
    pthread_mutex_unlock(&mutex_cantidadVieja);

    pthread_mutex_lock(&mutex_cantidadActual);
    cantidadActual = 0;
    pthread_mutex_unlock(&mutex_cantidadActual);

    contadorPCBs = 0;
    pthread_mutex_lock(&mutex_contadorSemGlobal);
    contadorSemGlobal = 0;
    pthread_mutex_unlock(&mutex_contadorSemGlobal);
    cantidadTCBTotales = 0;
    
    pthread_mutex_lock(&mutexValidador);
    validador = 1;
    pthread_mutex_unlock(&mutexValidador);

    pthread_mutex_lock(&mutexPlanificacionViva);
    planificacion_viva = 0;
    pthread_mutex_unlock(&mutexPlanificacionViva);

    pthread_mutex_lock(&mutexSabotajeActivado);
    sabotaje_activado = 0;
    pthread_mutex_unlock(&mutexSabotajeActivado);

    sabotaje_terminado = 0;
    contadorCicloCPU = 1;
    primerCicloSabotaje = 0;

    // conexion_RAM = _connect(ip_RAM, puerto_RAM, logger);
    // conexion_IMS = _connect(ip_IMS, puerto_IMS, logger);

    pthread_mutex_init(&mutexNew, NULL);
    pthread_mutex_init(&mutexReady, NULL);
    pthread_mutex_init(&mutexExec, NULL);
    pthread_mutex_init(&mutexBloqIO, NULL);
    pthread_mutex_init(&mutexBloqEmer, NULL);
    pthread_mutex_init(&mutexBloqEmerSorted, NULL);
    pthread_mutex_init(&mutexExit, NULL);
    pthread_mutex_init(&mutexCiclosTranscurridosSabotaje, NULL);
    pthread_mutex_init(&mutexListaPCB, NULL);
    pthread_mutex_init(&mutexBuffer,NULL);
    pthread_mutex_init(&mutexSemaforosTrip,NULL);
    pthread_mutex_init(&mutexContextoSabotaje,NULL);
    pthread_mutex_init(&mutexBuffersAEnviar,NULL);
    
    sem_init(&semNR, 0, 0);
    sem_init(&semRE, 0, 0);
    sem_init(&semER, 0, 0);
    sem_init(&semBLOCKIO, 0, 0);
    sem_init(&semEXIT, 0, 0);
    sem_init(&semEBIO, 0, 0);
    sem_init(&semEaX, 0, 0);
    sem_init(&semERM, 0, 0);
    sem_init(&semMR, 0, 0);
    sem_init(&pausar,0,0);
    
    pthread_create(&hNewaReady, NULL, (void *) funcionhNewaReady, logger);
    pthread_create(&hReadyaExec, NULL, (void *) funcionhReadyaExec, logger);
    pthread_create(&hExecaReady, NULL, (void *) funcionhExecaReady, logger);
    pthread_create(&hExecaBloqIO, NULL, (void *) funcionhExecaBloqIO, logger);
    pthread_create(&hExecaExit, NULL, (void *) funcionhExecaExit, logger);
    pthread_create(&hBloqIO, NULL, (void *) funcionhBloqIO, logger);
    pthread_create(&hExit, NULL, (void *) funcionhExit, logger);

    pthread_create(&hExecReadyaBloqEmer, NULL, (void *) funcionhExecReadyaBloqEmer, logger);
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
}

void servidor(){
    _start_server(puerto_DIS, handler, logger);
}

