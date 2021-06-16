#include "./handler.h"

void handler(int fd, char *id, int opcode, void *buffer, t_log *logger) {
    log_info(logger, "Recibi la siguiente operacion de %s: %d", id, opcode);

    switch (opcode){
        case INICIAR_PATOTA: // idPCB - tareas - cantTCB - IDTCB.... (N id)
        
      			//---------------------Deserializar -------------------------------
        		char* idPCB, tareas;
        		int cantTripulantes, tamStrPCB, tamStrTareas;
        		int tcbs[cantidadTripulantes];
        		int offset = 0;
        
        		memcpy(&tamStrPCB,payload,sizeof(int));
            offset += sizeof(int);
            idPCB = malloc(tamStrPCB + 1);
        
        		memcpy(idPCB, payload + offset ,tamStrPCB);
            offset += tamStrPCB;
        
        		memcpy(&tamStrTareas,payload,sizeof(int));
            offset += sizeof(int);
            tareas = malloc(tamStrTareas + 1);
        
        		memcpy(tareas, payload + offset, tamStrTareas);
        		offset += tamSrtTareas;
        
        
        		//-----------------------------------------------------------------

            // Recibe el idPCB para iniciar la patota y los idTCB de los tripulantes que pertenecen a la misma para crearlos tambien.

            // Primero chequeo por errores, asi no queda el pcb guardado en memoria faltandole cosas

            // Error por si se ingresa una cantidad menor o igual a cero de tripulantes

            if(cantTripulantes <= 0){

                //COMANDO ERROR, INGRESO UN NUMERO INCORRECTO DE TRIPULANTES: 560

                _send_message(fd, "RAM", 560, "", 1, logger);

                break;

            }



            // IMPORTANTE::: primero que nada hay que ver si nos va a alcanzar la memoria para guardar todo, porque sino habria que revertirlo y seria un quilombo. Chequear si hay espacio para el segmento del PCB, el de las tareas, y para los de todos los TCB... falta hacerlo

            

            // Recibo el id de la patota correspondiente



            // Busco la tabla de segmentos en el diccionario -> dictionary_has_key()



            // Si no existe la patota

                // pido las tareas de la patota X

            if (dictionary_has_key(diccionario, idPCB)){

                //COMANDO ERROR: YA EXISTE LA PATOTA, ERROR --> 550

                _send_message(fd, "RAM", 550 , "", 1 ,logger);

                break;

            }



            // Si no existe la patota

            else {



                // Creo el segmento de la patota, para ello necesito idPCB y las tasks(, numero de segmento que tomara y su base y limite?)

                pcb nuevoPCB = pcb_create(idPCB, tasks);

                segment* patota = crear_segmento(nuevoPCB);

                // Busco espacio libre

                int resultadoMemoria = operacion_de_memoria(&memory, &m_size, &diccionario, patota);

                if(resultadoMemoria == 0){

                    //COMANDO ERROR POR FALTA DE MEMORIA: 555

                    _send_message(fd, "RAM", 555, "", 1, logger);

                    break;

                }

                /*int operacion_de_memoria(void* memory, void* m_size, void* diccionario, void* elemento_a_operar) // Preguntarle tipos a agus{

                    int total_size = elemento_a_operar->limit - elemento_a_operar->baseAddr;

                    int found_segment = memory_seek(memory, m_size, diccionario, total_size);

                    if(found_segment >0) {

                        printf("Encontre espacio, guardando segmento...");

                        memcpy(memory + found_segment, elemento_a_operar, total_size);

                        return 1;

                    }

                    // Si no hay espacio

                        // Realizo compactacion

                        // Busco espacio libre

                    else {

                        printf("No encontre ningun segmento libre.. Iniciando compactacion.\n");



                        memory_compaction(memory, m_size, diccionario);



                        printf("Buscando un segmento de tamanio: %d\n", total_size);



                        found_segment = memory_seek(memory, m_size, diccionario, total_size);

                        // Si hay espacio

                            // guardo el segmento

                            // Agrego el segmento a la lista

                            // Devuelvo la tarea del tripulante

                        if(found_segment > 0){

                            printf("Encontre espacio, guardando segmento...");

                            memcpy(memory + found_segment, elemento_a_operar, total_size);

                            return 1;

                        }

                        // No hay espacio

                            // Envio mensaje de Error

                        else {

                            return 0;

                        }

                    }

                }*/

                

                //Creo el segmento de tareas y lo guardo en memoria

                segment* tareas = crear_segmento(tasks);

                // Busco espacio libre

                resultadoMemoria = operacion_de_memoria(&memory, &m_size, &diccionario, patota);

                if(resultadoMemoria == 0){

                    //COMANDO ERROR_POR_FALTA_DE_MEMORIA: 555

                    _send_message(fd, "RAM", 555, "", 1, logger);

                    break;

                }

                

                // Empiezo a crear los TCB

                for(int i=1, i<=cantTripulantes, i++){

                    // Creo el segmento del tripulante

                    tcb nuevoTripulante = crear_tripulante(idPCB, i, buffer); // Nota: adentro va a buscar la proxima tarea y asignarla

                    /*// Me traigo ese segmento de patota para usar la direccion de las tareas

                        t_queue* segmento_patota = dictionary_get(diccionario, idPCB);

                        // Busco la primer tarea de la lista correspondiente al pcb del tripulante

                        segment* tareas = obtener_tareas(segmento_patota);

                        char* proximaTarea = get_next_task(memory, tareas->baseAddr, tareas->limit);

                        nuevoTripulante->next = proximaTarea;*/

                    segment* tripulante = crear_segmento(nuevoTripulante);

                    // Busco espacio libre

                    int resultadoMemoria = operacion_de_memoria(&memory, &m_size, &diccionario, patota);

                    if(resultadoMemoria == 0){

                        //COMANDO ERROR_POR_FALTA_DE_MEMORIA: 555

                        _send_message(fd, "RAM", 555, "", 1, logger);

                        break;

                    }

                }

        break;
        
        case RECIBIR_UBICACION_TRIPULANTE: //ID_PATOTA, ID_TCB, POS_X, POS_Y

            char* idPCB;
            int tamStrPCB,idTCB,posX,posY;
            int offset = 0;
						
            
        //------------Deserializo parámetros-------------------
            memcpy(&tamStrPCB,payload,sizeof(int));
            offset += sizeof(int);
            idPCB = malloc(tamStrPCB + 1);
            
            memcpy(idPCB, payload + offset ,tamStrPCB);
            offset += tamStrPCB;
            
            memcpy(&idTCB, payload + offset, sizeof(int));
            offset += sizeof(int);
            
						memcpy(&posX, payload + offset, sizeof(int));
            offset += sizeof(int);
              
						memcpy(&posY, payload + offset, sizeof(int));
        //----------------------------------------------------
              
              {idPCB: valor}
            --> t_queue* segmento_pcb = dictionary_get(diccionario, idPCB);

            segment* segmento_tcb = get_tcb_by_id(segmento_pcb, idTCB);

            tcb* nuestroTCB = get_tcb_from_memory(memory, m_size, segmento_tcb);

            int error = save_tcb_in_memory(&memory, m_size, segmento_tcb, nuestroTCB);
					
            char* respuesta = string_new();
            
            if(error == -1){
              	log_error(logger, "No se pudo guardar el TCB: %d", idTCB);
                _send_message(fd, "RAM", ERROR_GUARDAR_TCB, respuesta, string_length(respuesta) , logger);
            }
            else{
              	string_append(&respuesta, "OK");
                _send_message(fd, "RAM", SUCCESS , respuesta, string_length(respuesta), logger);
            }
						
            
			//TO DO: actualizar_mapa(nuestroTCB);
            
            free(respuesta);					
            free(nuestroTCB);
            free(segmento_tcb);
            

            break;
        case ENVIAR_PROXIMA_TAREA:
            //idpcb, idtcb
            char* idPCB = recibir_pbc(buffer);

            t_queue* segmentos = dictionary_get(diccionario, idPCB);

            // Busco la info del tcb por id, y busco el segmento con el nroSegmento
            int idTCB = recibir_tcb(buffer);
            segment* segmento_tcb = get_tcb_by_id(segmentos, idTCB);
             // Me traigo la info de memoria en un struct tcb
            tcb* nuestroTCB = get_tcb_from_memory(segmento_tcb);
            // Buscar segmento de tareas
            segment* segmento_tareas = get_tareas_by_idPCB(segmentos, idPCB);
            // Validar si la direccion de la proxima tarea en el tcb es menor al limite del segmento de tareas (si hay mas tareas para hacer)
            int limite = confirmar_limite_tarea(segmento_tarea, nuestroTCB->next);
            // Si no hay mas tareas
                // Envio mensaje de error
            if(limite < 0){
                _send_message(fd, "RAM", 999, "Error, no hay mas tareas", tam_ese_string, logger);
            }
            // Si hay tareas
            else{
                // Leo una tarea con el segmento del tcb
                char* tarea = get_tarea_from_segmento_tareas(segmento_tareas);
                nuestroTCB->next = tarea;
                // Actualizo el tcb en memoria
                guardar_tcb_memory(segmento, nuestroTCB, memory, memory_size);
                // Envio proxima tarea
                char* buffer_a_enviar = serialize(tarea);
                _send_message(fd, "RAM", 520, buffer_a_enviar, sizeof(buffer_a_enviar), logger);
            }

            break;
        case EXPULSAR_TRIPULANTE:
            //idpcb, idtcb
            // Busco el segmento del tcb
            log_info(logger, "entre al case rut"); 
            // Recibo id tcb
            char* idPCB = recibir_pbc(buffer);
            // Buscar la tabla de segmentos correspondiente al id de patota
            t_queue* segmentos = dictionary_get(diccionario, idPCB);

            // Busco la info del tcb por id, y busco el segmento con el nroSegmento
            int idTCB = recibir_tcb(buffer);
            segment* segmento_tcb = get_tcb_by_id(segmentos, idTCB);
            tcb* nuestroTCB = get_tcb_from_memory(segmento_tcb);
            // Elimino el segmento de memoria
            borrar_tcb_memory(segmento_tcb, nuestroTCB, memory, memory_size);
            // Elimino el segmento de la lista de segmentos y de info
            eliminar_segmento(segmentos, segmento_tcb);
            //// Elimino el tcb del mapa
            eliminar_tripulante_mapa();
            // Envio mensaje de OK
            _send_message(fd, "RAM", 999, "Tripulante expulsado con exito", sizeof("Tripulante expulsado con exito"), logger);

            break;
        case MENSAJE:
            log_info(logger, "Message: %s", buffer);
            _send_message(fd, "RAM", 999, "ok", strlen("ok"), logger);
        break;
        
        default:
            log_info(logger, "No se pudo realizar la operacion %d", opcode);
            break;
    }
}