#include "headers/mainIMongoStore.h"

int main() {

    arch_config = config_create(CONFIG_PATH);
    logger = log_create("logMainIMS.log","IMS",1,LOG_LEVEL_INFO);


    setearConfiguraciones();


    //Checkear si FS existe y sino crearlo
    //Crear Servidor

    _select(datosConfig->puerto, handler,logger);


    finalizarProceso(logger, arch_config);

    return 0;
}

void setearConfiguraciones(){
    datosConfig = malloc(sizeof(configIMS));
    int tamanioString;

    if(_check_config(arch_config,keysConfig) ==0){
        log_error(logger, "No están todos los parametros en el archivo de configuracion");
    };

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
    log_destroy(logger);
    config_destroy(arch_config);


    free(datosConfig->puntoMontaje);
    free(datosConfig->puerto);
    free(datosConfig);
        
}

void handler(){
    printf("hola");
}