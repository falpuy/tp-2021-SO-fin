#include "atenderTripulantes.h"



void handler(int client, char* identificador, int comando, void* payload, t_log* logger){

    switch(comando){
        case MOVER_TRIPULANTE: // de X|Y --> X'|Y'
            //Escribir en Bitacora/TripulanteID.ims
            log_info(logger,"hola");
            log_info(logger,"%s",payload);

            break;
        
        case COMIENZA_EJECUCION_TAREA:
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

