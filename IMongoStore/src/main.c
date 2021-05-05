#include "headers/main.h"

int main() {
    crearSuperBloqueTest();
    setearConfiguraciones();

    

    inicializacionFS(logIMS);
    
   //int puertoEscucha = _create_socket_listenner(datosConfig->config,log);
    finalizarProceso();
    return 0;
}


void setearConfiguraciones(){
    arch_config = config_create(CONFIG_PATH);
    logIMS = log_create(LOG_PATH,"IMS",1,LOG_LEVEL_INFO);
    datosConfig = malloc(sizeof(configIMS));
    int tamanioString;

    tamanioString =strlen(config_get_string_value(arch_config,"PUNTO_MONTAJE"));
    datosConfig->puntoMontaje = malloc(tamanioString+1);
    strncpy(datosConfig->puntoMontaje,config_get_string_value(arch_config,"PUNTO_MONTAJE"),tamanioString);
    datosConfig->puntoMontaje[tamanioString] = '\0';

    tamanioString=strlen(config_get_string_value(arch_config,"PUERTO"));
    datosConfig->puerto = malloc(tamanioString+1);
    strncpy(datosConfig->puerto,config_get_string_value(arch_config,"PUERTO"),tamanioString);
    datosConfig->puerto[tamanioString] = '\0';

    datosConfig->tiempoSincronizacion = config_get_int_value(arch_config,"TIEMPO_SINCRONIZACION");
}

void finalizarProceso(){
    log_destroy(logIMS);
    config_destroy(arch_config);

    free(datosConfig->puntoMontaje);
    free(datosConfig->puerto);
    free(datosConfig);

    //rmdir("/home/utnso/Escritorio/TP_Operativos/tp-2021-1c-Unnamed-Group/IMongoStore/Filesystem/Bitacoras");
    exit(EXIT_SUCCESS);
}