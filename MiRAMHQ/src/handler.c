#include "./handler.h"

void handler(int fd, char *id, int opcode, void *buffer, t_log *logger) {
    log_info(logger, "Recibi la siguiente operacion de %s: %d", id, opcode);

    switch (opcode){
        case INICIAR_TRIPULANTE:
            // Será el encargado de crear la o las estructuras administrativas necesarias para que un tripulante pueda ejecutar.
            // En caso de que no se encuentre creada la patota a la que pertenece, deberá solicitar el listado de tareas.

            // Recibo el id de la patota correspondiente al tripulante

            // Busco la tabla de segmentos en el diccionario -> dictionary_has_key()

            // Si no existe la patota
                // pido las tareas de la patota X

            // Si existe la patota

                // Me traigo ese segmento de patota para usar la direccion de las tareas
                // Busco la primer tarea de la lista correspondiente al pcb del tripulante
                // Creo el segmento del tripulante
                // asigno la ubicacion de la siguiente tarea al segmento
                // Busco espacio libre

                // Si hay espacio
                    // guardo el segmento
                    // Agrego el segmento a la lista
                    // Devuelvo la tarea del tripulante

                    //// Agrego el tcb en el mapa
                
                // Si no hay espacio
                    // Realizo compactacion
                    // Busco espacio libre
                    
                    // Si hay espacio
                        // guardo el segmento
                        // Agrego el segmento a la lista
                        // Devuelvo la tarea del tripulante

                        //// Agrego el tcb en el mapa
                    
                    // No hay espacio
                        // Envio mensaje de Error

            // ------------- Analizar Caso ------------- //

            // verifico si existe el segmento del tripulante
                // Si existe el tripulante
                    // Verifico si tiene la direccion a la proxima tarea a ejecutar
                    // Busco la tarea correspondiente a la direccion (o la primer tarea, si no tiene)
                    // Actualizo el segmento con la proxima tarea
                    // Devuelvo la tarea del tripulante

        break;

        case RECIBIR_TAREAS_PATOTA:

         //pcb *patota = deserialize_pcb(buffer, &tasks_size);
        log_info(logger, "entre en el case");

        //ejemplo falsoPCB, tiene un int y un vector de char* que tiene dos char*
        //
        //variables
        int offset = 0;
        falsoPCB* pcb = malloc(sizeof(falsoPCB));
        int temp = 0;
        //sacamos el int
        memcpy(&offset, buffer, sizeof(int));
        pcb->idPat = offset;
        log_info(logger, "%d", pcb->idPat); //log para testear que salga bien
        //sacamos el primer string
        offset=sizeof(int)*2;
        memcpy(&temp, buffer + sizeof(int), sizeof(int));
        pcb->tripulantes[0] = malloc(temp + 1);
        memcpy(pcb->tripulantes[0], buffer + offset, temp);
        pcb->tripulantes[0][temp+1] = '\0';
        offset+=temp;
        offset+=1;
        log_info(logger, "%s", pcb->tripulantes[0]); //log para testear que salga bien
        //sacamos el segundo string (aca hay algo que no funca btw)
        memcpy(&temp, buffer + offset, sizeof(int));
        offset+=sizeof(int);
        pcb->tripulantes[1] = malloc(temp + 1);
        memcpy(pcb->tripulantes[1], buffer + offset, temp);
        pcb->tripulantes[1][temp] = '\0';
        log_info(logger, "%s", pcb->tripulantes[1]); //log para testear que salga bien

        log_info(logger, "%d %s %s", pcb->idPat, pcb->tripulantes[0], pcb->tripulantes[1]); //log que muestra todo el pcb falso de testeo

        break;
        /*
        case RECIBIR_TAREAS_PATOTA:

            // Recibo ID_PATOTA + LISTA_TAREAS
            
            // creo el segmento e info de las tareas asociado al pcb

            // Busco espacio libre para tareas
            
            // si hay espacio
                // Guardo las tareas en memoria
                // Actualizo el segmento de tareas con base | limite

                --------- PCB ---------
                // Creo el segmento para el pcb
                // Asocio la direccion de inicio de las tareas al pcb -> base
                // Busco espacio para el segmento de pcb
                
                // si hay espacio

                    // Guardo el pcb en memoria
                    // Agrego el segmento pcb a la lista
                    // Agrego el segmento tareas a la lista
                    // Agregar la lista de segmentos al diccionario
                    // Envio mensaje de OK
                
                // Si no hay espacio
                    // Realizo compactacion
                    // Busco espacio libre
                    
                    // Si hay
                        // Guardo el pcb en memoria
                        // Agrego el segmento pcb a la lista
                        // Agrego el segmento tareas a la lista
                        // Envio mensaje de OK
                    
                    // si no hay
                        // Elimino el segmento de tareas guardado en memoria
                        // No guardo los segmentos en la lista de segmentos
                        // Envio mensaje de Error

            
            // Si no hay espacio
                // Realizo compactacion
                // Busco espacio libre
                
                // Si hay
                    // Guardo las tareas en memoria

                    --------- PCB ---------
                    // Creo el segmento para el pcb
                    // Asocio la direccion de inicio de las tareas al pcb
                    // Busco espacio para el segmento de pcb
                    
                    // si hay espacio
                        // Guardo el pcb en memoria
                        // Agrego el segmento pcb a la lista
                        // Agrego el segmento tareas a la lista
                        // Envio mensaje de OK
                    
                    // Si no hay espacio
                        // Elimino el segmento de tareas guardado en memoria
                        // No guardo los segmentos en la lista de segmentos
                        // Envio mensaje de Error

                
                // si no hay
                    // Elimino el segmento de tareas guardado en memoria
                    // No guardo los segmentos en la lista de segmentos
                    // Envio mensaje de Error


            // ------------------------------------ IGNORE -------------------------------------//

            // recibirá el listado de tareas de la patota y los almacenará en la memoria.

            // deserializo el payload
            pcb *patota = deserialize_pcb(buffer);
            
            // ------------ PCB ID ------------ //

            int total_size = sizeof(uint32_t);

            // Busco una direccion en memoria con el tamanio necesario para guardar el buffer
            int addr = get_available_location(memory, total_size);
            
            // Creo el segmento asociado
            segment *new_segment = malloc(sizeof(segment));
            new_segment -> baseAddr = addr;
            new_segment -> limit = total_size;

            // Agrego el numero de segmento
            // Busco el ultimo indice que se agrego en la lista de segmentos y agrego uno mas
            new_segment -> nroSegmento = get_last_index(segmentTable) + 1;

            // Agrego el segmento a la lista
            list_add(segmentTable, new_segment);
            
            // Asocio el segmento al pcb
            p_info *info = malloc(sizeof(p_info));
            info -> id = patota -> pid;
            info -> isTask = 0; // false
            info -> nroSegmento = new_segment -> nroSegmento;

            // Guardo la info del pcb
            list_add(infoTable, info);
            
            // Agrego los datos a la memoria
            save_data(memory, new_segment, pcb -> id, total_size);

            // ------------ PCB TASKS ------------ //

            total_size = tasks_size;

            // Busco una direccion en memoria con el tamanio necesario para guardar el buffer
            addr = get_available_location(memory, total_size);
            
            // Creo el segmento asociado
            segment *task_segment = malloc(sizeof(segment));
            task_segment -> baseAddr = addr;
            task_segment -> limit = total_size;

            // Agrego el numero de segmento
            // Busco el ultimo indice que se agrego en la lista de segmentos y agrego uno mas
            task_segment -> nroSegmento = get_last_index(segmentTable) + 1;
            
            // Asocio el segmento al pcb
            p_info *info = malloc(sizeof(p_info));
            info -> id = patota -> pid;
            info -> isTask = 1; // true
            info -> nroSegmento = task_segment -> nroSegmento;

            // Guardo la info del pcb
            list_add(infoTable, info);
            
            // Agrego los datos a la memoria
            save_data(memory, task_segment, pcb -> tasks, total_size);

            // Devolver primer tarea
            int t_size;
            memcpy(&t_size, patota -> tasks, sizeof(int));
            char *task = malloc(t_size);
            memcpy(task, patota -> tasks + sizeof(int), t_size);

            // Limpiar la memoria de la primer task ?????
            // Actualizar el address del segmento ????

            // Actualizo el puntero qe devuelve las tareas.

            // Agrego el segmento a la lista
            list_add(segmentTable, task_segment);

            _send_message(fd, "RAM", INICIAR_TRIPULANTE, task, t_size);

        break;
        */
        case RECIBIR_UBICACION_TRIPULANTE:
            //ID_PATOTA, ID_TCB, POS_X, POS_Y

            log_info(logger, "entre al case rut"); 
            // Recibo id tcb
            char* idPCB = recibir_pbc(buffer);
            
            // Buscar la tabla de segmentos correspondiente al id de patota
            t_queue* segmentos = dictionary_get(diccionario, idPCB);

            // Busco la info del tcb por id, y busco el segmento con el nroSegmento
            int idTCB = recibir_tcb(buffer);
            segment* segmento = get_tcb_by_id(segmentos, idTCB);
            // Me traigo la info de memoria en un struct tcb
            tcb* nuestroTCB = get_tcb_from_memory(segmento);
            // actualizo los datos del struct
            /*typedef struct {
                uint32_t tid;
                uint32_t pid;
                char status;
                uint32_t xpos;
                uint32_t ypos;
                uint32_t next;
            } tcb;*/

            nuestroTCB->xpos = recibir_posicion(buffer);
            nuestroTCB->ypos = recibir_posicion(buffer);
            // Reemplazo los datos del tcb en la misma posicion de memoria
            guardar_tcb_memory(segmento, nuestroTCB, memory, memory_size);
            //// Actualizo el tcb en el mapa
            //actualizar_mapa(nuestroTCB);
            tcb_destroy(nuestroTCB); //free
            // Envio mensaje de OK
            _send_message(fd,"RAM", 200, "ok", 2, logger);

            break;
        case ENVIAR_PROXIMA_TAREA:
            //idpcb, idtcb
            // Buscar segmento de tcb
            log_info(logger, "entre al case rut"); 
            // Recibo id tcb
            char* idPCB = recibir_pbc(buffer);
            // Buscar la tabla de segmentos correspondiente al id de patota
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