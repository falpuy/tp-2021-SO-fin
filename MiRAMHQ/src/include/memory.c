#include "memory.h"

void *memory_init(int size) {
    void *temp = malloc(size);
    return temp;
}

void memory_destroy(void *memory, t_list *segmentTable) {
    //TODO: Liberar los segmentos y la tabla de segmentos
    free(memory);
}

pcb *deserialize_pcb(void *buffer, int tasks_size) {
    pcb *patota = malloc(sizeof(pcb));
    int offset = 0;

    memcpy(&patota -> pid, buffer + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    memcpy(&tasks_size, buffer + offset, sizeof(int));
    offset += sizeof(int);

    patota -> tasks = malloc(tasks_size);

    memcpy(patota->tasks, buffer + offset, tasks_size);

    return patota;
}