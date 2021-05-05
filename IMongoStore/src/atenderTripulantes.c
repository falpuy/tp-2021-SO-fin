#include "./headers/atenderTripulantes.h"



void handler(int client, char* identificador, int comando, void* payload, t_log* logger){
log_info(logger,"comando: %d", comando);
    switch(comando){
        case MOVER_TRIPULANTE: // de X|Y --> X'|Y'
            //Escribir en Bitacora/TripulanteID.ims
            
            //deserializacion para testear payload="testearSerializacion"
            // int offset;
            // char*buffer1;

            // memcpy(&offset, payload, sizeof(int));
            // buffer1 = malloc(offset + 1);
            // memcpy(buffer1,payload + sizeof(int),offset);
            // buffer1[offset] = '\0';

            //log_info(logger,"%s",buffer1);

            break;
        
        case COMIENZA_EJECUCION_TAREA:
            log_info(logger,"buenas");
            //Escribir en Bitacora
            //Escribir en la tarea.ims
            log_info(logger,"%s",payload);
            break;
        
        case FINALIZA_TAREA: // idTarea
            //Escribir en Bitacora
            //Escribir en la tarea.ims
            log_info(logger,"%s",payload);

            break;
        
        case ATENDER_SABOTAJE:
            //Escribir en Bitacora
            //protocolo fsck?
            log_info(logger,"%s",payload);

            break;
        
        case RESUELTO_SABOTAJE:
            //escribir en bitacora
            log_info(logger,"%s",payload);

            break;
    }
   
}

