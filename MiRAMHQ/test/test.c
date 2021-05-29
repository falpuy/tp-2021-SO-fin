#include <stdio.h>
#include <stdlib.h>
#include <unnamed/socket.h>
#include <unnamed/serialization.h>
#include <commons/log.h>
#include <commons/collections/list.h>
#include <commons/collections/queue.h>
#include <commons/collections/dictionary.h>
#include <commons/string.h>
#include <commons/error.h>
#include <commons/memory.h>
#include<ctype.h>

int status = 1;
int recvCounter = 0;

// void *handler(int socket) {
//   char *proceso;

//   log_info(logger, "Esperando mensajes..");
//   while(status) {
//     while (recvCounter--) {
//         proceso = malloc(3);
//         if (recv(socket, proceso, 3, 0) > 0) {
//             proceso[3] = '\0';
//         }
//     }
//   }
// }

enum tipo_segmento {
    PCB,
    TCB,
    TASK
};

typedef struct {
    uint32_t id;
    uint32_t type;
    uint32_t nroSegmento;
} p_info;

typedef struct {
    uint32_t id;
    uint32_t type;
    uint32_t nroSegmento;
    uint32_t baseAddr;
    uint32_t limit;
} segment;

typedef struct {
    uint32_t pid;
    uint32_t tasks;
} pcb;

typedef struct {
    uint32_t tid;
    uint32_t pid;
    char status;
    uint32_t xpos;
    uint32_t ypos;
    uint32_t next;
} tcb;

uint32_t global_index;
uint32_t global_segment;
uint32_t global_type;

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

    int segment_id = 0;

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

int memory_seek(void *memory, int mem_size, t_dictionary *collection, int total_size) {
    // Auxiliar para guardar el limite de cada segmento ocupado
    int limit;

    // Contador de bytes libres en la memoria
    int segment_counter = 0;

    // Busco espacio libre en la memoria
    for(int i = 0; i < mem_size; i ++) {
        if (memcmp(memory + i, "\0", 1)) {
            printf("Direccion ocupada: %d\n", i);
            limit = get_segment_limit(collection, i);
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

int save_tcb_in_memory(void *memory, int mem_size, segment *segmento, tcb *data) {
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

tcb *get_tcb_from_memory(void *memory, int mem_size, segment *segmento) {
    tcb *temp = malloc(sizeof(tcb));

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

int save_pcb_in_memory(void *memory, int mem_size, segment *segmento, pcb *data) {
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

pcb *get_pcb_from_memory(void *memory, int mem_size, segment *segmento) {
    int offset = 0;

    pcb *temp = malloc(sizeof(pcb));
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

void send_tareas(int id_pcb, char *ruta_archivo) {
    FILE * fp;
    char * line = NULL;
    size_t len = 0;
    ssize_t read;

    int b_size = 0;
    int offset = 0;
    int new_size;
    void *temp;

    void *buffer = malloc(sizeof(int));

    memcpy(buffer + offset, &id_pcb, sizeof(int));
    offset += sizeof(int);
    b_size += sizeof(int);

    fp = fopen(ruta_archivo, "r");
    if (fp == NULL)
        exit(EXIT_FAILURE);

    while ((read = getline(&line, &len, fp)) != -1) {

        // printf("Length: %d - String: %s", read, line);

        if (line[ read - 1 ] == '\n') {
            read--;
            memset(line + read, 0, 1);
        }

        new_size = sizeof(int) + read;
        
        temp = _serialize(new_size, "%s", line);

        b_size += new_size;
        buffer = realloc(buffer, b_size);
        
        memcpy(buffer + offset, temp, new_size);
        offset += new_size;

        free(temp);
    }

    fclose(fp);
    if (line)
        free(line);

    _send_message(socket_memoria, "DIS", 510, buffer, offset);

    free(buffer);
}

int main() {

    t_log *logger = log_create("../logs/test.log", "TEST", 1, LOG_LEVEL_TRACE);

    // ---------------- TEST MEMORY SPACE ----------------- //

    // int m_size = 10;
    // void *memory = malloc(m_size);

    // int temp = 8;
    // int temp2 = 7;

    // memcpy(memory, &temp, sizeof(int));
    // // memcpy(memory + 1, &temp2, sizeof(int));
    
    // int start = 0;
    // int end = 0;

    // // Delete space allocated by data in hq memory
    // // memset(memory + start, '\0', end - start);

    // for(int i = 0; i < m_size; i ++) {
    //     if (memcmp(memory + i, "\0", 1)) {
    //         printf("Direccion ocupada: %d\n", i);
    //     } else {
    //         printf("Direccion vacia: %d\n", i);
    //     }
    // }

    // int otro;
    // memcpy(&otro, memory, sizeof(int));

    // printf("Traje de memoria: %d", otro);

    // ---------------- TEST MEMORY SEEK & MEMORY COMPACTION WITH DICTIONARY ----------------- //

    /*

    int m_size = 30;
    void *memory = malloc(m_size);

    t_queue *segmentTable1 = queue_create();
    t_queue *segmentTable2 = queue_create();

    t_dictionary *table_collection = dictionary_create();

    int temp = 8;

    segment *uno = malloc(sizeof(segment));
    segment *dos = malloc(sizeof(segment));
    segment *tres = malloc(sizeof(segment));
    segment *cuatro = malloc(sizeof(segment));

    uno -> nroSegmento = get_last_index (segmentTable1) + 1;
    uno -> baseAddr = 1;
    uno -> limit = 5;
    uno -> id = 1;
    uno -> type = PCB;

    memcpy(memory + uno -> baseAddr, &temp, uno -> limit - uno -> baseAddr);

    queue_push(segmentTable1, uno);

    dos -> nroSegmento = get_last_index (segmentTable1) + 1;
    dos -> baseAddr = 10;
    dos -> limit = 14;
    dos -> id = 1;
    dos -> type = TCB;

    temp = 10;
    memcpy(memory + dos -> baseAddr, &temp, dos -> limit - dos -> baseAddr);

    queue_push(segmentTable1, dos);

    tres -> nroSegmento = get_last_index (segmentTable2) + 1;
    tres -> baseAddr = 15;
    tres -> limit = 19;
    tres -> id = 2;
    tres -> type = PCB;

    temp = 12;
    memcpy(memory + tres -> baseAddr, &temp, tres -> limit - tres -> baseAddr);

    queue_push(segmentTable2, tres);

    cuatro -> nroSegmento = get_last_index (segmentTable2) + 1;
    cuatro -> baseAddr = 22;
    cuatro -> limit = 26;
    cuatro -> id = 1;
    cuatro -> type = TASK;
    
    temp = 14;
    memcpy(memory + cuatro -> baseAddr, &temp, cuatro -> limit - cuatro -> baseAddr);

    queue_push(segmentTable2, cuatro);

    dictionary_put(table_collection, "1", segmentTable1);
    dictionary_put(table_collection, "2", segmentTable2);

    printf("\n------- Printing Dictionary -------\n");

    show_dictionary(table_collection);
    
    printf("\n------- ------------------- -------\n");

    // Tamanio del segmento que quiero guardar
    int total_size = 6;

    printf("Buscando un segmento de tamanio: %d\n", total_size);
    
    // Valida si encontre un segmento libre o no
    int found_segment = memory_seek(memory, m_size, table_collection, total_size);

    if(found_segment < 0) {
        printf("No encontre ningun segmento libre.. Iniciando compactacion.\n");

        memory_compaction(memory, m_size, table_collection);

        printf("Buscando un segmento de tamanio: %d\n", total_size);

        found_segment = memory_seek(memory, m_size, table_collection, total_size);
    }

    // char *mem_hexstring(void *source, size_t length);
    // void mem_hexdump(void *source, size_t length);
    printf("\nProceso: %d\t\tSegmento: %d\t\tInicio: %p\t\tTam: %db", tres -> id, tres -> nroSegmento, (memory + tres -> baseAddr), tres -> limit - tres -> baseAddr);
    
    // printf("\n\n--- Testing mem_hexstring() ---");

    // char *test_hex = mem_hexstring(memory + tres -> baseAddr, tres -> limit - tres -> baseAddr);
    
    // printf("\n%s", test_hex);

    // printf("\n\n--- Testing mem_hexdump() ---\n");

    // mem_hexdump(memory + tres -> baseAddr, tres -> limit - tres -> baseAddr);

    dictionary_destroy_and_destroy_elements(table_collection, table_destroyer);
    // queue_destroy_and_destroy_elements(segmentTable, destroyer);

    free(memory);

    */

    // ---------------- TEST QUEUES ----------------- //

    /*

    // t_queue *listaInfo = queue_create();
    t_queue *segmentTable = queue_create();

    segment *segmento = malloc(sizeof(segment));
    segmento -> id = 1;
    segmento -> type = PCB;
    segmento -> nroSegmento = 1;
    segmento -> baseAddr = 0;
    segmento -> limit = 14;
    
    segment *segmento2 = malloc(sizeof(segment));
    segmento2 -> id = 1;
    segmento2 -> type = TCB;
    segmento2 -> nroSegmento = 2;
    segmento2 -> baseAddr = 14;
    segmento2 -> limit = 20;
    
    segment *segmento3 = malloc(sizeof(segment));
    segmento3 -> id = 1;
    segmento3 -> type = TASK;
    segmento3 -> nroSegmento = 3;
    segmento3 -> baseAddr = 20;
    segmento3 -> limit = 32;

    // No conviene usar p_info porque los id de TCB se pueden repetir al igual que el nro de segmento
    // En mejor agregar el id al segmento y filtrar por procesos en el diccionario

    // p_info *info = malloc(sizeof(p_info));
    // info -> nroSegmento = 1;
    // info -> id = 10;
    // info -> type = PCB;

    // p_info *info = malloc(sizeof(p_info));
    // info -> nroSegmento = 8;
    // info -> id = 1;
    // info -> type = TCB;

    // queue_push(listaInfo, info);
    queue_push(segmentTable, segmento);
    queue_push(segmentTable, segmento2);
    queue_push(segmentTable, segmento3);

    t_dictionary *table = dictionary_create();

    // char *index = string_new();
    char *index = string_itoa(segmento -> id);

    dictionary_put(table, index, segmentTable);

    // list_iterate(listaInfo -> elements, mostrarInfo);
    // list_iterate(segmentTable -> elements, mostrarSegemento);

    show_dictionary(table);

    // p_info *infoTest = find_info_by_id(listaInfo -> elements, 0);

    // Parametros -> KEY de la tabla de segmentos en el diccionario (ID PCB), ID del objeto buscado (PCB|TCB), tipo de dato a buscar (PCB|TCB|TASK), diccionario
    // segment *segmentTest = find_segment_by_id(id_pcb, id_tcb, TCB, table);
    segment *segmentTCB = find_tcb_segment(1, index, table);

    segment *segmentTask = find_task_segment(index, table);

    segment *segmentPCB = find_pcb_segment(index, table);

    printf("Segmento PCB: %d\n", segmentPCB -> baseAddr);
    printf("Segmento TASK: %d\n", segmentTask -> baseAddr);
    printf("Segmento TCB: %d\n", segmentTCB -> baseAddr);


    free(index);
    // free(info);
    // free(segmento);
    // free(segmento2);

    // queue_destroy(listaInfo);
    dictionary_destroy_and_destroy_elements(table, table_destroyer);

    */

    // ---------------- TEST ARCHIVOS ----------------- //

    send_tareas(41, "./tareas.txt");
/*

    FILE * fp;
    char * line = NULL;
    size_t len = 0;
    ssize_t read;

    int b_size = 0;
    int offset = 0;
    int new_size;
    void *temp;

    void *buffer = malloc(sizeof(int));

    int id_pcb = 21;

    memcpy(buffer + offset, &id_pcb, sizeof(int));
    offset += sizeof(int);
    b_size += sizeof(int);

    fp = fopen("./tareas.txt", "r");
    if (fp == NULL)
        exit(EXIT_FAILURE);

    while ((read = getline(&line, &len, fp)) != -1) {

        // printf("Length: %d - String: %s", read, line);

        if (line[ read - 1 ] == '\n') {
            read--;
            memset(line + read, 0, 1);
        }

        new_size = sizeof(int) + read;
        
        temp = _serialize(new_size, "%s", line);

        b_size += new_size;
        buffer = realloc(buffer, b_size);
        
        memcpy(buffer + offset, temp, new_size);
        offset += new_size;

        free(temp);
    }

    fclose(fp);
    if (line)
        free(line);

    // Agregar id de pcb al que pertenece
    // _send(buffer, offset);

    // Prueba Deserializacion

    char *tarea;
    int size_tarea;
    int off = 0;
    int i = 0;

    int test_id;

    char *lista_tareas = string_new();

    memcpy(&test_id, buffer + off, sizeof(int));
    off += sizeof(int);

    log_info(logger, "ID PCB: %d", test_id);

    while (off < offset) {

        memcpy(&size_tarea, buffer + off, sizeof(int));
        off += sizeof(int);

        tarea = malloc(size_tarea + 1);
        memcpy(tarea, buffer + off, size_tarea);
        off += size_tarea;
        tarea[size_tarea] = '\0';

        log_info(logger, "Tarea %d - len %d: %s", i++, size_tarea, tarea);

        string_append(&lista_tareas, tarea);

        free(tarea);

    }

    free(buffer);

    log_info(logger, "Lista final en memoria: %s", lista_tareas);

    // -------- Recorrer las tareas en memoria

    // GENERAR_OXIGENO 12;3;2;5CONSUMIR_OXIGENO 120;2;3;1GENERAR_COMIDA 4;2;3;1CONSUMIR_COMIDA 1;2;3;4GENERAR_BASURA 12;2;3;5DESCARTAR_BASURA 10;3;1;7
    int start_task = 0;
    char *recv_task = get_next_task(lista_tareas, start_task, strlen(lista_tareas));

    log_info(logger, "Tarea: %s - Size: %d", recv_task, strlen(recv_task));
    
    // free(recv_task);

    start_task = strlen(recv_task);
     free(recv_task);
    recv_task = get_next_task(lista_tareas, start_task, strlen(lista_tareas));

    log_info(logger, "Tarea: %s - Size: %d", recv_task, strlen(recv_task));
    
    free(recv_task);

    free(lista_tareas);
*/

    // -------------- TEST SERIALIZACION -------------- //
    

    // void *buffer;
    // int b_size;

    // char *str = "HOLA!!!";
    // b_size = sizeof(int) + strlen(str);
    // buffer = _serialize(b_size, "%s", str);

    // char *str2 = "CHAU!!!";
    // int b_size2 = sizeof(int) + strlen(str2);
    // buffer = realloc(buffer, b_size + b_size2);
    // void * temp = _serialize(b_size2, "%s", str2);
    // memcpy(buffer + b_size, temp, b_size2);
    

    // log_info(logger, "Deserializando...");

    // char *otroBuffer;
    // int offset = 0;
    // int size;

    // memcpy(&size, buffer + offset, sizeof(int));
    // offset += sizeof(int);
    // log_info(logger, "Size primer string: %d", size);

    // otroBuffer = malloc(size + 1);
    // memcpy(otroBuffer, buffer + offset, size);
    // offset += size;
    // otroBuffer[offset] = '\0';

    // log_info(logger, "String: %s", otroBuffer);
    // free(otroBuffer);

    // memcpy(&size, buffer + offset, sizeof(int));
    // log_info(logger, "Size 2 string: %d - %d|%d", size, offset, b_size);
    // offset += sizeof(int);

    // otroBuffer = malloc(size + 1);
    // memcpy(otroBuffer, buffer + offset, size);
    // offset += size;
    // otroBuffer[offset] = '\0';

    // log_info(logger, "String 2: %s", otroBuffer);
    // free(otroBuffer);

    // free (temp);
    // free(buffer);
    
    //  ------------------------- TEST CASES

    // void *buffer;
    // int b_size;

    // char *str = "HOLA!!!";
    // b_size = sizeof(int) + strlen(str);
    // buffer = _serialize(b_size, "%s", str);
    // free(buffer);

    // b_size = sizeof(int);
    // buffer = _serialize(b_size, "%d", 25);
    // free(buffer);

    // b_size = sizeof(char);
    // buffer = _serialize(b_size, "%c", 'Z');
    // free(buffer);

    // b_size = sizeof(double);
    // buffer = _serialize(b_size, "%f", 100.33494);
    // free(buffer);

    // b_size = sizeof(uint32_t);
    // buffer = _serialize(b_size, "%u", 32);
    // free(buffer);

    // // Error de formato
    // b_size = sizeof(uint32_t);
    // buffer = _serialize(b_size, "%d%s%p%k", 32);
    // free(buffer);

    

    // ---------------- TEST CONEXION ----------------- //

    // int socket_memoria =  _connect("127.0.0.1", "5001", logger);

    // _send_message(socket_memoria, "DIS", 999, "asdasdasdasd", strlen("asdasdasdasd"), logger);

    // t_mensaje *mensaje = _receive_message(socket_memoria, logger);

    // free(mensaje -> identifier);
    // free(mensaje -> payload);
    // free(mensaje);

    // close(socket_memoria);

    // -------------------------------------------- //

    log_destroy(logger);

    return 0;
}

// int main()
// {
//     // char str[] = "comando blabla [1,2,3,4,5] bla bla";
//     // remove_all_chars(str, '[');
//     // remove_all_chars(str, ']');
//     void *str = malloc(strlen("STRING DE PRUEBBA\0"));

//     memcpy(str, "STRING DE PRUEBBA\0", strlen("STRING DE PRUEBBA\0"));

//     printf("str '%s'\n", str);
    
//     int start = 3;
//     int end = 6;

//     // Delete space allocated by data in hq memory
//     memset(str + start, '\0', end - start);

//     printf("str '%s'\n", str + end);

//     for(int i = 0; i < strlen("STRING DE PRUEBBA\0"); i ++) {
//         if (memcmp(str + i, "\0", 1)) {
//             printf("CHAR: %s\n", (char*)&(str[i]));
//         } else {
//             printf("EMPTY: %s\n", (char*)&(str[i]));
//         }
//     }

//     // Search for free spaces
//     // for -> primero elemento vacio
//     // for -> validar qe haya elementos vacios desde el primero q matchea
//     // hhasta el tamanio necesario
//     // si se encuentra una valor distinto de vacio en el medio de la segunda busqueda
//     // colocar el primer for a partir de este valor y seguir buscando hhasta finalizar

//     // Trim all values in memory
//     // Use segment table to iterate the memory
//     // Copy all values inside a new buffer next to each other
//     // delete the memory buffer and set the new values

//     free(str);
//     return 0;
// }