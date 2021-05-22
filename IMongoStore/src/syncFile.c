#include "headers/syncFile.h"

void actualizarArchivo(t_log* log){

    while(flagEnd){
        sleep(datosConfig->tiempoSincronizacion);
        log_info(log, "Comienza actualización de Blocks.ims...");

        pthread_mutex_lock(&mutexBlocks);
        if(msync(mapArchivo,tamanioBloque*cantidadBloques,0)==0){
            log_info(log, "Se actualizo Blocks.ims");
        }else{
            log_error(log, "Error al actualizar Blocks.ims [MSYNC TIME]");
        }
       pthread_mutex_unlock(&mutexBlocks);
    }
}
