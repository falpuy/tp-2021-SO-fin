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
    

    #define CONFIG_PATH "/home/utnso/Escritorio/TP/tp-2021-1c-Unnamed-Group/IMongoStore/configuraciones/ims.config"
    #define LOG_PATH "/home/utnso/Escritorio/TP/tp-2021-1c-Unnamed-Group/IMongoStore/logs/logIMongoStore.log"
    #define PROGRAM "IMS"

    void setearConfiguraciones();
    // void signal_handler(int sig_number);
    void finalizarProceso();
#endif