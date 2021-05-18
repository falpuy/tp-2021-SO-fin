#include "./handler.h"

// typedef struct {
//     uint32_t id;
//     uint32_t isTask;
//     uint32_t nroSegmento;
// } p_info;

// t_list *infoTable;
// t_list *segmentTable;

// typedef struct {
//     uint32_t nroSegmento;
//     uint32_t baseAddr;
//     uint32_t limit;
// } segment;

// typedef struct {
//     uint32_t pid;
//     void *tasks;
// } pcb;

// typedef struct {
//     uint32_t tid;
//     uint32_t pid;
//     char status;
//     uint32_t xpos;
//     uint32_t ypos;
//     uint32_t next;
// } tcb;

void handler(int fd, char *id, int opcode, void *buffer, t_log *logger) {
    log_info(logger, "Recibi la siguiente operacion de %s: %d", id, opcode);

    switch (opcode){
        case INICIAR_TRIPULANTE:
            // Será el encargado de crear la o las estructuras administrativas necesarias para que un tripulante pueda ejecutar.
            // En caso de que no se encuentre creada la patota a la que pertenece, deberá solicitar el listado de tareas.

            // Creo el tripulante

            // Busco la patota en la tabla de segmentos

            // Si existe, devuelvo la primer tarea y actualizo la memoria????

            // Si no existe, hago un send para pedir las tareas

        break;
        /*
        case RECIBIR_TAREAS_PATOTA:
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
        case ENVIAR_PROXIMA_TAREA:
        case EXPULSAR_TRIPULANTE:
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