#include "./handler.h"

void handler(int fd, char *id, int opcode, void *buffer, t_log *logger) {
    log_info(logger, "Recibi la siguiente operacion de %s: %d", id, opcode);
		
  	char* data_tareas;
    int cantTripulantes, tamStrTareas,idTCB,posX,posY,idPCB;
  
  	int segment_size;
  	int found_segment;
		int offset = 0;	
  
  	int size_a_guardar;
  
    switch (opcode){

        case INICIAR_PATOTA: // idPCB - tareas - cantTCB - IDTCB.... (N id)
        		log_info(logger,"-----------------------------------------------------");
            log_info(logger,"Llegó la operación: INICIAR_PATOTA ");
      			
        	//--------------------------Deserializar -------------------------------              
        		memcpy(&idPCB, buffer, sizeof(int);
            offset += sizeof(int);
        
        		memcpy(&tamStrTareas,buffer,sizeof(int));
            offset += sizeof(int);
            data_tareas = malloc(tamStrTareas + 1);
        
        		memcpy(data_tareas, buffer + offset, tamStrTareas);
        		offset += tamSrtTareas;
            data_tareas[tamStrTareas]='\0';
        
        		memcpy(&cantTripulantes, buffer + offset, sizeof(int));
        		offset += sizeof(int);
                   
            log_info(logger,"ID PCB: %d", idPCB);
						log_info(logger,"Tareas de la patota: %s", data_tareas);
          	log_info(logger,"Cantidad de tripulantes: %d", cantTripulantes);
            log_info(logger,"Posicion en Y: %d", posY);
						//--------------------------------------------------------------------        
        		
          	char* respuesta = string_new();
   					string_append(&respuesta, "Respuesta");
						
            if(cantTripulantes <= 0){
              	log_error(logger, "Error: La cantidad de tripulantes es nula o no existente");
                _send_message(fd, "RAM", ERROR_CANTIDAD_TRIPULANTES , respuesta, string_length(respuesta), logger);
            } else {
             		size_a_guardar = sizeof(pcb) + tamStrTareas + cantTripulantes * sizeof(tcb);
            
                if (check_space_memory(size_a_guardar)) {
                  	// creo tabla de segmentos
                  	t_queue *segmentTable = queue_create();
										// crear segmento para tareas
                  	segment_size = tamStrTareas;
                    found_segment = memory_seek(memory, m_size, table_collection, segment_size);
                  	
                  		if(found_segment < 0) {
                          memory_compaction(memory, m_size, table_collection);
                          found_segment = memory_seek(memory, m_size, table_collection, segment_size);
                      }
                  
                  		pcb temp = malloc(sizeof(pcb));
                    	temp -> pid = idPCB;
                      temp -> tasks = found_segment;
                    	
                    	// guardo tareas
                    	segment *tareas = malloc(sizeof(segment));
                    
                    	tareas -> nroSegmento = get_last_index (segmentTable) + 1;
                      tareas -> baseAddr = found_segment;
                      tareas -> limit = found_segment + tamStrTareas;
                      tareas -> id = idPCB;
                      tareas -> type = TASK;
                    	
                    	save_task_in_memory(memory, mem_size, tareas, data_tareas);

                    	// creo segmento pcb
                    	segment *segmento_pcb = malloc(sizeof(segment));
                    
                      segment_size = sizeof(pcb);
                      found_segment = memory_seek(memory, m_size, table_collection, segment_size);
                  
                  		if(found_segment < 0) {
                          memory_compaction(memory, m_size, table_collection);
                          found_segment = memory_seek(memory, m_size, table_collection, segment_size);
                      }
                    
                    	segmento_pcb -> nroSegmento = get_last_index (segmentTable) + 1;
                      segmento_pcb -> baseAddr = found_segment;
                      segmento_pcb -> limit = found_segment + sizeof(pcb);
                      segmento_pcb -> id = idPCB;
                      segmento_pcb -> type = PCB;
                    	
                    	save_task_in_memory(memory, mem_size, tareas, data_tareas);
                    
                    	// guardo los segmentos
                    	queue_push(segmentTable, segmento_pcb);
                    	queue_push(segmentTable, tareas);
                    
                    	// guardo los tcbs
                    	for(int i = 0; i < cantTripulantes; i++) {
                      
                      	tcb *aux = malloc(sizeof(tcb));
                        
                        memcpy(&aux -> tid, buffer + offset, sizeof(int));
                        memcpy(&aux -> pid, buffer + offset, sizeof(int));
                        memcpy(&aux -> status, buffer + offset, sizeof(c));
                        memcpy(&aux -> xPos, buffer + offset, sizeof(int));
                        memcpy(&aux -> yPos, buffer + offset, sizeof(int));
                        
                        aux -> next = temp -> tasks;
                        
                        segment_size = sizeof(tcb);
                        found_segment = memory_seek(memory, m_size, table_collection, segment_size);
                        
                        if(found_segment < 0) {
                            memory_compaction(memory, m_size, table_collection);
                            found_segment = memory_seek(memory, m_size, table_collection, segment_size);
                        }
                        
                        segment *segmento_tcb = malloc(sizeof(segment));
                        segmento_tcb -> id = aux -> tid;
                        segmento_tcb -> type = TCB;
                        segmento_tcb -> nroSegmento = get_last_index (segmentTable) + 1;
                        segmento_tcb -> baseAddr = found_segment
                        segmento_tcb -> limit = found_segment + sizeof(tcb);
                        
                        queue_push(segmentTable, segmento_tcb);
                      }
                  	
                  	// guardo la tabla en el diccionario
                  	dictionary_put(table_collection, string_itoa(temp -> pid), segmentTable);
                  
                  	log_info(logger, "Se guardaron exitosamente los datos de la patota"); 
                  	_send_message(fd, "RAM", SUCCESS ,respuesta, string_length(respuesta), logger);
                  
                } else {
            				log_error(logger, "No hay mas espacio en memoria");
                  	_send_message(fd, "RAM", ERROR_POR_FALTA_DE_MEMORIA ,respuesta, string_length(respuesta), logger);
            		}
      
            }
            log_info(logger,"-----------------------------------------------------");
            free(data_tareas);
            free(temp);      
            free(respuesta); 
              
        break;
                
        

        case RECIBIR_UBICACION_TRIPULANTE: //ID_PATOTA, ID_TCB, POS_X, POS_Y
          	log_info(logger,"-----------------------------------------------------");
            log_info(logger,"Llegó la operación: RECIBIR_UBICACION_TRIPULANTE");
        //------------Deserializo parámetros-------------------
            memcpy(&idPCB, payload,sizeof(int));
            offset += sizeof(int);
            
            memcpy(&idTCB, payload + offset, sizeof(int));
            offset += sizeof(int);
            
						memcpy(&posX, payload + offset, sizeof(int));
            offset += sizeof(int);
              
						memcpy(&posY, payload + offset, sizeof(int));
              
            log_info(logger,"ID PCB: %d", idPCB);
						log_info(logger,"ID TCB: %d", idTCB);
          	log_info(logger,"Posicion en X: %d", posX);
            log_info(logger,"Posicion en Y: %d", posY);
        //----------------------------------------------------
						t_queue* segmento_pcb = dictionary_get(diccionario, idPCB); 
            segment* segmento_tcb = get_tcb_by_id(segmento_pcb, idTCB);

            tcb* nuestroTCB = get_tcb_from_memory(memory, m_size, segmento_tcb);

            int error = save_tcb_in_memory(&memory, m_size, segmento_tcb, nuestroTCB);
					
            char* respuesta = string_new();
            string_append(&respuesta, "Respuesta");
            
            if(error == -1){
              	log_error(logger, "No se pudo guardar el TCB: %d", idTCB);
            }
            else{
              	string_append(&respuesta, "OK");
                _send_message(fd, "RAM", SUCCESS , respuesta, string_length(respuesta), logger);
								log_info(logger, "Se mando con éxito la ubicación del tripulante");

            }
						
						//TO DO: actualizar_mapa(nuestroTCB);
            log_info(logger,"-----------------------------------------------------");
            free(respuesta);					
            free(nuestroTCB);
            break;

        case ENVIAR_PROXIMA_TAREA://idpcb, idtcb
						
            log_info(logger,"-----------------------------------------------------");
						log_info(logger,"Llegó operación: ENVIAR_PROXIMA_TAREA");

            //-----------------------Deserializacion---------------------
            memcpy(&idPCB, buffer, sizeof(int));
						memcpy(&idTCB, buffer + sizeof(int), sizeof(int));
            log_info(logger,"ID PCB:%d", idPCB);
						log_info(logger,"ID TCB:%d", idTCB);
            //-----------------------------------------------------------
            t_queue* segmentos = dictionary_get(diccionario, idPCB);
            segment* segmento_tcb = get_tcb_by_id(segmentos, idTCB);
                
            tcb* nuestroTCB = get_tcb_from_memory(segmento_tcb);
            
            segment* segmento_tareas = get_tareas_by_idPCB(segmentos, idPCB);

            int limite = confirmar_limite_tarea(segmento_tarea, nuestroTCB->next);

						char* respuesta = string_new();
            string_append(&respuesta, "Respuesta");

            if(limite < 0){
								log_info(logger, "No hay mas tareas que mandar");
                _send_message(fd, "RAM", ERROR_NO_HAY_TAREAS, respuesta, string_length(respuesta), logger);

            }else{
                char* tarea = get_next_task(memory,nuestroTCB->next, segmento_tareas->limit);
								
              	nuestroTCB->next += string_length(tarea);
              
                char* buffer_a_enviar =  _serialize(sizeof(int)+string_length(tarea), "%s",tarea);

                _send_message(fd, "RAM", ENVIAR_TAREA , buffer_a_enviar, sizeof(sizeof(int)+string_length(tarea), "%s",tarea), logger);
              	
              	free(buffer_a_enviar);

            }
						free(respuesta);
						log_info(logger,"-----------------------------------------------------");
            break;

        case EXPULSAR_TRIPULANTE:	
                   
						log_info(logger,"-----------------------------------------------------");
						log_info(logger,"Llegó operación: EXPULSAR_TRIPULANTE");
                   
            //-----------------------Deserializo------------------
            memcpy(&idPCB, buffer , sizeof(int));
            memcpy(&idTCB, buffer + sizeof(int), sizeof(int));
            log_info(logger,"ID PCB:%d", idPCB);
						log_info(logger,"ID TCB:%d", idTCB);     
            
            //---------------------------------------------------
            t_queue* segmento_pcb = dictionary_get(diccionario, idPCB);
            segment* segmento_tcb = get_tcb_by_id(segmento_pcb, idTCB);

            int error =  remove_segment_from_memory (memory, memory_size, segmento_tcb);
						
          	if(error < 0){
              log_error(logger, "Error al eliminar el segmento asociado");
						else{
            //// Elimino el tcb del mapa

            //eliminar_tripulante_mapa();

						char* respuesta = string_new();
            string_append(&respuesta, "Respuesta");
            _send_message(fd, "RAM", SUCCESS,respuesta,string_lenght(respuesta),logger);
            free(respuesta);
						log_info(logger,"-----------------------------------------------------");

            }

            break;

        default:
						log_info(logger,"-----------------------------------------------------");
            log_info(logger, "No existe la operación:%d", opcode);
						log_info(logger,"-----------------------------------------------------");
            break;

    }

}