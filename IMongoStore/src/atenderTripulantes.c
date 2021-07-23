#include "./headers/atenderTripulantes.h"


void handler(int client, char* identificador, int comando, void* payload, t_log* logger){

    log_info(logger,"LLegó nuevo comando:%d", comando);
    int idTripulante, posXv, posYv, posXn, posYn,tamTarea,parametro;
    char* tarea;
    char* bitacora;
    char* strGuardar;
    char* path_fileTripulante;
    char* tripulante;

    switch(comando){
        case ESPERANDO_SABOTAJE:
            log_info(logger,"-----------------------------------------------------");
            log_info(logger, "Se ha conectado por primera vez Discordiador");
            log_info(logger, "Guardo socket por si llega un sabotaje");

            pthread_mutex_lock(&discordiador);
            socketDiscordiador = client;
            pthread_mutex_unlock(&discordiador);

            log_info(logger,"-----------------------------------------------------");
        break;

        case ATENDER_SABOTAJE:
            log_info(logger,"-----------------------------------------------------");
            log_info(logger, "Llego comando: Atiende sabotaje");
            
            memcpy(&idTripulante,payload,sizeof(int));
            log_info(logger, "ID Tripulante:%d", idTripulante);

            strGuardar = string_new();
            string_append(&strGuardar, "Se corre en pánico hacia la Ubicación del Sabotaje|");

            tripulante = crearStrTripulante(idTripulante);

            bitacora = string_new();
            string_append(&bitacora, "Bitacoras/");
            string_append(&bitacora,tripulante);

            path_fileTripulante = pathCompleto(bitacora);
            free(tripulante);
            free(bitacora);

            pthread_mutex_lock(&blocks_bitmap);
            if(access(path_fileTripulante,F_OK) < 0){
                log_info(logger,"No existe archivo en bitácora...Se crea archivo para este tripulante...");
                crearMetadataBitacora(path_fileTripulante);
            }
            guardarEnBlocks(strGuardar,path_fileTripulante,0);
            pthread_mutex_unlock(&blocks_bitmap);

            free(tripulante);
            free(path_fileTripulante);
            free(strGuardar);
            log_info(logger,"-----------------------------------------------------");


        break;
        case INVOCAR_FSK:
            log_info(logger,"-----------------------------------------------------");
            log_info(logger, "Llego comando: Invocar Protocolo FSK");

            pthread_mutex_lock(&blocks_bitmap);
            protocolo_fsck();
            pthread_mutex_unlock(&blocks_bitmap);
        break;

        case OBTENER_BITACORA:    
            log_info(logger,"-----------------------------------------------------");
            log_info(logger, "Llego comando: Obtener bitácora de.....");

            memcpy(&idTripulante,payload,sizeof(int));
            log_info(logger, "ID Tripulante:%d", idTripulante);
            pthread_mutex_lock(&blocks_bitmap);
            bitacora = obtenerBitacora(idTripulante);
            pthread_mutex_unlock(&blocks_bitmap);

            int tamBitacora = string_length(bitacora);
            char* tempBitacora = malloc(tamBitacora);
            
            memcpy(tempBitacora, bitacora, tamBitacora - 1);
            tempBitacora[tamBitacora-1] = '\0';

            //Falta enviar que no existe bitacora

            void* buffer = _serialize(sizeof(int)+ (tamBitacora),"%s",tempBitacora);
            _send_message(client, "IMS",RESPUESTA_OBTENER_BITACORA, buffer,sizeof(int)+ tamBitacora, logger);

            log_info(logger,"-----------------------------------------------------");
            free(buffer);
            free(tempBitacora);
            free(bitacora);
            break;  

        case MOVER_TRIPULANTE:
            log_info(logger,"-----------------------------------------------------");
            log_info(logger, "Llego comando: Mover Tripulante.....");

            memcpy(&idTripulante,payload,sizeof(int));
            memcpy(&posXv,payload + sizeof(int), sizeof(int));
            memcpy(&posYv,payload + sizeof(int)*2, sizeof(int));
            memcpy(&posXn,payload + sizeof(int)*3, sizeof(int));
            memcpy(&posYn,payload + sizeof(int)*4, sizeof(int));

            log_info(logger, "ID Tripulante:%d", idTripulante);
            log_info(logger, "Posicion Vieja en X:%d", posXv);
            log_info(logger, "Posicion Vieja en Y:%d", posYv);
            log_info(logger, "Posicion Nueva en X:%d", posXn);
            log_info(logger, "Posicion Nueva en Y:%d", posYn);
            log_info(logger,"-----------------------------------------------------");


            strGuardar = strMoverTripultante(idTripulante,posXv,posYv,posXn,posYn);
            string_append(&strGuardar,"|");
            
            char* tripulante = crearStrTripulante(idTripulante);//Creo el path de tripulanteN.ims

            bitacora = string_new();
            string_append(&bitacora, "Bitacoras/");
            string_append(&bitacora,tripulante);

            path_fileTripulante = pathCompleto(bitacora);
            free(bitacora);


            pthread_mutex_lock(&blocks_bitmap);
            if(access(path_fileTripulante,F_OK) < 0){
                log_info(logger,"No existe archivo en bitácora...Se crea archivo para este tripulante...");
                crearMetadataBitacora(path_fileTripulante);
            }
            guardarEnBlocks(strGuardar,path_fileTripulante,0);
            pthread_mutex_unlock(&blocks_bitmap);

            free(tripulante);
            free(path_fileTripulante);
            free(strGuardar);

        break;

        case COMIENZA_EJECUCION_TAREA: //IdTripulante TamTarea Tarea ParametrosTarea X Y T
            log_info(logger,"-----------------------------------------------------");
            log_info(logger, "Llego comando: Comienza ejecucion de tarea.....");

            memcpy(&idTripulante,payload,sizeof(int)); 
            memcpy(&tamTarea,payload+sizeof(int),sizeof(int)); 
            tarea = malloc(tamTarea+1);
            memcpy(tarea,payload+sizeof(int)*2,tamTarea);
            tarea[tamTarea]='\0';

            log_info(logger, "Tam tarea:%d", tamTarea);
            log_info(logger, "Tarea:%s", tarea);

            if(comandoTarea(tarea) == 7){
                comienzaEjecutarTarea(tamTarea,tarea,-1,idTripulante);
            }else{
                memcpy(&parametro,payload + sizeof(int)*2+ tamTarea,sizeof(int));
                log_info(logger, "Parametro de tarea:%d", parametro);
                comienzaEjecutarTarea(tamTarea,tarea,parametro,idTripulante);
            }
            log_info(logger,"-----------------------------------------------------");

            free(tarea);
            break;


        case FINALIZA_TAREA: 
            log_info(logger,"-----------------------------------------------------");
            log_info(logger, "Llego comando: Finaliza tarea....."); 

            memcpy(&idTripulante,payload,sizeof(int)); 
            memcpy(&tamTarea,payload+sizeof(int),sizeof(int)); 
            tarea = malloc(tamTarea+1);
            memcpy(tarea,payload+sizeof(int)*2,tamTarea);
            tarea[tamTarea]='\0';

            log_info(logger, "Tarea:%s", tarea);

            finalizaEjecutarTarea(tamTarea,tarea,idTripulante);
            free(tarea);
            log_info(logger,"-----------------------------------------------------");

            break;
        case RESUELTO_SABOTAJE:
            log_info(logger,"-----------------------------------------------------");
            log_info(logger, "Llego comando:Se resolvio Sabotaje.....");

            memcpy(&idTripulante,payload,sizeof(int));
            log_info(logger, "ID Tripulante:%d", idTripulante);
            log_info(logger,"-----------------------------------------------------");

            pthread_mutex_lock(&blocks_bitmap);
            // sabotajeResuelto(idTripulante);
            pthread_mutex_unlock(&blocks_bitmap);
            break;
        default:
            log_error(logger, "No se encontró comando");
            break;
    }

}