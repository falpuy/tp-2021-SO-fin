#include "memory.h"

void *memory_init(int size) {
    void *temp = malloc(size);
    return temp;
}


// ----------------------- PAGINATION ------------------------- //





// --------------------- END PAGINATION ----------------------- //

// --------------------- SEGMENTATION ----------------------- //


void mostrarInfo (void *element) {
    p_info *info = element;
    printf("ID: %d\n", info -> id);
    printf("Type: %d\n", info -> type);
    printf("Segment: %d\n", info -> nroSegmento);
}

char *get_segment_type(uint32_t segment_type) {
    switch(segment_type) {
        case PCB:
            return "PCB";
        break;
        case TCB:
            return "TCB";
        break;
        case TASK:
            return "TASK";
        break;
    }

    return "N/A";
}

void mostrarSegemento (void *element) {
    segment *segmento = element;
    printf("ID: %d\n", segmento -> id);
    printf("Type: %s\n", get_segment_type(segmento -> type));
    printf("Start: %d\n", segmento -> baseAddr);
    printf("End: %d\n", segmento -> limit);
    printf("Segment: %d\n\n", segmento -> nroSegmento);
}

void show_dictionary(t_dictionary *self) {

    t_queue *aux;

    // Recorro el diccionario
    int table_index;

	for (table_index = 0; table_index < self->table_max_size; table_index++) {
		t_hash_element *element = self->elements[table_index];
		t_hash_element *next_element = NULL;

        if (element != NULL)
            printf("\nKEY: %s\n\n", element -> key);

		while (element != NULL) {

			next_element = element->next;

            aux = element -> data;

            list_iterate(aux -> elements, mostrarSegemento);

			element = next_element;
		}
	}

}

void *find_info_by_id(t_list* self, int id) {
    p_info *temp;
	if ((self->elements_count > 0) && (id >= 0)) {

		t_link_element *element = self->head;
		temp = element -> data;

        while (temp -> id != id) {
			element = element->next;
            if (element != NULL)
                temp = element -> data;
		}

		return element -> data;
	}
	return NULL;
}

void *find_segment_by_number(t_list* self, int index) {
    segment *temp;

	if ((self->elements_count > 0) && (index >= 0)) {

		t_link_element *element = self->head;
		temp = element -> data;

        while (temp -> nroSegmento != index) {
			element = element->next;
            if (element != NULL)
                temp = element -> data;
		}

		return element -> data;
	}
	return NULL;
}

int get_segment_limit(t_dictionary* self, int start) {

    segment *temp;

    t_queue *aux;

    // Recorro el diccionario
    int table_index;

	for (table_index = 0; table_index < self->table_max_size; table_index++) {
		t_hash_element *element = self->elements[table_index];
		t_hash_element *next_element = NULL;

		while (element != NULL) {

			next_element = element->next;

            aux = element -> data;

            if ((aux -> elements -> elements_count > 0) && (start >= 0)) {

                t_link_element *element = aux -> elements -> head;
                temp = element -> data;

                while (temp -> baseAddr != start && element != NULL) {

                    element = element->next;
                    if (element != NULL)
                        temp = element -> data;
                }

                if (temp -> baseAddr == start)
                    return temp -> limit;
            }

			element = next_element;
		}
	}

    return -1;
}

void destroyer(void *item) {
    free(item);
}

void table_destroyer(void *item) {

    queue_destroy_and_destroy_elements(item, destroyer);

}

void memory_compaction(void *memory, int mem_size, t_dictionary* self) {
    void *aux_memory = malloc(mem_size);

    segment *temp;

    t_queue *aux;

    int data_size;

    int offset = 0;

    int new_base;
    int new_limit;

    int table_index;

	for (table_index = 0; table_index < self->table_max_size; table_index++) {
		t_hash_element *element = self->elements[table_index];
		t_hash_element *next_element = NULL;

		while (element != NULL) {

			next_element = element->next;

            aux = element -> data;

            if ((aux -> elements -> elements_count > 0) && (mem_size >= 0)) {

                t_link_element *element = aux -> elements -> head;
                temp = element -> data;

                while (element != NULL) {

                    data_size = temp -> limit - temp ->baseAddr;

                    printf("Copiando elementos.. %d - %d - %d\n", temp -> nroSegmento, temp -> baseAddr, temp -> limit);
                    
                    // Copio los datos del segmento en la memoria auxiliar
                    memcpy(aux_memory + offset, memory + temp -> baseAddr, data_size);

                    new_base = offset;
                    new_limit = offset + data_size;

                    printf("Creando nuevo Segmento.. %d - %d - %d\n", temp -> nroSegmento, new_base, new_limit);

                    offset += data_size;

                    temp -> baseAddr = new_base;
                    temp -> limit = new_limit;

                    element = element->next;
                    if (element != NULL)
                        temp = element -> data;
                    
                }
            }
            element = next_element;
        }

	}

    if (!dictionary_is_empty(self)) {
        memset(memory, 0, mem_size);
        memcpy(memory, aux_memory, mem_size);
    }

    free(aux_memory);
}

int check_space_memory(void *memory, int mem_size, int total_size, t_dictionary *table_collection) {
    // Auxiliar para guardar el limite de cada segmento ocupado
    int limit;

    // Contador de bytes libres en la memoria
    int segment_counter = 0;

    for(int i = 0; i < mem_size; i ++) {
        if (memcmp(memory + i, "\0", 1)) {
            printf("Direccion ocupada: %d\n", i);
            limit = get_segment_limit(table_collection, i);
            if (limit < 0) return -1;
            printf("Final de segmento: %d\n", limit);
            i = limit - 1;
        } else {
            printf("Direccion vacia: %d\n", i);
            segment_counter ++;
            if (segment_counter == total_size) {
                printf("Encontre un segmento disponible: %d\n", i - (total_size - 1));
                return 1;
            }
        }
    }
    return -1;
}

int memory_seek(void *memory, int mem_size, int total_size, t_dictionary *table_collection) {
    // Auxiliar para guardar el limite de cada segmento ocupado
    int limit;

    // Contador de bytes libres en la memoria
    int segment_counter = 0;

    // Busco espacio libre en la memoria
    for(int i = 0; i < mem_size; i ++) {
        if (memcmp(memory + i, "\0", 1)) {
            printf("Direccion ocupada: %d\n", i);
            limit = get_segment_limit(table_collection, i);
            if (limit < 0) return -1;
            printf("Final de segmento: %d\n", limit);
            i = limit - 1;
            segment_counter = 0;
        } else {
            printf("Direccion vacia: %d\n", i);
            segment_counter ++;
            if (segment_counter == total_size) {
                printf("Encontre un segmento disponible: %d\n", i - (total_size - 1));
                return i - (total_size - 1);
            }
        }
    }
    return -1;
}

int get_last_index (t_queue *segmentTable) {

    segment *temp;

    if (segmentTable -> elements -> elements_count > 0) {

		t_link_element *element = segmentTable -> elements -> head;

        while (element != NULL) {
            temp = element -> data;
            element = element -> next;
        }

        return temp -> nroSegmento;
    }

    return -1;
}

void *find_tcb_segment(int id, char *key, t_dictionary *table) {
    segment *temp;

    t_queue *self = dictionary_get(table, key);

	if ((self -> elements -> elements_count > 0) && (id >= 0)) {

		t_link_element *element = self -> elements -> head;
		temp = element -> data;

        while (element != NULL) {

            if (temp -> id == id && temp -> type == TCB) {
                return element -> data;
            }

			element = element->next;
            if (element != NULL)
                temp = element -> data;
		}
	}
	return NULL;
}

void *find_task_segment(char *key, t_dictionary *table) {
    segment *temp;

    t_queue *self = dictionary_get(table, key);

	if (self -> elements -> elements_count > 0) {

		t_link_element *element = self -> elements -> head;
		temp = element -> data;

        while (element != NULL) {

            if (temp -> type == TASK) {
                return element -> data;
            }

			element = element->next;
            if (element != NULL)
                temp = element -> data;
		}
	}
	return NULL;
}

void *find_pcb_segment(char *key, t_dictionary *table) {
    segment *temp;

    t_queue *self = dictionary_get(table, key);

	if (self -> elements -> elements_count > 0) {

		t_link_element *element = self -> elements -> head;
		temp = element -> data;

        while (element != NULL) {

            if (temp -> type == PCB) {
                return element -> data;
            }

			element = element->next;
            if (element != NULL)
                temp = element -> data;
		}
	}
	return NULL;
}

char get_char_value(void *buffer, int index) {

    char temp;

    memcpy(&temp, buffer + index, 1);

    return temp;
}

void *get_next_task(void *memory, int start_address, int limit_address) {

    // printf("Values - Start: %d - End: %d\n", start_address, limit_address);
    if (start_address >= limit_address) {
        return NULL;
    }

    void *aux_tareas = malloc(limit_address - start_address + 1);
    memcpy(aux_tareas, memory + start_address, limit_address - start_address);
    memset(aux_tareas + limit_address, '\0', 1);

    // printf("Lista: %s\n", aux_tareas);

    int counter = 0;
    
    // Get one byte of the memory as a CHAR
    // char test_c = get_char_value(aux_tareas, counter);

    while (memcmp(aux_tareas + counter, ";", 1) && aux_tareas + counter != NULL && counter < limit_address) {
        // printf("CHAR: %c\n", get_char_value(aux_tareas, counter));
        counter++;
    }
    while (!isalpha(get_char_value(aux_tareas, counter)) && aux_tareas + counter != NULL && counter < limit_address){
        // printf("CHAR: %c\n", get_char_value(aux_tareas, counter));
        counter++;
    }

    void *recv_task = malloc(counter + 1);
    memcpy(recv_task, aux_tareas, counter);
    memset(recv_task + counter, '\0', 1);

    free(aux_tareas);

    return recv_task;
}

int remove_segment_from_memory(void *memory, int mem_size, segment *segmento) {
    if (segmento -> limit < mem_size) {
        memset(memory + segmento -> baseAddr, 0 , segmento -> limit - segmento -> baseAddr);
        return 1;
    }

    // printf("El segmento que se intento eliminar es invalido.\n");
    return -1;
}

// typedef struct {
//     uint32_t pid;
//     uint32_t tasks;
// } pcb;

// typedef struct {
//     uint32_t tid;
//     uint32_t pid;
//     char status;
//     uint32_t xpos;
//     uint32_t ypos;
//     uint32_t next;
// } tcb;

int save_tcb_in_memory(void *memory, int mem_size, segment *segmento, tcb_t *data) {
    int offset = 0;
    if (segmento -> limit < mem_size) {
        memcpy(memory + segmento -> baseAddr + offset, &(data -> tid), sizeof(uint32_t));
        offset = sizeof(uint32_t);
        memcpy(memory + segmento -> baseAddr + offset, &(data -> pid), sizeof(uint32_t));
        offset = sizeof(uint32_t);
        memcpy(memory + segmento -> baseAddr + offset, &(data -> status), sizeof(char));
        offset = sizeof(char);
        memcpy(memory + segmento -> baseAddr + offset, &(data -> xpos), sizeof(uint32_t));
        offset = sizeof(uint32_t);
        memcpy(memory + segmento -> baseAddr + offset, &(data -> ypos), sizeof(uint32_t));
        offset = sizeof(uint32_t);
        memcpy(memory + segmento -> baseAddr + offset, &(data -> next), sizeof(uint32_t));
        offset = sizeof(uint32_t);

        return 1;
    }

    return -1;
}

tcb_t *get_tcb_from_memory(void *memory, int mem_size, segment *segmento) {
    tcb_t *temp = malloc(sizeof(tcb_t));

    int offset = 0;

    if (segmento -> limit < mem_size) {
        memcpy(&(temp -> tid), memory + segmento -> baseAddr + offset, sizeof(uint32_t));
        offset = sizeof(uint32_t);
        memcpy(&(temp -> pid), memory + segmento -> baseAddr + offset, sizeof(uint32_t));
        offset = sizeof(uint32_t);
        memcpy(&(temp -> status), memory + segmento -> baseAddr + offset, sizeof(char));
        offset = sizeof(char);
        memcpy(&(temp -> xpos), memory + segmento -> baseAddr + offset, sizeof(uint32_t));
        offset = sizeof(uint32_t);
        memcpy(&(temp -> ypos), memory + segmento -> baseAddr + offset, sizeof(uint32_t));
        offset = sizeof(uint32_t);
        memcpy(&(temp -> next), memory + segmento -> baseAddr + offset, sizeof(uint32_t));
        offset = sizeof(uint32_t);

        return temp;
    }

    return NULL;
}

int save_pcb_in_memory(void *memory, int mem_size, segment *segmento, pcb_t *data) {
    int offset = 0;
    if (segmento -> limit < mem_size) {
        memcpy(memory + segmento -> baseAddr + offset, &(data -> pid), sizeof(uint32_t));
        offset = sizeof(uint32_t);
        memcpy(memory + segmento -> baseAddr + offset, &(data -> tasks), sizeof(uint32_t));
        offset = sizeof(uint32_t);

        return 1;
    }

    return -1;
}

pcb_t *get_pcb_from_memory(void *memory, int mem_size, segment *segmento) {
    int offset = 0;

    pcb_t *temp = malloc(sizeof(pcb_t));
    if (segmento -> limit < mem_size) {
        memcpy(&(temp -> pid), memory + segmento -> baseAddr + offset, sizeof(uint32_t));
        offset = sizeof(uint32_t);
        memcpy(&(temp -> tasks), memory + segmento -> baseAddr + offset, sizeof(uint32_t));
        offset = sizeof(uint32_t);

        return temp;
    }

    return NULL;
}

// Hay que pasarle la lista completa de tareas, tal cual se guarda en memoria
int save_task_in_memory(void *memory, int mem_size, segment *segmento, void *data) {

    if (segmento -> limit < mem_size) {
        memcpy(memory + segmento -> baseAddr, data, segmento -> limit - segmento -> baseAddr);

        return 1;
    }

    return -1;
}

// void send_tareas(int id_pcb, char *ruta_archivo) {
//     FILE * fp;
//     char * line = NULL;
//     size_t len = 0;
//     ssize_t read;

//     int b_size = 0;
//     int offset = 0;
//     int new_size;
//     void *temp;

//     void *buffer = malloc(sizeof(int));

//     memcpy(buffer + offset, &id_pcb, sizeof(int));
//     offset += sizeof(int);
//     b_size += sizeof(int);

//     fp = fopen(ruta_archivo, "r");
//     if (fp == NULL)
//         exit(EXIT_FAILURE);

//     while ((read = getline(&line, &len, fp)) != -1) {

//         // printf("Length: %d - String: %s", read, line);

//         if (line[ read - 1 ] == '\n') {
//             read--;
//             memset(line + read, 0, 1);
//         }

//         new_size = sizeof(int) + read;
        
//         temp = _serialize(new_size, "%s", line);

//         b_size += new_size;
//         buffer = realloc(buffer, b_size);
        
//         memcpy(buffer + offset, temp, new_size);
//         offset += new_size;

//         free(temp);
//     }

//     fclose(fp);
//     if (line)
//         free(line);

//     // _send_message(socket_memoria, "DIS", 510, buffer, offset, logger);

//     free(buffer);
// }

// --------------------- END SEGMENTATION ----------------------- //
