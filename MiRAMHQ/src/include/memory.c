#include "memory.h"

void *memory_init(int size) {
    void *temp = malloc(size);
    return temp;
}

void memory_destroy(void *memory, t_list *segmentTable) {
    //TODO: Liberar los segmentos y la tabla de segmentos
    free(memory);
}

pcb *deserialize_pcb(void *buffer, int *tasks_size) {

}