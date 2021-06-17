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
#include <commons/process.h>
#include<ctype.h>
#include<pthread.h>
#include <semaphore.h>
#include <math.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdint.h>
#include <commons/bitarray.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdarg.h>
#include <sys/types.h>
#include <dirent.h>

t_dictionary *global_tables;

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

// Pagi

typedef struct {
    uint32_t number;
    uint32_t start;
    uint8_t modified;
    uint8_t presence;
} frame_t;

typedef struct {
    frame_t *frame;
} page_t;


int page_size;

void *memory;
void *virtual_memory;

t_bitarray *bitmap;
t_bitarray *virtual_bitmap;

int err;

int frames_memory;
int frames_virtual;

// void mostrarInfo (void *element) {
//     p_info *info = element;
//     printf("ID: %d\n", info -> id);
//     printf("Type: %d\n", info -> type);
//     printf("Segment: %d\n", info -> nroSegmento);
// }

// char *get_segment_type(uint32_t segment_type) {
//     switch(segment_type) {
//         case PCB:
//             return "PCB";
//         break;
//         case TCB:
//             return "TCB";
//         break;
//         case TASK:
//             return "TASK";
//         break;
//     }

//     return "N/A";
// }

// void mostrarSegemento (void *element) {
//     segment *segmento = element;
//     printf("ID: %d\n", segmento -> id);
//     printf("Type: %s\n", get_segment_type(segmento -> type));
//     printf("Start: %d\n", segmento -> baseAddr);
//     printf("End: %d\n", segmento -> limit);
//     printf("Segment: %d\n\n", segmento -> nroSegmento);
// }

// void show_dictionary(t_dictionary *self) {

//     t_queue *aux;

//     // Recorro el diccionario
//     int table_index;

// 	for (table_index = 0; table_index < self->table_max_size; table_index++) {
// 		t_hash_element *element = self->elements[table_index];
// 		t_hash_element *next_element = NULL;

//         if (element != NULL)
//             printf("\nKEY: %s\n\n", element -> key);

// 		while (element != NULL) {

// 			next_element = element->next;

//             aux = element -> data;

//             list_iterate(aux -> elements, mostrarSegemento);

// 			element = next_element;
// 		}
// 	}

// }

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

// void *find_segment_by_number(t_list* self, int index) {
//     segment *temp;

// 	if ((self->elements_count > 0) && (index >= 0)) {

// 		t_link_element *element = self->head;
// 		temp = element -> data;

//         while (temp -> nroSegmento != index) {
// 			element = element->next;
//             if (element != NULL)
//                 temp = element -> data;
// 		}

// 		return element -> data;
// 	}
// 	return NULL;
// }

// int get_segment_limit(t_dictionary* self, int start) {

//     segment *temp;

//     t_queue *aux;

//     // Recorro el diccionario
//     int table_index;

// 	for (table_index = 0; table_index < self->table_max_size; table_index++) {
// 		t_hash_element *element = self->elements[table_index];
// 		t_hash_element *next_element = NULL;

// 		while (element != NULL) {

// 			next_element = element->next;

//             aux = element -> data;

//             if ((aux -> elements -> elements_count > 0) && (start >= 0)) {

//                 t_link_element *element = aux -> elements -> head;
//                 temp = element -> data;

//                 while (temp -> baseAddr != start && element != NULL) {

//                     element = element->next;
//                     if (element != NULL)
//                         temp = element -> data;
//                 }

//                 if (temp -> baseAddr == start)
//                     return temp -> limit;
//             }

// 			element = next_element;
// 		}
// 	}

//     return -1;
// }

// void destroyer(void *item) {
//     free(item);
// }

// void table_destroyer(void *item) {

//     queue_destroy_and_destroy_elements(item, destroyer);

// }

// void memory_compaction(void *memory, int mem_size, t_dictionary* self) {
//     void *aux_memory = malloc(mem_size);

//     segment *temp;

//     t_queue *aux;

//     int data_size;

//     int offset = 0;

//     int segment_id = 0;

//     int new_base;
//     int new_limit;

//     int table_index;

// 	for (table_index = 0; table_index < self->table_max_size; table_index++) {
// 		t_hash_element *element = self->elements[table_index];
// 		t_hash_element *next_element = NULL;

// 		while (element != NULL) {

// 			next_element = element->next;

//             aux = element -> data;

//             if ((aux -> elements -> elements_count > 0) && (mem_size >= 0)) {

//                 t_link_element *element = aux -> elements -> head;
//                 temp = element -> data;

//                 while (element != NULL) {

//                     data_size = temp -> limit - temp ->baseAddr;

//                     printf("Copiando elementos.. %d - %d - %d\n", temp -> nroSegmento, temp -> baseAddr, temp -> limit);
                    
//                     // Copio los datos del segmento en la memoria auxiliar
//                     memcpy(aux_memory + offset, memory + temp -> baseAddr, data_size);

//                     new_base = offset;
//                     new_limit = offset + data_size;

//                     printf("Creando nuevo Segmento.. %d - %d - %d\n", temp -> nroSegmento, new_base, new_limit);

//                     offset += data_size;

//                     temp -> baseAddr = new_base;
//                     temp -> limit = new_limit;

//                     element = element->next;
//                     if (element != NULL)
//                         temp = element -> data;
                    
//                 }
//             }
//             element = next_element;
//         }

// 	}

//     if (!dictionary_is_empty(self)) {
//         memset(memory, 0, mem_size);
//         memcpy(memory, aux_memory, mem_size);
//     }

//     free(aux_memory);
// }

// int memory_seek(void *memory, int mem_size, t_dictionary *collection, int total_size) {
//     // Auxiliar para guardar el limite de cada segmento ocupado
//     int limit;

//     // Contador de bytes libres en la memoria
//     int segment_counter = 0;

//     // Busco espacio libre en la memoria
//     for(int i = 0; i < mem_size; i ++) {
//         if (memcmp(memory + i, "\0", 1)) {
//             printf("Direccion ocupada: %d\n", i);
//             limit = get_segment_limit(collection, i);
//             if (limit < 0) return -1;
//             printf("Final de segmento: %d\n", limit);
//             i = limit - 1;
//             segment_counter = 0;
//         } else {
//             printf("Direccion vacia: %d\n", i);
//             segment_counter ++;
//             if (segment_counter == total_size) {
//                 printf("Encontre un segmento disponible: %d\n", i - (total_size - 1));
//                 return i - (total_size - 1);
//             }
//         }
//     }
//     return -1;
// }

// int get_last_index (t_queue *segmentTable) {

//     segment *temp;

//     if (segmentTable -> elements -> elements_count > 0) {

// 		t_link_element *element = segmentTable -> elements -> head;

//         while (element != NULL) {
//             temp = element -> data;
//             element = element -> next;
//         }

//         return temp -> nroSegmento;
//     }

//     return -1;
// }

// void *find_tcb_segment(int id, char *key, t_dictionary *table) {
//     segment *temp;

//     t_queue *self = dictionary_get(table, key);

// 	if ((self -> elements -> elements_count > 0) && (id >= 0)) {

// 		t_link_element *element = self -> elements -> head;
// 		temp = element -> data;

//         while (element != NULL) {

//             if (temp -> id == id && temp -> type == TCB) {
//                 return element -> data;
//             }

// 			element = element->next;
//             if (element != NULL)
//                 temp = element -> data;
// 		}
// 	}
// 	return NULL;
// }

// void *find_task_segment(char *key, t_dictionary *table) {
//     segment *temp;

//     t_queue *self = dictionary_get(table, key);

// 	if (self -> elements -> elements_count > 0) {

// 		t_link_element *element = self -> elements -> head;
// 		temp = element -> data;

//         while (element != NULL) {

//             if (temp -> type == TASK) {
//                 return element -> data;
//             }

// 			element = element->next;
//             if (element != NULL)
//                 temp = element -> data;
// 		}
// 	}
// 	return NULL;
// }

// void *find_pcb_segment(char *key, t_dictionary *table) {
//     segment *temp;

//     t_queue *self = dictionary_get(table, key);

// 	if (self -> elements -> elements_count > 0) {

// 		t_link_element *element = self -> elements -> head;
// 		temp = element -> data;

//         while (element != NULL) {

//             if (temp -> type == PCB) {
//                 return element -> data;
//             }

// 			element = element->next;
//             if (element != NULL)
//                 temp = element -> data;
// 		}
// 	}
// 	return NULL;
// }

char get_char_value(void *buffer, int index) {

    char temp;

    memcpy(&temp, buffer + index, 1);

    return temp;
}

// void *get_next_task(void *memory, int start_address, int limit_address) {

//     // printf("Values - Start: %d - End: %d\n", start_address, limit_address);

//     void *aux_tareas = malloc(limit_address - start_address + 1);
//     memcpy(aux_tareas, memory + start_address, limit_address - start_address);
//     memset(aux_tareas + limit_address, '\0', 1);

//     // printf("Lista: %s\n", aux_tareas);

//     int counter = 0;
    
//     // Get one byte of the memory as a CHAR
//     // char test_c = get_char_value(aux_tareas, counter);

//     while (memcmp(aux_tareas + counter, ";", 1) && aux_tareas + counter != NULL && counter < limit_address) {
//         // printf("CHAR: %c\n", get_char_value(aux_tareas, counter));
//         counter++;
//     }
//     while (!isalpha(get_char_value(aux_tareas, counter)) && aux_tareas + counter != NULL && counter < limit_address){
//         // printf("CHAR: %c\n", get_char_value(aux_tareas, counter));
//         counter++;
//     }

//     void *recv_task = malloc(counter + 1);
//     memcpy(recv_task, aux_tareas, counter);
//     memset(recv_task + counter, '\0', 1);

//     free(aux_tareas);

//     return recv_task;
// }

// int remove_segment_from_memory(void *memory, int mem_size, segment *segmento) {
//     if (segmento -> limit < mem_size) {
//         memset(memory + segmento -> baseAddr, 0 , segmento -> limit - segmento -> baseAddr);
//         return 1;
//     }

//     // printf("El segmento que se intento eliminar es invalido.\n");
//     return -1;
// }

// // typedef struct {
// //     uint32_t pid;
// //     uint32_t tasks;
// // } pcb;

// // typedef struct {
// //     uint32_t tid;
// //     uint32_t pid;
// //     char status;
// //     uint32_t xpos;
// //     uint32_t ypos;
// //     uint32_t next;
// // } tcb;

// int save_tcb_in_memory(void *memory, int mem_size, segment *segmento, tcb *data) {
//     int offset = 0;
//     if (segmento -> limit < mem_size) {
//         memcpy(memory + segmento -> baseAddr + offset, data -> tid, sizeof(uint32_t));
//         offset = sizeof(uint32_t);
//         memcpy(memory + segmento -> baseAddr + offset, data -> pid, sizeof(uint32_t));
//         offset = sizeof(uint32_t);
//         memcpy(memory + segmento -> baseAddr + offset, data -> status, sizeof(char));
//         offset = sizeof(char);
//         memcpy(memory + segmento -> baseAddr + offset, data -> xpos, sizeof(uint32_t));
//         offset = sizeof(uint32_t);
//         memcpy(memory + segmento -> baseAddr + offset, data -> ypos, sizeof(uint32_t));
//         offset = sizeof(uint32_t);
//         memcpy(memory + segmento -> baseAddr + offset, data -> next, sizeof(uint32_t));
//         offset = sizeof(uint32_t);

//         return 1;
//     }

//     return -1;
// }

// tcb *get_tcb_from_memory(void *memory, int mem_size, segment *segmento) {
//     tcb *temp = malloc(sizeof(tcb));

//     int offset = 0;

//     if (segmento -> limit < mem_size) {
//         memcpy(temp -> tid, memory + segmento -> baseAddr + offset, sizeof(uint32_t));
//         offset = sizeof(uint32_t);
//         memcpy(temp -> pid, memory + segmento -> baseAddr + offset, sizeof(uint32_t));
//         offset = sizeof(uint32_t);
//         memcpy(temp -> status, memory + segmento -> baseAddr + offset, sizeof(char));
//         offset = sizeof(char);
//         memcpy(temp -> xpos, memory + segmento -> baseAddr + offset, sizeof(uint32_t));
//         offset = sizeof(uint32_t);
//         memcpy(temp -> ypos, memory + segmento -> baseAddr + offset, sizeof(uint32_t));
//         offset = sizeof(uint32_t);
//         memcpy(temp -> next, memory + segmento -> baseAddr + offset, sizeof(uint32_t));
//         offset = sizeof(uint32_t);

//         return temp;
//     }

//     return NULL;
// }

// int save_pcb_in_memory(void *memory, int mem_size, segment *segmento, pcb *data) {
//     int offset = 0;
//     if (segmento -> limit < mem_size) {
//         memcpy(memory + segmento -> baseAddr + offset, data -> pid, sizeof(uint32_t));
//         offset = sizeof(uint32_t);
//         memcpy(memory + segmento -> baseAddr + offset, data -> tasks, sizeof(uint32_t));
//         offset = sizeof(uint32_t);

//         return 1;
//     }

//     return -1;
// }

// pcb *save_pcb_in_memory(void *memory, int mem_size, segment *segmento) {
//     int offset = 0;

//     pcb *temp = malloc(sizeof(pcb));
//     if (segmento -> limit < mem_size) {
//         memcpy(temp -> pid, memory + segmento -> baseAddr + offset, sizeof(uint32_t));
//         offset = sizeof(uint32_t);
//         memcpy(temp -> tasks, memory + segmento -> baseAddr + offset, sizeof(uint32_t));
//         offset = sizeof(uint32_t);

//         return temp;
//     }

//     return NULL;
// }

// // Hay que pasarle la lista completa de tareas, tal cual se guarda en memoria
// int save_task_in_memory(void *memory, int mem_size, segment *segmento, void *data) {

//     if (segmento -> limit < mem_size) {
//         memcpy(memory + segmento -> baseAddr, data, segmento -> limit - segmento -> baseAddr);

//         return 1;
//     }

//     return -1;
// }

uint32_t get_next_page(t_queue *page_table, uint32_t index) {

    page_t *page = list_get(page_table -> elements, index);

    return (page -> frame) -> start;
}
// 1     3     2    4    5
// [T1T][sss][2T3][fff][ggg]
//            678

// [4,3,TAREAS,4,5,6,7,8]

// [tarea6;8;4|tarea3;8;5, 0, 0]

// [0,1,2,3]



// [123123123, 0, 0, 0, 23424234234, 0, 0, 0]

// // copio todos los frames, y cuando voy a copiar el frame donde empiece la tarea, hago la traduccion de la dirreccion original a la direccion del buffer temporal

// int task_start_addr = 7

// int offset = 0;
// while(frame) {
//     memcpy(temp + offset, memory + frame -> start, frame -> size);
//     if (task_start_addr > frame -> start && task_start_addr < frame -> end) {
//         int off = task_start_addr - frame -> start
//         off += offset; // 4
//     }
//     offset += frame -> size; // 3 bytes
// }

// temp -> [T1T2T3sssfffggg]
//              4


void *search_task(void *memory, uint32_t start_addr) {
    //params?
    t_queue* page_table;
    frame_t* frame;

    uint32_t counter = start_addr;
    uint32_t start = start_addr;
    void *recv_task;
    uint32_t recv_start = 0;
    uint32_t page_counter = 1;

    if (start + 1 >= frame -> start + page_size) {
        recv_task = malloc(1);
        memcpy(recv_task + recv_start, memory + start, 1);
        recv_start++;
        counter = get_next_page(page_table, page_counter++);
        start = counter;
    }

    if (memcmp(memory + start + 1, "\0", 1)) {
        // printf("First step..");
        while (memcmp(memory + counter, ";", 1) && memory + counter != NULL) {
            counter++;

            if (counter >= frame -> start + page_size) {
                // COPIO LA MEMORIA QUE LEI HASTA EL MOMENTO
                memcpy(recv_task + recv_start, memory + start, counter);
                // BUSCO PROXIMO FRAME Y LO ASIGNO AL COUNTER
                counter = get_next_page(page_table, page_counter++);
                start = counter;
            } else {
                if (!memcmp(memory + counter, "\0", 1)){
                    break;
                }
            }
        }
        
        while (!isalpha(get_char_value(memory, counter)) && memory + counter != NULL && memcmp(memory + counter + 2, "\0", 1)){
            counter++;

            if (counter >= frame -> start + page_size) {
                // COPIO LA MEMORIA QUE LEI HASTA EL MOMENTO
                memcpy(recv_task + recv_start, memory + start, counter);
                // BUSCO PROXIMO FRAME Y LO ASIGNO AL COUNTER
                counter = get_next_page(page_table, page_counter++);
                start = counter;
            } else {
                if (!memcmp(memory + counter, "\0", 1)){
                    break;
                }
            }
        }
        
        if (memcmp(memory + counter + 2, "\0", 1)) {
            global_index = counter;
        } else {
            global_index = counter + 1;
        }

        memcpy(recv_task, memory + start, counter);
        memset(recv_task + counter, '\0', 1);

        return recv_task;
    } else {

        return NULL;
    }
    // printf("\nLast Index: %d\n", counter);
}

int s_tcb = 16;

typedef struct {
    t_log *logger;
    uint32_t current;
} t_data;

sem_t s_main;
sem_t *t_sem;

void _thread_function(t_data *data) {
    while(1) {

        sem_wait(&t_sem[data -> current]);

        int t_id = process_get_thread_id();

        int p_id = process_getpid();

        log_info(data -> logger, "Process ID: %d - Thread ID: %d", p_id, t_id);

        if (data -> current + 1 >= s_tcb) {
            sem_post(&s_main);
        } else {
            sem_post(&t_sem[data -> current + 1]);
        }
    }
}

// pagi

int check_free_frames(int frames_count) {
    int counter = 0;

    // Busco frames libres en el bitmap y agrego a un counter
    for (int i = 0; i < frames_memory; i++) {
        if (!bitarray_test_bit(bitmap, i)) {
            counter++;
        }
        if(counter >= frames_count) {
            return 1;
        }
    }

    // lo sumo a los frames libres en disco
    for (int j = 0; j < frames_virtual; j++) {
        if (!bitarray_test_bit(virtual_bitmap, j)) {
            counter++;
        }
        if(counter >= frames_count) {
            return 1;
        }
    }

    return 0;

}

uint32_t get_frame() {
    for (int i = 0; i < frames_memory; i++) {
        if (!bitarray_test_bit(bitmap, i)) {
            return i;
        }
    }
    // Si salgo del for, significa que no encontro ningun frame libre en la memoria principal, por lo que se debe hacer un SWAP
    // muevo un frame al virtual (seleccionado por algoritmo)
    // seteo el bit del memoria real en cero
    // devuelvo la posicion de ese bit en memoria real
}

void save_data_in_memory(void *buffer) {

    void *temp;
    pcb *p_aux = malloc(sizeof(pcb));
    int task_size;
    int tcb_count;
    int offset = 0;

    p_aux -> tasks = 8; // Al leer un frame, las tareas siempre comienzan en la posicion 8 del frame leido (paginacion)
    memcpy(&p_aux -> pid, buffer + offset, sizeof(uint32_t));
    offset += sizeof(int);
    
    memcpy(&task_size, buffer + offset, sizeof(int));
    offset += sizeof(int);

    void *tasks = malloc(task_size);
    memcpy(tasks, buffer + offset, task_size);
    offset += task_size;

    memcpy(&tcb_count, buffer + offset, sizeof(int));
    offset += sizeof(int);

    int memory_size = sizeof(pcb) + task_size + tcb_count * sizeof(tcb);

    int frames_count = ceil(memory_size / page_size);

    temp = malloc(memory_size);
    int temp_off = 0;

    memcpy(temp + temp_off, &p_aux -> pid, sizeof(uint32_t));
    temp_off += sizeof(uint32_t);

    memcpy(temp + temp_off, &p_aux -> tasks, sizeof(uint32_t));
    temp_off += sizeof(uint32_t);

    // Me guardo el ID del pcb como string para el diccionario
    char *pid = string_itoa(p_aux -> pid);
    int p_id = p_aux -> pid;
    free(p_aux);

    memcpy(temp + temp_off, tasks, task_size);
    temp_off += task_size;

    free(tasks);

    for( int i = 0; i < tcb_count; i++) {
        memcpy(temp + temp_off, buffer + offset, sizeof(uint32_t));
        temp_off += sizeof(uint32_t);
        offset += sizeof(uint32_t);
        
        memcpy(temp + temp_off, buffer + offset, sizeof(uint32_t));
        temp_off += sizeof(uint32_t);
        offset += sizeof(uint32_t);
        
        memcpy(temp + temp_off, buffer + offset, sizeof(char));
        temp_off += sizeof(char);
        offset += sizeof(char);
        
        memcpy(temp + temp_off, buffer + offset, sizeof(uint32_t));
        temp_off += sizeof(uint32_t);
        offset += sizeof(uint32_t);
        
        memcpy(temp + temp_off, buffer + offset, sizeof(uint32_t));
        temp_off += sizeof(uint32_t);
        offset += sizeof(uint32_t);
        
        memcpy(temp + temp_off, buffer + offset, sizeof(uint32_t));
        temp_off += sizeof(uint32_t);
        offset += sizeof(uint32_t);
    }

    // ---------------- GUARDO FRAMES ---------------- //

    // Busco si hay frames libres en memoria/disco
    if (check_free_frames(frames_count)) {

        // Creo tabla de paginas
        t_queue *tabla = queue_create();

        // Si hay, guardo la data
        int bytes_left = memory_size;

        for (int j = 0; j < frames_count; j++) {

            uint32_t n_frame = get_frame(); // actualiza el bit en el bitmap y devuelve el indice
            bitarray_set_bit(bitmap, n_frame);
            
            // Creo frame
            frame_t *frame = malloc(sizeof(frame_t));
            frame -> number = n_frame;
            frame -> start = n_frame * page_size;
            frame -> modified = 1;
            frame -> presence = 1;

            // Creo pagina
            page_t *page = malloc(sizeof(page_t));
            page -> frame = frame;

            if (bytes_left < page_size) {
                // copio bytes_left
                memcpy(memory + (n_frame * page_size), temp, bytes_left);
            } else {
                // copio page_size
                memcpy(memory + (n_frame * page_size), temp, page_size);
                bytes_left -= page_size;
            }

            queue_push(tabla, page);
        }

        // Agrego tabla al diccionario
        dictionary_put(global_tables, pid, tabla);

    } else {
        printf("No hay espacio disponible en memoria\n");
    }

    free(temp);
    free(pid);

}

int main() {

    t_log *logger = log_create("../logs/test.log", "TEST", 1, LOG_LEVEL_TRACE);

    // --------------- CREATE BITMAPS ---------------- //

    // ---------- REAL MEMORY SETUP ---------- //
    
    int real_size = 80;
    memory = malloc(real_size);
    page_size = 10;

    frames_memory = real_size / page_size;

    bitmap = bitarray_create_with_mode((char *) memory, frames_memory / 8, MSB_FIRST);

    for(int i = 0; i < frames_memory; i++){
        bitarray_clean_bit(bitmap, i);
    }

    log_info(logger, "Muestro valores del bitmap para REAL..");
    for(int i = 0; i < frames_memory; i++){
        log_info(logger, "Bit %d: %d", i, bitarray_test_bit(bitmap, i));
    }

    // ---------- VIRTUAL MEMORY SETUP ---------- //

    int virtual_size = 160;

    char *path = "./virtual.mem";

    frames_virtual = virtual_size / page_size;

    int arch_bitmap;

    if( access( path, F_OK ) == 0 ) {
        
        arch_bitmap = open(path, O_CREAT | O_RDWR, 0664);

        virtual_memory = mmap(NULL, virtual_size, PROT_READ | PROT_WRITE, MAP_SHARED, arch_bitmap, 0);

        // Creo el bitmap seguido del espacio de memoria para disco
        virtual_bitmap = bitarray_create_with_mode((char *) virtual_memory + virtual_size, frames_virtual / 8, MSB_FIRST);

        log_info(logger, "Muestro valores del bitmap para VIRTUAL..");
        for(int i = 0; i < frames_virtual; i++){
            log_info(logger, "Bit %d: %d", i, bitarray_test_bit(virtual_bitmap, i));
        }

        char *test = malloc(page_size + 1);
        memcpy(test, virtual_memory + (2 * page_size), page_size);
        test[page_size] = '\0';
        log_info(logger, "TEST: %s", test);
        free(test);

        log_info(logger, "5 frames libres?: %d", check_free_frames(5));
        log_info(logger, "23 frames libres?: %d", check_free_frames(23));
        log_info(logger, "24 frames libres?: %d", check_free_frames(24));

    } else {

        arch_bitmap = open(path, O_CREAT | O_RDWR, 0664);
        posix_fallocate(arch_bitmap, 0, virtual_size + frames_virtual / 8);

        virtual_memory = mmap(NULL, virtual_size + frames_virtual / 8, PROT_READ | PROT_WRITE, MAP_SHARED, arch_bitmap, 0);

        // Creo el bitmap seguido del espacio de memoria para disco
        virtual_bitmap = bitarray_create_with_mode((char *) virtual_memory + virtual_size, frames_virtual / 8, MSB_FIRST);

        for(int i = 0; i < frames_virtual; i++){
            bitarray_clean_bit(virtual_bitmap, i);
            // err = msync(virtual_bitmap -> bitarray, frames_virtual / 8, MS_ASYNC);
            // if (err == -1){
            //     log_error(logger, "Error de sincronizar a disco clean bitmap");
            // }
        }

        log_info(logger, "Muestro valores INICIALES del bitmap para VIRTUAL..");
        for(int i = 0; i < frames_virtual; i++){
            log_info(logger, "Bit %d: %d", i, bitarray_test_bit(virtual_bitmap, i));
        }

        bitarray_set_bit(virtual_bitmap, 2);
        // err = msync(virtual_bitmap -> bitarray, frames_virtual / 8, MS_ASYNC);
        // if (err == -1){
        //     log_error(logger, "Error de sincronizar a disco bitmap");
        // }
        memcpy(virtual_memory + (2 * page_size), "0123456789", page_size);
        err = msync(virtual_memory, virtual_size + frames_virtual / 8, MS_ASYNC);
        if (err == -1){
            log_error(logger, "Error de sincronizar a disco memoria");
        }

        log_info(logger, "Muestro valores del bitmap para VIRTUAL..");
        for(int i = 0; i < frames_virtual; i++){
            log_info(logger, "Bit %d: %d", i, bitarray_test_bit(virtual_bitmap, i));
        }
    }

    close(arch_bitmap);

    free(memory);

    munmap(virtual_memory, virtual_size);

    bitarray_destroy(bitmap);

    bitarray_destroy(virtual_bitmap);

    // //////////////////////////// GONZA //////////////////////////////////// //

//     void inicializar_superbloque()
// {
//     superbloque = open(SUPERBLOQUE_PATH, O_RDWR, (mode_t)0777); // Intenta abrir el archivo con permisos de lectura y escritura.
//     if (superbloque == -1)
//     {
//         superbloque = open(SUPERBLOQUE_PATH, O_CREAT | O_RDWR, (mode_t)0777); // Si no existe el archivo lo crea con permisos de lectura y escritura.

//         uint32_t block_size = config_get_int_value(config_file, "BLOCK_SIZE");
//         uint32_t block_count = config_get_int_value(config_file, "BLOCKS");
//         void *segmento_bitmap = malloc(block_count / 8);
//         t_bitarray *bitmap = bitarray_create_with_mode((char *)segmento_bitmap, block_count / 8, LSB_FIRST);
//         for (int i = 0; i < block_count; i++)
//         {
//             bitarray_clean_bit(bitmap, i);
//         }

//         ftruncate(superbloque, 2 * sizeof(uint32_t) + block_count / 8);

//         uint32_t offset = 0;
//         void *contenido_superbloque = malloc(2 * sizeof(uint32_t) + block_count / 8);
//         memcpy(contenido_superbloque + offset, &block_size, sizeof(uint32_t));
//         offset += sizeof(uint32_t);
//         memcpy(contenido_superbloque + offset, &block_count, sizeof(uint32_t));
//         offset += sizeof(uint32_t);
//         memcpy(contenido_superbloque + offset, bitmap->bitarray, block_count / 8);

//         void *contenido_archivo = mmap(NULL, 2 * sizeof(uint32_t) + block_count / 8, PROT_READ | PROT_WRITE, MAP_SHARED, superbloque, 0); // Mapear todo el superbloque
//         if (contenido_archivo == MAP_FAILED)
//         {
//             printf("Error al mapear el archivo superbloque. Errno: %d.\n", errno);
//         }

//         memcpy(contenido_archivo, contenido_superbloque, 2 * sizeof(uint32_t) + block_count / 8);
//         msync(contenido_archivo, 2 * sizeof(uint32_t) + block_count / 8, MS_SYNC);

//         free(segmento_bitmap);
//         bitarray_destroy(bitmap);
//         free(contenido_superbloque);
//         close(superbloque);
//         log_info(log_file, "Archivo SuperBloque creado correctamente.\n");
//     }
//     else
//     {
//         uint32_t block_size = config_get_int_value(config_file, "BLOCK_SIZE");
//         uint32_t block_count = config_get_int_value(config_file, "BLOCKS");

//         void *contenido_archivo = mmap(NULL, 2 * sizeof(uint32_t) + block_count / 8, PROT_READ | PROT_WRITE, MAP_SHARED, superbloque, 0);

//         uint32_t offset = 0;
//         memcpy(&block_size, contenido_archivo + offset, sizeof(uint32_t));
//         offset += sizeof(uint32_t);
//         memcpy(&block_count, contenido_archivo + offset, sizeof(uint32_t));
//         offset += sizeof(uint32_t);
//         void *segmento_bitmap = malloc(block_count / 8);
//         memcpy(segmento_bitmap, contenido_archivo + offset, block_count / 8);

//         t_bitarray *bitmap = bitarray_create_with_mode((char *)segmento_bitmap, block_count / 8, LSB_FIRST);

//         free(segmento_bitmap);
//         bitarray_destroy(bitmap);
//         close(superbloque);
//         log_info(log_file, "Archivo SuperBloque abierto correctamente.\n");
//     }
// }

    // //////////////////////////// GONZA //////////////////////////////////// //

    // --------------- TEST DATA SPLIT --------------- //

    // // Test data split in memory
    // void *memory = malloc (100);
    // void *temp = malloc(4);
    // int number = 1455;
    // printf("number: %d\n", number);
    // memcpy(temp, &number, 4);

    // memcpy(memory, temp, 3);
    // memcpy(memory + 40, temp + 1, 1);

    // free(temp);

    // // join data from memory

    // void *temp2 = malloc(4);
    // int second;

    // memcpy(temp2, memory, 3);
    // memcpy(temp2 + 1, memory + 40, 1);

    // memcpy(&second, temp2, 4);

    // printf("second: %d\n", second);

    // free(temp2);
    // free(memory);

    // --------------- TEST SYNC THREADS --------------- //
    // // sem_destroy(&sem);

    // t_sem = malloc(sizeof(sem_t) * s_tcb);

    // //Inicializo los semaforos
    // sem_init(&s_main, 0, 1);

    // for (int j = 0; j < s_tcb; j++) {
    //     sem_init(&t_sem[j], 0, 0);
    // }

    // for (int i = 0; i < s_tcb; i++) {

    //     t_data *data = malloc(sizeof(t_data));
    //     data -> logger = logger;
    //     data -> current = i;

    //     pthread_t thread;
    //     pthread_create(&thread, NULL, (void *) _thread_function, data);
    //     pthread_detach(thread);
    // }

    // // Hilo principal
    // while(1) {
    //     sem_wait(&s_main);

    //     log_info(logger, "Dejo correr los threads..");
    //     sleep(2);

    //     sem_post(&t_sem[0]);
    // }

    // --------------- TEST PAGINCACION --------------- //

    // int offset = 0;

    // tcb *temp = malloc(sizeof(tcb));

    // temp -> tid = 1;
    // temp -> pid = 2;
    // temp -> status = 'N';
    // temp -> xpos = 3;
    // temp -> ypos = 4;
    // temp -> next = 5;

    // pcb *patota = malloc(sizeof(pcb));

    // patota -> pid = 1;

    // char *tareas = "GENERAR_OXIGENO 12;3;2;5CONSUMIR_OXIGENO 120;2;3;1";

    // int size = sizeof(tcb) + sizeof(int) * 3 + strlen(tareas);

    // void *buffer = _serialize(
    //     size,
    //     "%d%s%s%d%d%c%d%d%d",
    //     patota -> pid,
    //     tareas,
    //     1, // Cantidad tcbs
    //     temp -> tid,
    //     temp -> pid,
    //     temp -> status,
    //     temp -> xpos,
    //     temp -> ypos,
    //     temp -> next
    // );

    // // paso el buffer que llega desde cliente
    // save_data_in_memory(buffer);

    // -------------

    // memcpy(memory + offset, tareas, strlen(tareas));
    // offset += strlen(tareas);
    // memcpy(memory + offset, &temp -> tid, sizeof(uint32_t));
    // offset += sizeof(uint32_t);
    // memcpy(memory + offset, &temp -> pid, sizeof(uint32_t));
    // offset += sizeof(uint32_t);
    // memcpy(memory + offset, &temp -> status, sizeof(char));
    // offset += sizeof(char);
    // memcpy(memory + offset, &temp -> xpos, sizeof(uint32_t));
    // offset += sizeof(uint32_t);
    // memcpy(memory + offset, &temp -> ypos, sizeof(uint32_t));
    // offset += sizeof(uint32_t);
    // memcpy(memory + offset, &temp -> next, sizeof(uint32_t));
    // offset += sizeof(uint32_t);

    // // ---------- TEST GET TASK IN PAGINATION

    // void *tarea = search_task(memory, 0);

    // if (tarea) {
    //     printf("Task: %s\n", tarea);
    //     free(tarea);
    // } else {
    //     printf("No hay mas tareas\n");
    // }

    // tarea = search_task(memory, global_index);

    // if (tarea) {
    //     printf("Task: %s\n", tarea);
    //     free(tarea);
    // } else {
    //     printf("No hay mas tareas\n");
    // }

    // tarea = search_task(memory, global_index);

    // if (tarea) {
    //     printf("Task: %s\n", tarea);
    //     free(tarea);
    // } else {
    //     printf("No hay mas tareas\n");
    // }
    
    // // End test

    // free(temp);

    // // dictionary_destroy_and_destroy_elements(table_collection, table_destroyer);


    // -------------- TEST SERIALIZACION -------------- //

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

    // free(memory);

    log_destroy(logger);

    return 0;
}
