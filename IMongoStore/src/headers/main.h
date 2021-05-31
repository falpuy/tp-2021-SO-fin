#ifndef I_MONGO_STORE_H
#define I_MONGO_STORE_H

    #include <stdio.h>
    #include <stdlib.h>
    #include <unnamed/socket.h>
    #include <unnamed/server.h>
    #include <signal.h>
    #include <pthread.h>
    #include "syncFile.h"
    #include <signal.h>
    #include <sys/mman.h>

    #include "atenderTripulantes.h"
    
    #include <commons/log.h>
    #include <commons/config.h>
    #include <commons/bitarray.h>

    #define CONFIG_PATH "../configuraciones/configuracion.config"
    #define LOG_PATH "../logs/logIMS.log"
    #define PROGRAM "IMS"

    t_log* logIMS;
    t_config* arch_config;
    pthread_t threadSelect;

    void setearConfiguraciones();
    void finalizarProceso();
    char* path(char* str);


#endif