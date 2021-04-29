#include "./handler.h"

// typedef struct {
//     uint32_t id;
//     uint32_t nroSegmento;
// } p_info;

// t_list *segmentTable;

// typedef struct {
//     uint32_t baseAddr;
//     uint32_t limit;
// } segment;

// typedef struct {
//     uint32_t pid;
//     int task_size;
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

pcb *deserialize_pcb(void *buffer) {
    pcb *patota = malloc(sizeof(pcb));
    int size;
    int offset = 0;

    memcpy(&patota -> pid, buffer + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    memcpy(&size, buffer + offset, sizeof(int));
    offset += sizeof(int);

    patota -> tasks = malloc(size);

    memcpy(patota->tasks, buffer + offset, size);

    retrun pcb;
}

void handler(int fd, char *id, int opcode, void *buffer, t_log *logger) {
    log_info(logger, "Recibi la siguiente operacion de %s: %d", id, opcode);

    switch (opcode){
        case INICIAR_TRIPULANTE:
            // Será el encargado de crear la o las estructuras administrativas necesarias para que un tripulante pueda ejecutar.
            // En caso de que no se encuentre creada la patota a la que pertenece, deberá solicitar el listado de tareas.

        case RECIBIR_TAREAS_PATOTA:
            pcb *patota = deserialize_pcb(buffer);


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