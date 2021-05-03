#include "main.h"

int main() {

    arch_config = config_create(CONFIG_PATH);
    logger = log_create("./logs/logIMS.log","IMS",1,LOG_LEVEL_INFO);
    
    signal(SIGINT, finalizarProceso);
    //signal(SIGUSR1, avisarDiscordiador;

    
    setearConfiguraciones(logger, arch_config);
    //crearEstructurasFS(logger);
    _select(datosConfig->puerto, handler,logger);

    return 0;
}


void setearConfiguraciones(t_log* logger, t_config* arch_config){
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



void finalizarProceso(int val){
    log_destroy(logger);
    config_destroy(arch_config);

    free(datosConfig->puntoMontaje);
    free(datosConfig->puerto);
    free(datosConfig);

    exit(EXIT_FAILURE);
}