#include "./headers/atenderTripulantes.h"

void handler(int client, char* identificador, int comando, void* payload, t_log* logger){

    log_info(logger,"LLegó nuevo comando:%d", comando);
    int idTripulante, posXv, posYv, posXn, posYn,tamTarea,parametro;
    int offset = 0;
    char* tarea;
        
        switch(comando){
            case OBTENER_BITACORA:    
                log_info(logger,"-----------------------------------------------------");
                log_info(logger, "Llego comando: Obtener bitácora de.....");

                memcpy(&idTripulante,payload,sizeof(int));
                log_info(logger, "ID Tripulante:%d", idTripulante);
                log_info(logger,"-----------------------------------------------------");
                //obtenerBitacora(idTripulante);
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


                char* stringGuardar = strMoverTripultante(idTripulante,posXv,posYv,posXn,posYn);
                char* tripulante = crearStrTripulante(idTripulante);//Creo el path de tripulanteN.ims
                
                char* bitacora = string_new();
                string_append(&bitacora, "Bitacoras/");
                string_append(&bitacora,tripulante);

                char* path_fileTripulante = pathCompleto(bitacora);
                free(bitacora);
                
                pthread_mutex_lock(&m_metadata);
                if(access(path_fileTripulante,F_OK) < 0){
                    log_info(logger,"No existe archivo en bitácora...Se crea archivo para este tripulante...");
                    crearMetadataBitacora(path_fileTripulante,logger);
                }
                pthread_mutex_unlock(&m_metadata);

                pthread_mutex_lock(&m_blocks);
                //guardarEnBlocks(stringGuardar,path_fileTripulante,0,logger);
                pthread_mutex_unlock(&m_blocks);
                
                free(tripulante);
                free(path_fileTripulante);
                free(stringGuardar);

            break;

            case COMIENZA_EJECUCION_TAREA: //IdTripulante TamTarea Tarea ParametrosTarea X Y T
                log_info(logger,"-----------------------------------------------------");
                log_info(logger, "Llego comando: Comienza ejecucion de tarea.....");

                memcpy(&idTripulante,payload,sizeof(int)); 
                memcpy(&tamTarea,payload+sizeof(int),sizeof(int)); 
                tarea = malloc(tamTarea+1);
                memcpy(tarea,payload+sizeof(int)*2,tamTarea);
                tarea[tamTarea]='\0';
                memcpy(&parametro,payload + sizeof(int)*2+ tamTarea,sizeof(int));


                log_info(logger, "Tam tarea:%d", tamTarea);
                log_info(logger, "Tarea:%s", tarea);
                log_info(logger, "Parametro de tarea:%d", parametro);
                log_info(logger,"-----------------------------------------------------");
                
                //ejecutarTarea(tamTarea,tarea,parametro,logger);
                break;
            
            case FINALIZA_TAREA: 
                log_info(logger,"-----------------------------------------------------");
                log_info(logger, "Llego comando: Finaliza tarea.....");

                memcpy(&idTripulante,payload,sizeof(int)); 
                memcpy(&tamTarea,payload+sizeof(int),sizeof(int)); 
                tarea = malloc(tamTarea+1);
                memcpy(tarea,payload+sizeof(int)*2,tamTarea);
                tarea[tamTarea]='\0';
                memcpy(&parametro,payload + sizeof(int)*2+ tamTarea,sizeof(int));

                log_info(logger, "Tarea:%s", tarea);
                log_info(logger,"-----------------------------------------------------");
                //finalizaTarea(tamTarea,tarea,parametro,logger);
                break;
            
            case ATENDER_SABOTAJE: 
                log_info(logger,"-----------------------------------------------------");
                log_info(logger, "Llego comando: Atender Sabotaje.....");

                memcpy(&idTripulante,payload,sizeof(int));
                log_info(logger, "ID Tripulante:%d", idTripulante);
                log_info(logger,"-----------------------------------------------------");
                //TO DO: funcionalidad

                break;
            
            case RESUELTO_SABOTAJE:
                log_info(logger,"-----------------------------------------------------");
                log_info(logger, "Llego comando:Se resolvio Sabotaje.....");
                
                memcpy(&idTripulante,payload,sizeof(int));
                log_info(logger, "ID Tripulante:%d", idTripulante);
                log_info(logger,"-----------------------------------------------------");
                //TO DO: funcionalidad

                break;
            default:
                log_error(logger, "No se encontró comando");
                break;
    }
   
}
