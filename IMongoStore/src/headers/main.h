#ifndef I_MONGO_STORE_H
#define I_MONGO_STORE_H

    #include <stdio.h>
    #include <stdlib.h>
    #include <unnamed/socket.h>
    #include <unnamed/server.h>
    #include <signal.h>
    #include <pthread.h>
    #include <signal.h>
    #include <sys/mman.h>
    #include <commons/log.h>
    #include <commons/config.h>
    #include <commons/bitarray.h>
    #include <pthread.h>

    #include "preparacionFS.h"
    

    #define CONFIG_PATH "../configuraciones/configuracion.config"
    #define LOG_PATH "../logs/logIMongoStore.log"
    #define PROGRAM "IMS"

    void setearConfiguraciones();
    void finalizarProceso();
    //void _start_server_ims(char* port,void (*callback)());


#endif