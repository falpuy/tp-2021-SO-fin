#ifndef I_MONGO_STORE_H
#define I_MONGO_STORE_H

    #include <stdio.h>
    #include <stdlib.h>
    #include <unnamed/socket.h>
    #include <unnamed/select.h>
    #include <signal.h>
    //#include "../headers/preparacionFS.h"
    #include <pthread.h>
    #include "atenderTripulantes.h"
    #include <commons/config.h>

    #define CONFIG_PATH "./configuraciones/configuracion.config"
    #define PROGRAM "IMS"

    typedef struct{
        char* puntoMontaje;
        char* puerto;
        int tiempoSincronizacion;
    }configIMS;

    configIMS* datosConfig;

    t_config* arch_config;
    t_log* logger;
    pthread_t threadSelect;

    void setearConfiguraciones();
    void finalizarProceso();





#endif