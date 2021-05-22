#include "./headers/atenderTripulantes.h"



void handler(int client, char* identificador, int comando, void* payload, t_log* logger){

log_info(logger,"LLegó nuevo comando: %d", comando);
    
    switch(comando){
        case MOVER_TRIPULANTE: // de X|Y --> X'|Y'
            
            //PAYLOAD:
            // int             int         int         int         int
            // idTripulante    posXVieja   posYvieja   posXNueva   posYNueva

            //DESEARILIZO

            int idTripulante, posXVieja, posYVieja, posXNueva, posYNueva;
            
            char* comando = string_new();
            string_append(&comando,"Se mueve de ");
            string_append(&comando,string_itoa(posXVieja));
            string_append(&comando,"|");
            string_append(&comando,string_itoa(posYVieja));
            string_append(&comando," a ");
            string_append(&comando,string_itoa(posXNueva));
            string_append(&comando,"'|");
            string_append(&comando,string_itoa(posYNueva));
            string_append(&comando,"' .");
            
    

            char* idTrip = string_new();
            strcpy(idTrip,string_itoa(idTripulante));
            //hacer funcion que concatene las cosas.

            if(access("../Filesystem/Bitacoras/Tripulante" + idTrip ,F_OK) < 0){
                log_info(logger,"Se crea archivo en Bitacora para este tripulante...")
                
                //crear bitacora como config
                //setear todo en 0
                //checkear bloques libres en bitmap
                //ir guardando bloques
                //actualizar la bitacora
                
            }else{
                //checkear que bloques estan libres por bitmap
                //ir guardando en cada bloque libre
                //actualizar la bitacora
            }


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
        //TAREA PARAMETROS;POS X;POS Y;TIEMPO
        //GENERAR_OXIGENOS 12; 3,2,7

        //logeo en bitacora
        //logeo en files

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
        
        case ATENDER_SABOTAJE: //hacer conjunto a sabotaje
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

