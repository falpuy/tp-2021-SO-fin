#include "main.h"

int main() {

    // Inicializando

    isBestFit = 0;
    global_clock_key = 0;
    global_clock_index = 0;
    hasLRU = 1;

    config = config_create(CONFIG_PATH);
    logger = log_create(ARCHIVO_LOG, PROGRAM, 1, LOG_LEVEL_TRACE);
   
    signal(SIGINT, signal_handler);
    signal(SIGUSR1, signal_handler);
    signal(SIGUSR2, signal_handler);
    //

    //
    if (!_check_config(config, KEYS)) {
        log_error(logger, "Error en el archivo de configuracion..");
        signal_handler(SIGINT);
    }

    log_info(logger, "Mi RAM HQ ejecutando correctamente.. PID: %d", process_getpid());

    // Creo el mapa
    // create_map(logger);
    
    // Valido esquema de memoria
    esquema = config_get_string_value(config, "ESQUEMA_MEMORIA");
    log_info(logger, "Esquema utilizado: %s", esquema);

    mem_size = config_get_int_value(config, "TAMANIO_MEMORIA");
    log_info(logger, "Tamanio de la memoria: %d", mem_size);
    memory = malloc(mem_size);
    admin = malloc(mem_size);
    memset(admin, 0, mem_size);
    table_collection = dictionary_create();
    admin_collection = dictionary_create();

    isBestFit = !strcmp(config_get_string_value(config, "CRITERIO_SELECCION"), "BF");

    char *aux_timer = temporal_get_string_time("%H%M%S");
    timer = atoi(aux_timer);
    free(aux_timer);

    if (!strcmp(esquema, "PAGINACION")) {

      setup_pagination(
        memory,
        config_get_string_value(config, "PATH_SWAP"),
        config_get_int_value(config, "TAMANIO_PAGINA"),
        mem_size,
        config_get_int_value(config, "TAMANIO_SWAP"),
        logger
      );

      // Creo el server
      _start_server(config_get_string_value(config, "PUERTO"), pagination_handler, logger);

    } else {
      // Segmentacion

      // Creo el server
      _start_server(config_get_string_value(config, "PUERTO"), segmentation_handler, logger);

    }

    return EXIT_SUCCESS;
}

void signal_handler(int sig_number) {

  switch(sig_number) {
    case SIGINT:

      if (!strcmp(esquema, "PAGINACION")) {

        free(bitmap);

        munmap(virtual_memory, virtual_size);

        bitarray_destroy(virtual_bitmap);

        dictionary_destroy_and_destroy_elements(table_collection, table_destroyer_pagination);

      }
      else {
        dictionary_destroy_and_destroy_elements(table_collection, table_destroyer);
      }

      free(memory);
      free(admin);
      dictionary_destroy_and_destroy_elements(admin_collection, admin_destroyer);

      // Eliminar Archivo Swap????

      log_destroy(logger);
      config_destroy(config); 

      exit(EXIT_FAILURE);

    break;

    case SIGUSR1:
      memory_compaction(admin, memory, mem_size, table_collection);
    break;

    case SIGUSR2:

      if (!strcmp(esquema, "PAGINACION")) {
        page_dump(table_collection);
      } else {
        memory_dump(table_collection, memory);
      }

    break;
  }
}


// --------------------- SEGMENTATION HANDLER ----------------------- //

void segmentation_handler(int fd, char *id, int opcode, void *buffer, t_log *logger) {
    log_info(logger, "Recibi la siguiente operacion de %s: %d", id, opcode);

    char* data_tareas;
    int cantTripulantes;
    int tamStrTareas;
    int idPCB;
        
    int segment_size;
    int found_segment;
    int offset = 0;

    int idTCB;
    int posX;
    int posY;

    int error;

    char status;
  
    char *respuesta;

    char *temp_id;

    tcb_t *nuestroTCB;
    segment *segmento_tcb;

    segment *segmento_aux;

    int size_a_guardar;

    // TODO: COMANDO UPDATE STATUS
  
    switch (opcode){

        case INICIAR_PATOTA: // idPCB - tareas - cantTCB - IDTCB.... (N id)

        		log_info(logger,"-----------------------------------------------------");
            log_info(logger,"Llegó la operación: INICIAR_PATOTA ");
      			
        	//--------------------------Deserializar -------------------------------              
        		memcpy(&idPCB, buffer, sizeof(int));
            offset += sizeof(int);
        
        		memcpy(&tamStrTareas, buffer + offset, sizeof(int));
            offset += sizeof(int);
            data_tareas = malloc(tamStrTareas + 1);
        
        		memcpy(data_tareas, buffer + offset, tamStrTareas);
        		offset += tamStrTareas;
            data_tareas[tamStrTareas]='\0';
        
        		memcpy(&cantTripulantes, buffer + offset, sizeof(int));
        		offset += sizeof(int);
                   
            log_info(logger,"ID PCB: %d", idPCB);
						log_info(logger,"Tareas de la patota: %s", data_tareas);
          	log_info(logger,"Cantidad de tripulantes: %d", cantTripulantes);
						//--------------------------------------------------------------------
						
            if(cantTripulantes <= 0){
              	log_error(logger, "Error: La cantidad de tripulantes es nula o no existente");
                _send_message(fd, "RAM", ERROR_CANTIDAD_TRIPULANTES , respuesta, string_length(respuesta), logger);
            } else {

             		size_a_guardar = sizeof(pcb_t) + tamStrTareas + cantTripulantes * sizeof(tcb_t);
                log_info(logger,"Size total a guardar en memoria: %d", size_a_guardar);
            
                if (check_space_memory(admin, mem_size, size_a_guardar, table_collection)) {
                  // creo tabla de segmentos
                  t_queue *segmentTable = queue_create();
                  // crear segmento para tareas
                  segment_size = tamStrTareas;
                  found_segment = isBestFit ? memory_best_fit(admin, mem_size, table_collection, segment_size) : memory_seek(admin, mem_size, segment_size, table_collection);
                  
                  if(found_segment < 0) {
                      memory_compaction(admin, memory, mem_size, table_collection);
                      found_segment = isBestFit ? memory_best_fit(admin, mem_size, table_collection, segment_size) : memory_seek(admin, mem_size, segment_size, table_collection);
                  }
              
                  pcb_t *temp = malloc(sizeof(pcb_t));
                  temp -> pid = idPCB;
                  temp -> tasks = found_segment;
                  
                  // guardo tareas
                  segment *tareas = malloc(sizeof(segment));
                
                  tareas -> nroSegmento = get_last_index (segmentTable) + 1;
                  tareas -> baseAddr = found_segment;
                  tareas -> limit = found_segment + tamStrTareas;
                  tareas -> id = idPCB;
                  tareas -> type = TASK;
                  
                  save_task_in_memory(admin, memory, mem_size, tareas, data_tareas);
                  queue_push(segmentTable, tareas);
                  temp_id = string_itoa(idPCB);
                  dictionary_put(table_collection, temp_id, segmentTable);

                  // creo segmento pcb
                  segment *segmento_pcb = malloc(sizeof(segment));
                
                  segment_size = sizeof(pcb_t);
                  found_segment = isBestFit ? memory_best_fit(admin, mem_size, table_collection, segment_size) : memory_seek(admin, mem_size, segment_size, table_collection);
              
                  if(found_segment < 0) {
                      memory_compaction(admin, memory, mem_size, table_collection);
                      found_segment = isBestFit ? memory_best_fit(admin, mem_size, table_collection, segment_size) : memory_seek(admin, mem_size, segment_size, table_collection);
                  }
                
                  segmento_pcb -> nroSegmento = get_last_index (segmentTable) + 1;
                  segmento_pcb -> baseAddr = found_segment;
                  segmento_pcb -> limit = found_segment + sizeof(pcb_t);
                  segmento_pcb -> id = idPCB;
                  segmento_pcb -> type = PCB;
                  
                  // save_task_in_memory(memory, mem_size, tareas, data_tareas);
                  save_pcb_in_memory(admin, memory, mem_size, segmento_pcb, temp);
                  queue_push(segmentTable, segmento_pcb);
                  dictionary_put(table_collection, temp_id, segmentTable);

                  // guardo los tcbs
                  for(int i = 0; i < cantTripulantes; i++) {
                  
                    tcb_t *aux = malloc(sizeof(tcb_t));
                    
                    memcpy(&aux -> tid, buffer + offset, sizeof(int));
                    offset += sizeof(int);
                    memcpy(&aux -> pid, buffer + offset, sizeof(int));
                    offset += sizeof(int);
                    memcpy(&aux -> status, buffer + offset, sizeof(char));
                    offset += sizeof(char);
                    memcpy(&aux -> xpos, buffer + offset, sizeof(int));
                    offset += sizeof(int);
                    memcpy(&aux -> ypos, buffer + offset, sizeof(int));
                    offset += sizeof(int);
                    
                    aux -> next = temp -> tasks;
                    
                    segment_size = sizeof(tcb_t);
                    found_segment = isBestFit ? memory_best_fit(admin, mem_size, table_collection, segment_size) : memory_seek(admin, mem_size, segment_size, table_collection);
                    
                    if(found_segment < 0) {
                        memory_compaction(admin, memory, mem_size, table_collection);
                        found_segment = isBestFit ? memory_best_fit(admin, mem_size, table_collection, segment_size) : memory_seek(admin, mem_size, segment_size, table_collection);
                    }
                    
                    segment *segmento_aux = malloc(sizeof(segment));
                    segmento_aux -> id = aux -> tid;
                    segmento_aux -> type = TCB;
                    segmento_aux -> nroSegmento = get_last_index (segmentTable) + 1;
                    segmento_aux -> baseAddr = found_segment;
                    segmento_aux -> limit = found_segment + sizeof(tcb_t);
                    
                    save_tcb_in_memory(admin, memory, mem_size, segmento_aux, aux);
                    free(aux);
                    queue_push(segmentTable, segmento_aux);
                    dictionary_put(table_collection, temp_id, segmentTable);
                  }

                  free(temp);
                  free(temp_id);
                
                  // guardo la tabla en el diccionario
                  // dictionary_put(table_collection, string_itoa(temp -> pid), segmentTable);
                
                  log_info(logger, "Se guardaron exitosamente los datos de la patota");
                  respuesta = string_new();
   					      string_append(&respuesta, "Respuesta");
                  _send_message(fd, "RAM", SUCCESS ,respuesta, string_length(respuesta), logger);
                  free(respuesta); 
                
              } else {
                  log_error(logger, "No hay mas espacio en memoria");
                  respuesta = string_new();
   					      string_append(&respuesta, "Respuesta");
                  _send_message(fd, "RAM", ERROR_POR_FALTA_DE_MEMORIA ,respuesta, string_length(respuesta), logger);
                  free(respuesta); 
              }
      
            }
            log_info(logger,"-----------------------------------------------------");
            free(data_tareas);
              
        break;

        case RECIBIR_ESTADO_TRIPULANTE: //ID_PATOTA, ID_TCB, POS_X, POS_Y
          	log_info(logger,"-----------------------------------------------------");
            log_info(logger,"Llegó la operación: RECIBIR_ESTADO_TRIPULANTE");
            //------------Deserializo parámetros-------------------
            memcpy(&idPCB, buffer,sizeof(int));
            offset += sizeof(int);
            
            memcpy(&idTCB, buffer + offset, sizeof(int));
            offset += sizeof(int);
            
						memcpy(&status, buffer + offset, sizeof(char));
            offset += sizeof(char);
              
            log_info(logger,"ID PCB: %d", idPCB);
						log_info(logger,"ID TCB: %d", idTCB);
          	log_info(logger,"Estado: %c", status);
            //----------------------------------------------------
            temp_id = string_itoa(idPCB);
            segmento_tcb = find_tcb_segment(idTCB, temp_id, table_collection);
            free(temp_id); 

            nuestroTCB = get_tcb_from_memory(memory, mem_size, segmento_tcb);

            nuestroTCB -> status = status;

            error = save_tcb_in_memory(admin, memory, mem_size, segmento_tcb, nuestroTCB);
					
            char* respuesta2 = string_new();
            string_append(&respuesta2, "Respuesta");
            
            if(error == -1){
              	log_error(logger, "No se pudo guardar el TCB: %d", idTCB);
                _send_message(fd, "RAM", ERROR_GUARDAR_TCB, respuesta2, string_length(respuesta2) , logger);
            }
            else{
                _send_message(fd, "RAM", SUCCESS , respuesta2, string_length(respuesta2), logger);
								log_info(logger, "Se mando con éxito el estado del tripulante");

            }
						
						//TO DO: actualizar_mapa(nuestroTCB);
            log_info(logger,"-----------------------------------------------------");
            free(respuesta2);					
            free(nuestroTCB);
        break;

        case RECIBIR_UBICACION_TRIPULANTE: //ID_PATOTA, ID_TCB, POS_X, POS_Y
          	log_info(logger,"-----------------------------------------------------");
            log_info(logger,"Llegó la operación: RECIBIR_UBICACION_TRIPULANTE");
            //------------Deserializo parámetros-------------------
            memcpy(&idPCB, buffer,sizeof(int));
            offset += sizeof(int);
            
            memcpy(&idTCB, buffer + offset, sizeof(int));
            offset += sizeof(int);
            
						memcpy(&posX, buffer + offset, sizeof(int));
            offset += sizeof(int);
              
						memcpy(&posY, buffer + offset, sizeof(int));
              
            log_info(logger,"ID PCB: %d", idPCB);
						log_info(logger,"ID TCB: %d", idTCB);
          	log_info(logger,"Posicion en X: %d", posX);
            log_info(logger,"Posicion en Y: %d", posY);
            //----------------------------------------------------
            temp_id = string_itoa(idPCB);
            segmento_tcb = find_tcb_segment(idTCB, temp_id, table_collection);
            free(temp_id); 

            nuestroTCB = get_tcb_from_memory(memory, mem_size, segmento_tcb);

            nuestroTCB -> xpos = posX;
            nuestroTCB -> ypos = posY;

            error = save_tcb_in_memory(admin, memory, mem_size, segmento_tcb, nuestroTCB);
					
            char* respuesta3 = string_new();
            string_append(&respuesta3, "Respuesta");
            
            if(error == -1){
              	log_error(logger, "No se pudo guardar el TCB: %d", idTCB);
                _send_message(fd, "RAM", ERROR_GUARDAR_TCB, respuesta3, string_length(respuesta3) , logger);
            }
            else{
                _send_message(fd, "RAM", SUCCESS , respuesta3, string_length(respuesta3), logger);
								log_info(logger, "Se mando con éxito la ubicación del tripulante");

            }
						
						//TO DO: actualizar_mapa(nuestroTCB);
            log_info(logger,"-----------------------------------------------------");
            free(respuesta3);					
            free(nuestroTCB);
        break;

        case ENVIAR_TAREA://idpcb, idtcb
						
            log_info(logger,"-----------------------------------------------------");
						log_info(logger,"Llegó operación: ENVIAR_PROXIMA_TAREA");

            //-----------------------Deserializacion---------------------
            memcpy(&idPCB, buffer, sizeof(int));
						memcpy(&idTCB, buffer + sizeof(int), sizeof(int));

            log_info(logger,"ID PCB:%d", idPCB);
						log_info(logger,"ID TCB:%d", idTCB);
            //-----------------------------------------------------------

            temp_id = string_itoa(idPCB);
            segmento_tcb = find_tcb_segment(idTCB, temp_id, table_collection);
            segment* segmento_tareas = find_task_segment(temp_id, table_collection);
            free(temp_id); 

            nuestroTCB = get_tcb_from_memory(memory, mem_size, segmento_tcb);

            char* tarea = get_next_task(memory, nuestroTCB->next, segmento_tareas->limit,logger);
            log_info(logger, "La tarea a enviar es:%s",tarea);
            
            if(!tarea){
								log_info(logger, "No hay mas tareas que mandar");
                char* temp = string_new();
                string_append(&temp, "answer");
                char* mandamos = _serialize(sizeof(int)+strlen(temp),"%s",temp);

                _send_message(fd, "RAM", ERROR_NO_HAY_TAREAS, mandamos, sizeof(int)+strlen(mandamos), logger);
            }else{
								
              	nuestroTCB->next += strlen(tarea);

                save_tcb_in_memory(admin, memory, mem_size, segmento_tcb, nuestroTCB);
                log_info(logger, "La tarea a enviar es: %s",tarea);
                log_info(logger, "El tamaño de tarea a enviar es: %d",strlen(tarea));

                char* buffer_a_enviar =  _serialize(sizeof(int)+strlen(tarea),"%s",tarea);
                _send_message(fd, "RAM", SUCCESS, buffer_a_enviar, sizeof(int)+string_length(tarea), logger);
              	
              	free(buffer_a_enviar);

            }
						log_info(logger,"-----------------------------------------------------");
        break;

        case ELIMINAR_PATOTA:

            log_info(logger,"-----------------------------------------------------");
            log_info(logger,"Llegó la operación: ELIMINAR_PATOTA");         

            //------------Deserializo parámetros-------------------
            memcpy(&idPCB, buffer,sizeof(int));
            offset += sizeof(int);

            temp_id = string_itoa(idPCB);
            log_info(logger,"ID PCB: %s", temp_id);

            // remove_pcb_from_memory(memory, mem_size, table_collection, temp_id);
            segmento_aux = get_next_segment(table_collection, temp_id);

            while (segmento_aux != NULL) {
              if(remove_segment_from_memory(admin, mem_size, segmento_aux)) {
                  remove_segment_from_table(table_collection, temp_id, segmento_aux);
              }

              segmento_aux = get_next_segment(table_collection, temp_id);
            }

            free(temp_id); 
              
            //----------------------------------------------------

            char* respuesta4 = string_new();
            string_append(&respuesta4, "Respuesta");
            _send_message(fd, "RAM", SUCCESS,respuesta4,string_length(respuesta4),logger);
            free(respuesta4);

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
            temp_id = string_itoa(idPCB);
            segmento_tcb = find_tcb_segment(idTCB, temp_id, table_collection);

            error =  remove_segment_from_memory (admin, mem_size, segmento_tcb);

            // TODO: BORRAR SEGMENTO DE LA TABLA DE SEGMENTOS
            remove_segment_from_table(table_collection, temp_id, segmento_tcb);

            free(temp_id); 
						
          	if(error < 0){
              log_error(logger, "Error al eliminar el segmento asociado");
              respuesta = string_new();
              string_append(&respuesta, "Respuesta");
              _send_message(fd, "RAM", ERROR_GUARDAR_TCB,respuesta,string_length(respuesta),logger);
              free(respuesta);
						} else {
            //// Elimino el tcb del mapa

            //eliminar_tripulante_mapa();

              char* respuesta5 = string_new();
              string_append(&respuesta5, "Respuesta");
              _send_message(fd, "RAM", SUCCESS,respuesta5,string_length(respuesta5),logger);
              free(respuesta5);
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


void pagination_handler(int fd, char *id, int opcode, void *buffer, t_log *logger) {


    log_info(logger, "Recibi la siguiente operacion de %s: %d", id, opcode);

    char* data_tareas;
    int cantTripulantes;
    int tamStrTareas;
    int idPCB;
    int idTCB, posX, posY;
    int offset = 0;
    char* idPCBkey;  
    char *respuesta;

    // TODO: COMANDO UPDATE STATUS
  
    switch (opcode){

        /*case RECIBIR_ESTADO_TRIPULANTE:
        break;*/

        
        case INICIAR_PATOTA: // idPCB - tareas - cantTCB - IDTCB.... (N id)

        		log_info(logger,"-----------------------------------------------------");
            log_info(logger,"Llegó la operación: INICIAR_PATOTA ");
      			
        	//--------------------------Deserializar -------------------------------              
        		memcpy(&idPCB, buffer, sizeof(int));
            offset += sizeof(int);
        
        		memcpy(&tamStrTareas, buffer + offset, sizeof(int));
            offset += sizeof(int);
            data_tareas = malloc(tamStrTareas + 1);

        		memcpy(data_tareas, buffer + offset, tamStrTareas);
        		offset += tamStrTareas;
            data_tareas[tamStrTareas]='\0';
                    
        		memcpy(&cantTripulantes, buffer + offset, sizeof(int));
        		offset += sizeof(int);
                   
            log_info(logger,"ID PCB: %d", idPCB);
						log_info(logger,"Tareas de la patota: %s", data_tareas);
          	log_info(logger,"Cantidad de tripulantes: %d", cantTripulantes);
						//--------------------------------------------------------------------        
        		
          	respuesta = string_new();
   					string_append(&respuesta, "Respuesta");
						
            if(cantTripulantes <= 0){
              	log_error(logger, "Error: La cantidad de tripulantes es nula o no existente");
                _send_message(fd, "RAM", ERROR_CANTIDAD_TRIPULANTES , respuesta, string_length(respuesta), logger);
            } else {
              // guardo en memoria, primero se chequea si hay espacio o no
              int hayEspacio = 0;
              //int temporal_memory_size = sizeof(pcb) + tamStrTareas + cantTripulantes * sizeof(tcb);

              double val = mem_size / page_size;
              int frames_count = ceil(val);
              hayEspacio = verificarCondicionDeMemoria(frames_count);

              log_info(logger, "--------------------------------------");
              if(hayEspacio > 0){
                save_data_in_memory(memory, table_collection, admin_collection, buffer);
                log_info(logger, "Se pudo guardar correctamente en memoria, enviando respuesta a Discordiador\n");
                _send_message(fd, "RAM", SUCCESS, respuesta, string_length(respuesta), logger);
              }else{
                if(hayEspacio < 0){
                  log_info(logger, "No habia memoria, enviamos mensaje a Discordiador\n");
                  _send_message(fd, "RAM", ERROR_POR_FALTA_DE_MEMORIA, respuesta, string_length(respuesta), logger);
                }
                else{
                  log_info(logger, "Ocurrio un error inesperado al tratar de chequear el estado de memoria\n");
                  _send_message(fd, "RAM", ERROR_POR_FALTA_DE_MEMORIA, respuesta, string_length(respuesta), logger);
                }
              }
      
            }
            log_info(logger,"-----------------------------------------------------");
            free(data_tareas);     
            free(respuesta); 
              
        break;
                
        

        case RECIBIR_UBICACION_TRIPULANTE: //ID_PATOTA, ID_TCB, POS_X, POS_Y 
          	log_info(logger,"-----------------------------------------------------");
            log_info(logger,"Llegó la operación: RECIBIR_UBICACION_TRIPULANTE");
            //------------Deserializo parámetros-------------------
            memcpy(&idPCB, buffer,sizeof(int));
            offset += sizeof(int);
            
            memcpy(&idTCB, buffer + offset, sizeof(int));
            offset += sizeof(int);
            
						memcpy(&posX, buffer + offset, sizeof(int));
            offset += sizeof(int);
              
						memcpy(&posY, buffer + offset, sizeof(int));
              
            log_info(logger,"ID PCB: %d", idPCB);
						log_info(logger,"ID TCB: %d", idTCB);
          	log_info(logger,"Posicion en X: %d", posX);
            log_info(logger,"Posicion en Y: %d", posY);
            //----------------------------------------------------
            idPCBkey = string_itoa(idPCB);
            update_position_from_page(memory, admin_collection, table_collection, idPCBkey, idTCB, posX, posY);
					
            respuesta = string_new();
            string_append(&respuesta, "Respuesta");
            
            
            _send_message(fd, "RAM", SUCCESS , respuesta, string_length(respuesta), logger);
						log_info(logger, "Se mando con éxito la ubicación del tripulante");
            
						
						//TO DO: actualizar_mapa(nuestroTCB);
            log_info(logger,"-----------------------------------------------------");
            free(respuesta);					
            break;

        case ENVIAR_TAREA://idpcb, idtcb
						
            log_info(logger,"-----------------------------------------------------");
						log_info(logger,"Llegó operación: ENVIAR_PROXIMA_TAREA");

            //-----------------------Deserializacion---------------------
            memcpy(&idPCB, buffer, sizeof(int));
						memcpy(&idTCB, buffer + sizeof(int), sizeof(int));
            log_info(logger,"ID PCB:%d", idPCB);
						log_info(logger,"ID TCB:%d", idTCB);
            //-----------------------------------------------------------
            
            idPCBkey = string_itoa(idPCB);
            char* tarea = get_task_from_page(memory, admin_collection, table_collection, idPCBkey, idTCB);

						respuesta = string_new();
            string_append(&respuesta, "Respuesta");

            int tamTarea = string_length(tarea);  
            char* buffer_a_enviar =  _serialize(sizeof(int) + tamTarea, tarea);

            _send_message(fd, "RAM", ENVIAR_TAREA , buffer_a_enviar, sizeof(sizeof(int) + tamTarea), logger);
              	
            free(buffer_a_enviar);
            free(tarea);
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
            //"-----------------------------------------------------"
            //// Elimino el tcb del mapa

            //eliminar_tripulante_mapa();

            idPCBkey = string_itoa(idPCB);

            respuesta = string_new();
            string_append(&respuesta, "Respuesta");

            remove_tcb_from_page(memory, admin_collection, table_collection, idPCBkey, idTCB);
            _send_message(fd, "RAM", SUCCESS,respuesta, string_length(respuesta), logger);
            
            free(respuesta);
            
            log_info(logger,"-----------------------------------------------------");


        break;

        default:
          log_info(logger,"-----------------------------------------------------");
          log_info(logger, "No existe la operación:%d", opcode);
          log_info(logger,"-----------------------------------------------------");
        break;

    }

}

// --------------------- END HANDLER ----------------------- //

// ------------------ SETUP PAGINATION --------------------- //

void setup_pagination(void *memory, char *path, int tam_pag, int real_size, int v_size, t_log *logger) {

    page_size = tam_pag;
    
    frames_memory = real_size / page_size;

    // printf("Cant: %d", frames_memory);

    bitmap = malloc(frames_memory * sizeof(uint8_t));

    for(int i = 0; i < frames_memory; i++) {
        bitmap[i] = 0;
    }

    log_info(logger, "Muestro valores del bitmap para REAL..");
    for(int i = 0; i < frames_memory; i++){
        log_info(logger, "Bit %d: %d", i, bitmap[i]);
    }

    // ---------- VIRTUAL MEMORY SETUP ---------- //

    virtual_size = v_size;

    frames_virtual = virtual_size / page_size;

    int arch_bitmap;

    if( access( path, F_OK ) >= 0 ) {
        // Existe el file..

        remove(path);
    }

    arch_bitmap = open(path, O_CREAT | O_RDWR, 0664);
    posix_fallocate(arch_bitmap, 0, virtual_size + frames_virtual / 8);

    virtual_memory = mmap(NULL, virtual_size + frames_virtual / 8, PROT_READ | PROT_WRITE, MAP_SHARED, arch_bitmap, 0);

    // Creo el bitmap seguido del espacio de memoria para disco
    virtual_bitmap = bitarray_create_with_mode((char *) virtual_memory + virtual_size, frames_virtual / 8, MSB_FIRST);

    for(int i = 0; i < frames_virtual; i++){
        bitarray_clean_bit(virtual_bitmap, i);
    }

    log_info(logger, "Muestro valores INICIALES del bitmap para VIRTUAL..");
    for(int i = 0; i < frames_virtual; i++){
        log_info(logger, "Bit %d: %d", i, bitarray_test_bit(virtual_bitmap, i));
    }
    
}