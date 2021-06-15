#include "./headers/atenderTripulantes.h"

void handler(int client, char* identificador, int comando, void* payload, t_log* log){

    log_info(log,"LLegó nuevo comando:%d", comando);
    int idTripulante, posXv, posYv, posXn, posYn,tamTarea,parametro;
    int offset = 0;
    char* tarea;
        
        switch(comando){
            case OBTENER_BITACORA:    
                log_info(log,"-----------------------------------------------------");
                log_info(log, "Llego comando: Obtener bitácora de.....");

                memcpy(&idTripulante,payload,sizeof(int));
                log_info(log, "ID Tripulante:%d", idTripulante);
                log_info(log,"-----------------------------------------------------");
                //obtenerBitacora(idTripulante);
                break;  

            case MOVER_TRIPULANTE:
                log_info(log,"-----------------------------------------------------");
                log_info(log, "Llego comando: Mover Tripulante.....");

                memcpy(&idTripulante,payload,sizeof(int));
                memcpy(&posXv,payload + sizeof(int), sizeof(int));
                memcpy(&posYv,payload + sizeof(int)*2, sizeof(int));
                memcpy(&posXn,payload + sizeof(int)*3, sizeof(int));
                memcpy(&posYn,payload + sizeof(int)*4, sizeof(int));

                log_info(log, "ID Tripulante:%d", idTripulante);
                log_info(log, "Posicion Vieja en X:%d", posXv);
                log_info(log, "Posicion Vieja en Y:%d", posYv);
                log_info(log, "Posicion Nueva en X:%d", posXn);
                log_info(log, "Posicion Nueva en Y:%d", posYn);
                log_info(log,"-----------------------------------------------------");


                char* stringGuardar = strMoverTripultante(idTripulante,posXv,posYv,posXn,posYn);
                char* tripulante = crearStrTripulante(idTripulante);//Creo el path de tripulanteN.ims
                
                char* bitacora = string_new();
                string_append(&bitacora, "Bitacoras/");
                string_append(&bitacora,tripulante);

                char* path_fileTripulante = pathCompleto(bitacora);
                free(bitacora);
                
                pthread_mutex_lock(&m_metadata);
                if(access(path_fileTripulante,F_OK) < 0){
                    log_info(log,"No existe archivo en bitácora...Se crea archivo para este tripulante...");
                    crearMetadataBitacora(path_fileTripulante,log);
                }
                pthread_mutex_unlock(&m_metadata);

                pthread_mutex_lock(&m_blocks);
                //guardarEnBlocks(stringGuardar,path_fileTripulante,0,log);
                pthread_mutex_unlock(&m_blocks);
                
                free(tripulante);
                free(path_fileTripulante);
                free(stringGuardar);

            break;

            case COMIENZA_EJECUCION_TAREA: //IdTripulante TamTarea Tarea ParametrosTarea X Y T
                log_info(log,"-----------------------------------------------------");
                log_info(log, "Llego comando: Comienza ejecucion de tarea.....");

                memcpy(&idTripulante,payload,sizeof(int)); 
                memcpy(&tamTarea,payload+sizeof(int),sizeof(int)); 
                tarea = malloc(tamTarea+1);
                memcpy(tarea,payload+sizeof(int)*2,tamTarea);
                tarea[tamTarea]='\0';
                memcpy(&parametro,payload + sizeof(int)*2+ tamTarea,sizeof(int));


                log_info(log, "Tam tarea:%d", tamTarea);
                log_info(log, "Tarea:%s", tarea);
                log_info(log, "Parametro de tarea:%d", parametro);
                log_info(log,"-----------------------------------------------------");
                
                //ejecutarTarea(tamTarea,tarea,parametro,log);
                break;
            
            case FINALIZA_TAREA: 
                log_info(log,"-----------------------------------------------------");
                log_info(log, "Llego comando: Finaliza tarea.....");

                memcpy(&idTripulante,payload,sizeof(int)); 
                memcpy(&tamTarea,payload+sizeof(int),sizeof(int)); 
                tarea = malloc(tamTarea+1);
                memcpy(tarea,payload+sizeof(int)*2,tamTarea);
                tarea[tamTarea]='\0';
                memcpy(&parametro,payload + sizeof(int)*2+ tamTarea,sizeof(int));

                log_info(log, "Tarea:%s", tarea);
                log_info(log,"-----------------------------------------------------");
                //finalizaTarea(tamTarea,tarea,parametro,log);
                break;
            
            case ATENDER_SABOTAJE: 
                log_info(log,"-----------------------------------------------------");
                log_info(log, "Llego comando: Atender Sabotaje.....");

                memcpy(&idTripulante,payload,sizeof(int));
                log_info(log, "ID Tripulante:%d", idTripulante);
                log_info(log,"-----------------------------------------------------");
                //TO DO: funcionalidad

                break;
            
            case RESUELTO_SABOTAJE:
                log_info(log,"-----------------------------------------------------");
                log_info(log, "Llego comando:Se resolvio Sabotaje.....");
                
                memcpy(&idTripulante,payload,sizeof(int));
                log_info(log, "ID Tripulante:%d", idTripulante);
                log_info(log,"-----------------------------------------------------");
                //TO DO: funcionalidad

                break;
            default:
                log_error(log, "No se encontró comando");
                break;
    }
   
}
