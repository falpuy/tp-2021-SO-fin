#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unnamed/socket.h>
#include <unnamed/serialization.h>
#include <commons/log.h>
#include <string.h>
#include <unistd.h>
#include <commons/string.h>
#include <stdint.h>
#include <commons/bitarray.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdarg.h>
#include <sys/types.h>
#include <dirent.h>

// Globals
void *blocks_memory;
void *bitmap_memory;

#define _FSDIR_ "./fs"

<<<<<<< HEAD
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

int memory_best_fit(void *memory, int mem_size, t_dictionary *collection, int total_size) {
    // Auxiliar para guardar el limite de cada segmento ocupado
    int limit;

    t_list *temp = list_create();

    int result;

    int start = 0;

    // Contador de bytes libres en la memoria
    int segment_counter = 0;

    // Busco espacio libre en la memoria
    for(int i = 0; i < mem_size; i ++) {
        if (memcmp(memory + i, "\0", 1)) {
            printf("Direccion ocupada: %d\n", i);
            if (segment_counter >= total_size) {
                printf("Encontre un segmento: %d - %d\n", start, segment_counter);
                best_fit_data *data = malloc(sizeof(best_fit_data));
                data -> addr = start;
                data -> counter = segment_counter;
                list_add_sorted(temp, data, segment_cmp);
            }
            segment_counter = 0;
            limit = get_segment_limit(collection, i);
            if (limit < 0) return -1;
            printf("Final de segmento: %d\n", limit);
            i = limit - 1;
            start = limit;
        } else {
            printf("Direccion vacia: %d\n", i);
            segment_counter ++;
        }
    }

    if (segment_counter >= total_size) {
        printf("Encontre un segmento: %d\n", start);
        best_fit_data *data = malloc(sizeof(best_fit_data));
        data -> addr = start;
        data -> counter = segment_counter;
        list_add_sorted(temp, data, segment_cmp);
    }

    if (list_size(temp) > 0) {
        best_fit_data *aux = list_get(temp, 0);
        result = aux -> addr;
        list_destroy_and_destroy_elements(temp, best_fit_destroyer);
        printf("Direccion a devolver: %d\n", result);
        return result;
    }

    return -1;
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

int remove_segment_from_memory(void *memory, int mem_size, segment *segmento ,t_dictionary * table_collection, int id_pcb) {
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
=======
char *get_fs_dir(char *endpoint) {
    char *route = string_new();
>>>>>>> 83af44fadd17d60d7cf9e3f55af2e465ff067ca6

    string_append(&route, _FSDIR_);

    if (endpoint) {
        string_append(&route, endpoint);
    }

    return route;
}


// void validarBlocks(t_log* log){
//     log_info(log, "Validando existencia de Blocks.ims....");

//     if(access("../Filesystem/Blocks.ims",F_OK) < 0){
//         log_error(log, "No se encontró archivo Blocks.ims. Se crea archivo");
        
//         FILE* arch_bloques = fopen("../Filesystem/Blocks.ims","w");
//         void* punteroTamanioBlocks = calloc(tamanioBloque*cantidadBloques, 1);
//         fwrite(punteroTamanioBlocks,tamanioBloque*cantidadBloques ,1 , arch_bloques);
//         fseek(arch_bloques,tamanioBloque*cantidadBloques,SEEK_SET);
        
//         char finArchivo = 'f';
//         fwrite(&finArchivo,sizeof(char),1, arch_bloques);
//         fclose(arch_bloques);
//         free(punteroTamanioBlocks);
//         log_info(log, "Se creó archivo Blocks.ims");

//         //TO DO: mapear a memoria el archivo entero.
//     }else{
//         log_info(log,"Existe archivo Blocks.ims.");
//     }


// }


// void guardarEspacioBitmap(t_log* log){
    
//     FILE* superBloque = fopen("../Filesystem/SuperBloque.ims","wb");
//     fseek(superBloque,sizeof(int)*2,SEEK_SET);
// 	void* punteroABitmap = calloc(cantidadBloques/8, 1);
//     fwrite(punteroABitmap, 1, cantidadBloques/8, superBloque);
//     free(punteroABitmap);
//     fclose(superBloque);

//     log_info(log, "Guarde espacio para el bitmap en superbloque");
// }

// void generarBitmap(t_log* log){

//     log_info(log, "Generando bitmap...");
//     guardarEspacioBitmap(log);
    
//     int archBitmap = open("../Filesystem/SuperBloque.ims", O_CREAT | O_RDWR, 0664);
//     lseek(archBitmap, sizeof(int)*2, SEEK_SET);
    
//     p_bitmap = mmap(NULL, cantidadBloques/8, PROT_READ | PROT_WRITE, MAP_SHARED, archBitmap, 0);
//     t_bitarray* bitmap = bitarray_create_with_mode((char*)p_bitmap, cantidadBloques/8, MSB_FIRST);  


//     for(int i=0; i<cantidadBloques; i++){
//         bitarray_clean_bit(bitmap,i);
//         msync(bitmap->bitarray,cantidadBloques/8 ,0);
//     }

//     log_info(log, "Muestro mis valores del bitmap..");
//     for(int i=0; i<cantidadBloques; i++){
//         log_info(log,"%d",bitarray_test_bit(bitmap,i));
//     }
    
//     bitarray_destroy(bitmap);
//     close(archBitmap);

//     printf("\n[TEST bitmap] Tamaño bloque: %d",tamanioBloque);
//     printf("\n[TEST bitmap] Cantidad de bloques:%d", cantidadBloques);
// }

// void mapearBlocks(t_log* log){

//     printf("\n[TEST Map] Tamaño bloque: %d",tamanioBloque);
//     printf("\n[TEST Map] Cantidad de bloques:%d", cantidadBloques);
    

//     int bloques = open("../Filesystem/Blocks.ims", O_CREAT | O_RDWR, 0664);    
//     mapArchivo = mmap(NULL,tamanioBloque*cantidadBloques, PROT_READ | PROT_WRITE, MAP_SHARED, bloques, 0);

//     char* test = "OOO";
//     memcpy(mapArchivo,test,strlen(test)+1);
//     printf(".......Escribo OOO.........");
// } 

void validarDirectorioFS(t_log* log){
    DIR* dir = opendir(get_fs_dir(NULL));
    if(ENOENT == errno){
        log_info(log, "No existe directorio: Filesystem. Se crea.");
        mkdir(get_fs_dir(NULL), 0664);
        mkdir(get_fs_dir("/bitacoras"), 0664);
        mkdir(get_fs_dir("/files"), 0664);
    }
    else{
        log_info(log, "Ya existe directorio");
    }
}

void validarSuperBloque(t_log* log){
    
    log_info(log, "Validando existencia de superbloque.ims....");
    if(access(get_fs_dir("/superBloque.ims"),F_OK) < 0){
        log_error(log, "No se encontró archivo superBloque.ims. Se crea archivo");
        
        log_info(log, "Ingresar tamaño de cada bloque");
        scanf("%d", &tamanioBloque);
        log_info(log, "Ingresar cantidad de bloques");
        scanf("%d", &cantidadBloques);

        FILE* superBloque = fopen(get_fs_dir("/SuperBloque.ims"),"wb");

        if(superBloque != NULL){    
            fwrite(&tamanioBloque, sizeof(int), 1, superBloque);
            fseek(superBloque,sizeof(int),SEEK_SET);
            fwrite(&cantidadBloques, sizeof(int), 1, superBloque);
            fclose(superBloque);
        }else
            log_error(log,"Error al abrir Superbloque.ims");
        log_info(log, "Se creó archivo superBloque.ims");
    }else{
        log_info(log, "Se encontró el archivo superBloque.ims");
        
        FILE* superBloque = fopen(get_fs_dir("/SuperBloque.ims"),"rb");
        if(superBloque != NULL){
            fread(&tamanioBloque, sizeof(int), 1, superBloque);
            fseek(superBloque,sizeof(int),SEEK_SET);
            fread(&cantidadBloques, sizeof(int), 1, superBloque);
            fclose(superBloque);
        }else
            log_error(log,"Error al abrir Superbloque.ims");

        log_info(log, "\nSe muestra los datos del superBloque.\nTamaño de bloque: %d\nCantidad de bloques: %d\n",tamanioBloque, cantidadBloques);
    }
}

int main() {

    t_log *log = log_create("../logs/test.log", "TEST", 1, LOG_LEVEL_TRACE);

    // -------------- TEST FILE SYSTEM -------------- //

    validarDirectorioFS(log);
    // validarSuperBloque(log);
    // validarBlocks(log);
    // generarBitmap(log);
    // mapearBlocks(log);

    // --------------- TEST DIR ROUTE -------------- //

    // log_info(log, "Testing..");
    // log_info(log, "FS Root: %s", get_fs_dir(NULL));
    // log_info(log, "FS /Blocks: %s", get_fs_dir("/blocks"));

    log_destroy(log);

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