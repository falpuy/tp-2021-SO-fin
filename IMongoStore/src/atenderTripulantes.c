#include "./headers/atenderTripulantes.h"

void handler(int client, char* identificador, int comando, void* payload, t_log* logger){

    log_info(logger,"LLegó nuevo comando: %d", comando);
        
        switch(comando){
            case OBTENER_BITACORA: //760
                log_info(logger,"-----------------------------------------------------");
                log_info(logger, "Llego comando: Obtener bitácora de.....");

                int o_idTripulante;
                memcpy(&o_idTripulante,payload,sizeof(int));
                log_info(logger, "ID Tripulante:         %d", o_idTripulante);
                log_info(logger,"-----------------------------------------------------");

                break;  

            case MOVER_TRIPULANTE:
                log_info(logger,"-----------------------------------------------------");
                log_info(logger, "Llego comando: Mover Tripulante.....");
                
                int idTripulante,posX_v,posY_v, posX_n, posY_n;

                //SEPARO LO QUE ME MANDA
                memcpy(&idTripulante,payload,sizeof(int));
                memcpy(&posX_v,payload + sizeof(int), sizeof(int));
                memcpy(&posY_v,payload + sizeof(int)*2, sizeof(int));
                memcpy(&posX_n,payload + sizeof(int)*3, sizeof(int));
                memcpy(&posY_n,payload + sizeof(int)*4, sizeof(int));

                log_info(logger, "ID Tripulante:         %d", idTripulante);
                log_info(logger, "Posicion Vieja en X:   %d", posX_v);
                log_info(logger, "Posicion Vieja en Y:   %d", posY_v);
                log_info(logger, "Posicion Nueva en X:   %d", posX_n);
                log_info(logger, "Posicion Nueva en Y:   %d", posY_n);
                log_info(logger,"-----------------------------------------------------");


                char* stringGuardar = strMoverTripultante(idTripulante,posX_v,posY_v,posX_n,posY_n);
                
                //Creo el path de tripulanteN.ims
                char* tripulante = crearStrTripulante(idTripulante);
                
                char* bitacora = string_new();
                string_append(&bitacora, "Bitacoras/");
                string_append(&bitacora,tripulante);

                char* path_fileTripulante = pathCompleto(bitacora);
                free(bitacora);
                
                if(access(path_fileTripulante,F_OK) < 0){
                    log_info(logger,"No existe archivo en bitácora...\tSe crea archivo para este tripulante...");
                    crearMetadataBitacora(path_fileTripulante,logger);
                }
                guardarEnBlocks(stringGuardar,path_fileTripulante,0,logger);
                free(tripulante);
                free(path_fileTripulante);
                free(stringGuardar);

            break;

            case COMIENZA_EJECUCION_TAREA: //IdTripulante TamTarea Tarea ParametrosTarea X Y T
                log_info(logger,"-----------------------------------------------------");
                log_info(logger, "Llego comando: Comienza ejecucion de tarea.....");

                int c_tamanioTarea,c_posX,c_posY,c_tiempo,c_idTripulante,c_parametros;
                int c_offset = 0;
                char* c_tarea;

                memcpy(&c_idTripulante,payload,sizeof(int)); //idTripulante
                c_offset +=sizeof(int);
                memcpy(&c_tamanioTarea,payload+c_offset,sizeof(int)); //tareaLen
                c_offset += sizeof(int);
                c_tarea = malloc(c_tamanioTarea+1);
                memcpy(c_tarea,payload + c_offset,c_tamanioTarea);
                c_tarea[c_tamanioTarea] = '\0';
                c_offset+= c_tamanioTarea;
                memcpy(&c_parametros,payload + c_offset, sizeof(int));//parametros
                c_offset += sizeof(int);
                memcpy(&c_posX,payload + c_offset, sizeof(int));//pos x
                c_offset += sizeof(int);
                memcpy(&c_posY,payload + c_offset, sizeof(int)); //pos y
                c_offset += sizeof(int);
                memcpy(&c_tiempo,payload + c_offset, sizeof(int)); //tiempo
                
                log_info(logger, "ID Tripulante:\t%d", c_idTripulante);
                log_info(logger, "Tam tarea:\t%d", c_tamanioTarea);
                log_info(logger, "Tarea:\t%s", c_tarea);
                log_info(logger, "Parametros tarea:\t%d", c_parametros);
                log_info(logger, "Posicion en X:\t%d", c_posX);
                log_info(logger, "Posicion en Y:\t%d", c_posY);
                log_info(logger, "Tiempo:\t%d", c_tiempo);
                log_info(logger,"-----------------------------------------------------");
                
                //ejecutarTarea(c_tamanioTarea,c_tarea,c_parametros,logger);
                break;
            
            case FINALIZA_TAREA: // 
                log_info(logger,"-----------------------------------------------------");
                log_info(logger, "Llego comando: Finaliza tarea.....");

                int f_tamTarea;
                int f_offset = 0;
                char* f_tarea;

                //SEPARO LO QUE ME MANDA
                memcpy(&f_tamTarea,payload,sizeof(int));
                f_offset += sizeof(int);
                
                f_tarea = malloc(f_tamTarea);
                memcpy(f_tarea,payload + sizeof(int),f_tamTarea);
                f_tarea[f_tamTarea] = '\0';
                f_offset+= f_tamTarea;

                log_info(logger, "Tarea:         %s", f_tarea);
                log_info(logger,"-----------------------------------------------------");
                //finalizaTarea(c_tamanioTarea,c_tarea,c_parametros,logger);


                break;
            
            case ATENDER_SABOTAJE:  //idTripulante
                log_info(logger,"-----------------------------------------------------");
                log_info(logger, "Llego comando: Atender Sabotaje.....");

                int a_idTripulante;
                memcpy(&a_idTripulante,payload,sizeof(int));
                log_info(logger, "ID Tripulante:         %d", a_idTripulante);
                log_info(logger,"-----------------------------------------------------");

                break;
            
            case RESUELTO_SABOTAJE: //idTripulante
                log_info(logger,"-----------------------------------------------------");
                log_info(logger, "Llego comando:Se resolvio Sabotaje.....");
                
                int r_idTripulante;
                memcpy(&r_idTripulante,payload,sizeof(int));
                log_info(logger, "ID Tripulante:         %d", r_idTripulante);
                log_info(logger,"-----------------------------------------------------");
                break;
            default:
                log_error(logger, "No se encontró comando");
                break;
    }
   
}
