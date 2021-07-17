#include "memory.h"

void *memory_init(int size) {
    void *temp = malloc(size);
    return temp;
}


// ----------------------- PAGINATION ------------------------- //





// --------------------- END PAGINATION ----------------------- //

// --------------------- SEGMENTATION ----------------------- //


// void mostrarInfo (void *element) {
//     p_info *info = element;
//     printf("ID: %d\n", info -> id);
//     printf("Type: %d\n", info -> type);
//     printf("Segment: %d\n", info -> nroSegmento);
// }

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

// void *find_info_by_id(t_list* self, int id) {
//     p_info *temp;
// 	if ((self->elements_count > 0) && (id >= 0)) {

// 		t_link_element *element = self->head;
// 		temp = element -> data;

//         while (temp -> id != id) {
// 			element = element->next;
//             if (element != NULL)
//                 temp = element -> data;
// 		}

// 		return element -> data;
// 	}
// 	return NULL;
// }

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

void memory_compaction(void *admin, void *memory, int mem_size, t_dictionary* self) {
    void *aux_memory = malloc(mem_size);
    memset(admin, 0, mem_size);

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
                    memset(admin + offset, 1, data_size);

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

int check_space_memory(void *admin, int mem_size, int total_size, t_dictionary *table_collection) {

    if (total_size > mem_size) {
        return 0;
    }

    // Contador de bytes libres en la memoria
    int segment_counter = 0;

    for(int i = 0; i < mem_size; i ++) {
        if (!memcmp(admin + i, "\0", 1)) {

            // printf("Direccion vacia: %d\n", i);
            segment_counter ++;
            if (segment_counter == total_size) {
                // printf("Hay espacio disponible en memoria..\n");
                return 1;
            }
        }
    }
    return 0;
}

typedef struct {
    int addr;
    int counter;

} best_fit_data;

bool segment_cmp(void *first, void *second) {
    best_fit_data *f = (best_fit_data *) first;
    best_fit_data *s = (best_fit_data *) second;

    printf("Comparando: %d - %d\n", f -> counter, s -> counter);

    return f -> counter < s -> counter;
}

void best_fit_destroyer(void *data) {
    best_fit_data *temp = (best_fit_data *) data;
    
    free(temp);
}

int memory_best_fit(void *admin, int mem_size, t_dictionary *collection, int total_size) {

    t_list *temp = list_create();

    int result;

    int start = 0;

    int j;

    // Contador de bytes libres en la memoria
    int segment_counter = 0;

    // Busco espacio libre en la memoria
    for(int i = 0; i < mem_size; i ++) {
        if (!memcmp(admin + i, "\0", 1)) {
            // printf("Segmento Libre en: %d\n", i);
            start = i;
            j = i;
            while(j < mem_size && !memcmp(admin + j, "\0", 1)) {
                // printf("siguiente en: %d\n", j);
                segment_counter ++;
                j++;
            }
            i = j;
            // printf("Data: %d - %d - %d\n", i, segment_counter, mem_size);
            if (i <= mem_size) {
                if (segment_counter >= total_size) {
                    best_fit_data *data = malloc(sizeof(best_fit_data));
                    data -> addr = start;
                    data -> counter = segment_counter;
                    list_add_sorted(temp, data, segment_cmp);
                }

                segment_counter = 0;
            }

        }

    }

    if (list_size(temp) > 0) {
        best_fit_data *aux = list_get(temp, 0);
        result = aux -> addr;
        list_destroy_and_destroy_elements(temp, best_fit_destroyer);
        // printf("Direccion a devolver: %d\n", result);
        return result;
    }

    return -1;
}


int memory_seek(void *admin, int mem_size, int total_size, t_dictionary *table_collection) {

    // Contador de bytes libres en la memoria
    int segment_counter = 0;

    int start;
    int j;

    // Busco espacio libre en la memoria
    for(int i = 0; i < mem_size; i ++) {
        if (!memcmp(admin + i, "\0", 1)) {
            // printf("Segmento Libre en: %d\n", i);
            start = i;
            j = i;
            while(!memcmp(admin + j, "\0", 1) && j < mem_size) {
                // printf("siguiente en: %d\n", j);
                segment_counter ++;
                j++;

                if (segment_counter >= total_size) {
                    // printf("Direccion a devolver: %d\n", start);
                    return start;
                }
            }
            i = j;

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

void *get_next_task(void *memory, int start_address, int limit_address,t_log* logger) {

    // printf("Values - Start: %d - End: %d\n", start_address, limit_address);
    if (start_address >= limit_address) {
        return NULL;
    }
    log_info(logger, "Limit address:%d", limit_address);

    void *tareas = malloc(limit_address - start_address + 1);
    memcpy(tareas, memory + start_address, limit_address - start_address);
    memset(tareas + (limit_address-start_address), '\0', 1);

    printf("Lista: %s\n",(char*) tareas);
    int cantidadLetrasLeidas = 0;
    
    // TOMAR AIRE;2,2,T|HCONSUMIR OXIGENO P;3;3;T|HOLA

    // Get one byte of the memory as a CHAR
    // char test_c = get_char_value(tareas, counter);
    int offset = 0;
    while (memcmp(tareas + offset, ";", 1) && tareas + offset != NULL && cantidadLetrasLeidas < limit_address ) {
        // printf("CHAR: %c\n", get_char_value(tareas, cantidadLetrasLeidas));
        
        if(get_char_value(tareas,offset) != '\n'){
            cantidadLetrasLeidas++;
        }

        offset++;
    }
    if(primeraVez=0 /*idea no implementada*/){
        cantidadLetrasLeidas--;
    }
    
    log_info(logger, "Cantidad letras hasta primer ;%d",cantidadLetrasLeidas);

    while (!isalpha(get_char_value(tareas, cantidadLetrasLeidas)) && cantidadLetrasLeidas < limit_address){
        printf("CHAR: %c\n", get_char_value(tareas, cantidadLetrasLeidas));

        offset++;
    }
    cantidadLetrasLeidas--;
    log_info(logger, "Cantidad letras despues de segundo while: %d",cantidadLetrasLeidas);

    void *recv_task = malloc(cantidadLetrasLeidas + 1);
    memcpy(recv_task, tareas, cantidadLetrasLeidas);
    memset(recv_task + cantidadLetrasLeidas, '\0', 1);

    free(tareas);
    
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

int save_tcb_in_memory(void *admin, void *memory, int mem_size, segment *segmento, tcb_t *data) {
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

        memset(admin + segmento -> baseAddr, 1, segmento -> limit - segmento -> baseAddr);

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

int save_pcb_in_memory(void *admin, void *memory, int mem_size, segment *segmento, pcb_t *data) {
    int offset = 0;
    if (segmento -> limit < mem_size) {
        memcpy(memory + segmento -> baseAddr + offset, &(data -> pid), sizeof(uint32_t));
        offset = sizeof(uint32_t);
        memcpy(memory + segmento -> baseAddr + offset, &(data -> tasks), sizeof(uint32_t));
        offset = sizeof(uint32_t);

        memset(admin + segmento -> baseAddr, 1, segmento -> limit - segmento -> baseAddr);

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
int save_task_in_memory(void *admin, void *memory, int mem_size, segment *segmento, void *data) {

    if (segmento -> limit < mem_size) {
        memcpy(memory + segmento -> baseAddr, data, segmento -> limit - segmento -> baseAddr);
        memset(admin + segmento -> baseAddr, 1, segmento -> limit - segmento -> baseAddr);
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

void remove_segment_from_table(t_dictionary* table_collection, char *key, segment *segmento) {
    segment *temp;

    int index = 0;

    t_queue *self = dictionary_get(table_collection, key);

	if (self -> elements -> elements_count > 0) {

		t_link_element *element = self -> elements -> head;
		temp = element -> data;

        while (element != NULL) {

            if (temp -> nroSegmento == segmento -> nroSegmento) {
                // return element -> data;
                break;
            }

			element = element->next;

            if (element != NULL){
                index ++;
                temp = element -> data;
            }
		}

        list_remove_and_destroy_element(self -> elements, index, destroyer);
	}

}

segment *get_next_segment(t_dictionary *table_collection, char* key) {
    segment *temp;

    t_queue *self = dictionary_get(table_collection, key);

	if (self -> elements -> elements_count > 0) {

		t_link_element *element = self -> elements -> head;
		temp = element -> data;

        if (element != NULL) {
            return temp;
        }
	}

    return NULL;
}

void remove_pcb_from_memory(void *memory, int mem_size, t_dictionary *table_collection, char* key) {
    printf("Key: %s\n", key);
    segment *temp;

    int index = 0;

    t_queue *self = dictionary_get(table_collection, key);

    printf("CANTIDAD ELEMENTOS: %d\n", self -> elements -> elements_count);

	if (self -> elements -> elements_count > 0) {

		t_link_element *element = self -> elements -> head;
		temp = element -> data;

        while (element != NULL) {

            // Segment
            // element -> data;
            printf("Data: %d", temp -> type);

			element = element->next;

            if (element != NULL){
                index ++;
                temp = element -> data;
            }
		}
	}
}

// --------------------- END SEGMENTATION ----------------------- //


// --------------------- DUMP ----------------------- //

int get_page_number(t_dictionary *self, uint32_t frame) {

    t_queue *aux;

    // Recorro el diccionario
    int table_index;

    int index;


	for (table_index = 0; table_index < self->table_max_size; table_index++) {
		t_hash_element *element = self->elements[table_index];
		t_hash_element *next_element = NULL;

		while (element != NULL) {

			next_element = element->next;

            aux = element -> data;

            // page_iterate(aux -> elements, save_in_file, memory, file, index, element -> key);
            // ITERACION DE LISTA

            t_link_element *elementl = (aux -> elements)->head;
            t_link_element *aux2 = NULL;

            index = 0;

            while (elementl != NULL) {
                aux2 = elementl->next;
                page_t *page = elementl -> data;
                if((page -> frame) -> number == frame && (page -> frame) -> presence) {
                    global_process = string_new();
                    string_append(&global_process, element -> key);
                    global_page = index;
                    return 1;
                }
                elementl = aux2;
                index++;
            }

            // END INTERATION

			element = next_element;
		}
	}

    return  1;

}

void page_dump(t_dictionary *table) {

    //  Dump_<Timestamp>.dmp
    char *timestamp = temporal_get_string_time("%d-%m-%y");
    char *file_name = string_new();
    string_append(&file_name, "./Dump_");
    string_append(&file_name, timestamp);
    string_append(&file_name, ".dmp");

    FILE* file = fopen(file_name, "w");

    if(file == NULL)
    {
        perror("Error al abrir archivo Dump");
    }

    free(timestamp);
    free(file_name);

    txt_write_in_file(file, "--------------------------------------------------------------------------\n");
    char *title = string_new();
    char *date = temporal_get_string_time("%d/%m/%y %H:%M:%S");
    string_append_with_format(&title, "Dump: %s\n", date);
    txt_write_in_file(file, title);
    free(date);
    free(title);
    
    for (int i = 0; i < frames_memory; i++) {
        char *line = string_new(); 
        
        if (bitmap[i]) {
            if(get_page_number(table, i)) {
                string_append_with_format(&line, "Marco: %d\t\tEstado: %s\t\tProceso: %s\t\tPagina: %d\n", i, "Ocupado", global_process, global_page);
                free(global_process); 
            }
        } else {
            string_append_with_format(&line, "Marco: %d\t\tEstado: %s\t\tProceso: %s\t\tPagina: %s\n", i, "Libre", "-", "-");
        }
        
        txt_write_in_file(file, line);

        free(line);
    }

    txt_write_in_file(file, "--------------------------------------------------------------------------\n");

    txt_close_file(file);
}

void save_in_file (void *element, void *memory, FILE *file) {
    segment *segmento = element;

    char *line = string_new();
    string_append_with_format(&line, "Proceso: %d\t\tSegmento: %d\t\tInicio: %p\t\tTam: %db\n", segmento -> id, segmento -> nroSegmento, (memory + segmento -> baseAddr), segmento -> limit - segmento -> baseAddr);
    
    txt_write_in_file(file, line);

    free(line);

}

void process_iterate(t_list *self, void(*closure)(), void *memory, FILE *file) {
    t_link_element *element = self->head;
	t_link_element *aux = NULL;
	while (element != NULL) {
		aux = element->next;
		closure(element->data, memory, file);
		element = aux;
	}
}

void memory_dump(t_dictionary *self, void *memory) {

    //  Dump_<Timestamp>.dmp
    char *timestamp = temporal_get_string_time("%d-%m-%y");
    char *file_name = string_new();
    string_append(&file_name, "./Dump_");
    string_append(&file_name, timestamp);
    string_append(&file_name, ".dmp");

    FILE* file = fopen(file_name, "w");

    if(file == NULL)
    {
        perror("Error al abrir archivo Dump");
    }

    free(timestamp);
    free(file_name);

    t_queue *aux;

    // Recorro el diccionario
    int table_index;
    txt_write_in_file(file, "--------------------------------------------------------------------------\n");
    char *title = string_new();
    char *date = temporal_get_string_time("%d/%m/%y %H:%M:%S");
    string_append_with_format(&title, "Dump: %s\n", date);
    txt_write_in_file(file, title);
    free(date);
    free(title);


	for (table_index = 0; table_index < self->table_max_size; table_index++) {
		t_hash_element *element = self->elements[table_index];
		t_hash_element *next_element = NULL;

		while (element != NULL) {

			next_element = element->next;

            aux = element -> data;

            process_iterate(aux -> elements, save_in_file, memory, file);

			element = next_element;
		}
	}

    txt_write_in_file(file, "--------------------------------------------------------------------------\n");

    txt_close_file(file);
}


// --------------------- END DUMP ----------------------- //