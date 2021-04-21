#ifndef I_MONGO_STORE_H
#define I_MONGO_STORE_H

    #include <stdio.h>
    #include <stdlib.h>
    #include <commons/config.h>
    #include <commons/log.h>
    #include <unnamed/socket.h>
    #include <unnamed/select.h>
    #include <unnamed/validation.h>
    #include <string.h>

    #define CONFIG_PATH "configuracion.config"
    #define PROGRAM "IMS" // Se utiliza para el protocolo

    typedef struct{
        char* puntoMontaje;
        char* puerto;
        int tiempoSincronizacion;
    }configIMS;

    char *keysConfig[] = {
         "PUNTO_MONTAJE",
         "PUERTO",
         "TIEMPO_SINCRONIZACION",
    };

    configIMS* datosConfig;

    t_config* arch_config;
    t_log* logger;

    void setearConfiguraciones();
    void finalizarProceso();
    void handler();




#endif