#include <stdio.h>
#include <stdlib.h>
#include <unnamed/socket.h>
#include <unnamed/serialization.h>
#include <commons/log.h>
#include <commons/collections/list.h>
#include <commons/collections/queue.h>

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
    uint32_t nroSegmento;
    uint32_t baseAddr;
    uint32_t limit;
} segment;

uint32_t global_index;
uint32_t global_segment;
uint32_t global_type;

void mostrarInfo (void *element) {
    p_info *info = element;
    printf("ID: %d\n", info -> id);
    printf("Type: %d\n", info -> type);
    printf("Segment: %d\n", info -> nroSegmento);
}

void mostrarSegemento (void *element) {
    segment *segmento = element;
    printf("Start: %d\n", segmento -> baseAddr);
    printf("End: %d\n", segmento -> limit);
    printf("Segment: %d\n", segmento -> nroSegmento);
}

void *find_info_by_id(t_list* self, int id) {
    p_info *temp;
	if ((self->elements_count > 0) && (id >= 0)) {

		t_link_element *element = self->head;
		temp = element -> data;

        while (temp -> id != id) {
			element = element->next;
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
            temp = element -> data;
		}

		return element -> data;
	}
	return NULL;
}

int get_segment_limit(t_list* self, int start) {
    segment *temp;

	if ((self->elements_count > 0) && (start >= 0)) {

		t_link_element *element = self->head;
		temp = element -> data;

        while (temp -> baseAddr != start) {
			element = element->next;
            temp = element -> data;
		}


		return temp -> limit;
	}
	return -1;
}

void destroyer(void *item) {
    free(item);
}

void memory_compaction(void *memory, int mem_size, t_list* segmentTable) {
    void *aux_memory;

    segment *temp;

    t_queue * aux_table;

    int data_size;

    int offset = 0;

    int segment_id = 0;

	if ((segmentTable -> elements_count > 0) && (mem_size >= 0)) {

		t_link_element *element = segmentTable -> head;
		temp = element -> data;

        aux_memory = malloc(mem_size);
        aux_table = queue_create();

        while (element != NULL) {

            data_size = temp -> limit - temp ->baseAddr;

            printf("Copiando elementos.. %d - %d - %d\n", temp -> nroSegmento, temp -> baseAddr, temp -> limit);
            
            // Copio los datos del segmento en la memoria auxiliar
            memcpy(aux_memory + offset, memory + temp -> baseAddr, data_size);

            segment *new_segment = malloc(sizeof(segment));
            new_segment -> nroSegmento = segment_id++;
            new_segment -> baseAddr = offset;
            new_segment -> limit = offset + data_size - 1;

            printf("Creando nuevo Segmento.. %d - %d - %d\n", new_segment -> nroSegmento, new_segment -> baseAddr, new_segment -> limit);

            queue_push(aux_table, new_segment);

            offset += data_size;

            element = element -> next;
            temp = element -> data;
        }
	}

    printf("HERE!");
	// Asigno los punteros respectivos a memoria y segmentTable

    memory = aux_memory;

    segmentTable = aux_table -> elements;

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

    int m_size = 30;
    void *memory = malloc(m_size);

    t_queue *segmentTable = queue_create();

    int temp = 8;

    segment *uno = malloc(sizeof(segment));
    segment *dos = malloc(sizeof(segment));
    segment *tres = malloc(sizeof(segment));
    segment *cuatro = malloc(sizeof(segment));

    uno -> nroSegmento = 2;
    uno -> baseAddr = 1;
    uno -> limit = 4;

    dos -> nroSegmento = 3;
    dos -> baseAddr = 10;
    dos -> limit = 13;

    tres -> nroSegmento = 5;
    tres -> baseAddr = 15;
    tres -> limit = 18;

    cuatro -> nroSegmento = 7;
    cuatro -> baseAddr = 22;
    cuatro -> limit = 25;

    memcpy(memory + uno -> baseAddr, &temp, uno -> limit - uno -> baseAddr);
    temp = 10;
    memcpy(memory + dos -> baseAddr, &temp, dos -> limit - dos -> baseAddr);
    temp = 12;
    memcpy(memory + tres -> baseAddr, &temp, tres -> limit - tres -> baseAddr);
    temp = 14;
    memcpy(memory + cuatro -> baseAddr, &temp, cuatro -> limit - cuatro -> baseAddr);

    queue_push(segmentTable, uno);
    queue_push(segmentTable, dos);
    queue_push(segmentTable, tres);
    queue_push(segmentTable, cuatro);

    // Auxiliar para guardar el limite de cada segmento ocupado
    int limit;

    // Contador de bytes libres en la memoria
    int segment_counter = 0;

    // Tamanio del segmento que quiero guardar
    int total_size = 6;
    
    // Valida si encontre un segmento libre o no
    int found_segment = 0;

    // Busco espacio libre en la memoria
    for(int i = 0; i < m_size; i ++) {
        if (memcmp(memory + i, "\0", 1)) {
            printf("Direccion ocupada: %d\n", i);
            limit = get_segment_limit(segmentTable -> elements, i);
            printf("Final de segmento: %d\n", limit);
            i = limit;
            segment_counter = 0;
        } else {
            printf("Direccion vacia: %d\n", i);
            segment_counter ++;
            if (segment_counter == total_size) {
                found_segment = 1;
                printf("Encontre un segmento disponible\n");
            }
        }
    }

    if(!found_segment) {
        printf("No encontre ningun segmento libre.. Iniciando compactacion.\n");
        memory_compaction(memory, m_size, segmentTable -> elements);

        // Busco espacio libre en la memoria
        for(int i = 0; i < m_size; i ++) {
            if (memcmp(memory + i, "\0", 1)) {
                printf("Direccion ocupada: %d\n", i);
                limit = get_segment_limit(segmentTable -> elements, i);
                printf("Final de segmento: %d\n", limit);
                i = limit;
                segment_counter = 0;
            } else {
                printf("Direccion vacia: %d\n", i);
                segment_counter ++;
                if (segment_counter == total_size) {
                    found_segment = 1;
                    printf("Encontre un segmento disponible\n");
                }
            }
        }
    }

    queue_destroy_and_destroy_elements(segmentTable, destroyer);

    free(memory);

    // ---------------- TEST QUEUES ----------------- //

    // t_queue *listaInfo = queue_create();
    // t_queue *segmentTable = queue_create();

    // segment *segmento = malloc(sizeof(segment));
    // segmento -> nroSegmento = 1;
    // segmento -> baseAddr = 0;
    // segmento -> limit = 14;

    // p_info *info = malloc(sizeof(p_info));
    // info -> nroSegmento = 1;
    // info -> id = 0;
    // info -> type = PCB;

    // queue_push(listaInfo, info);
    // queue_push(segmentTable, segmento);

    // list_iterate(listaInfo -> elements, mostrarInfo);
    // list_iterate(segmentTable -> elements, mostrarSegemento);

    // p_info *infoTest = find_info_by_id(listaInfo -> elements, 0);

    // segment *segmentTest = find_segment_by_number(segmentTable -> elements, infoTest -> nroSegmento);

    // printf("Segmento test: %d\n", segmentTest -> limit);

    // free(info);
    // free(segmento);

    // queue_destroy(listaInfo);
    // queue_destroy(segmentTable);


    // ---------------- TEST ARCHIVOS ----------------- //
/*

    FILE * fp;
    char * line = NULL;
    size_t len = 0;
    ssize_t read;

    void *buffer;
    int b_size = 0;
    int offset = 0;
    int new_size;
    void *temp;

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

        if (!b_size) {

            b_size = new_size;
            buffer = malloc(b_size);
        } else {

            b_size += new_size;
            buffer = realloc(buffer, b_size);
        }
        
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

    while (off < offset) {

        memcpy(&size_tarea, buffer + off, sizeof(int));
        off += sizeof(int);

        tarea = malloc(size_tarea + 1);
        memcpy(tarea, buffer + off, size_tarea);
        off += size_tarea;
        tarea[size_tarea] = '\0';

        log_info(logger, "Tarea %d - len %d: %s", i++, size_tarea, tarea);
        free(tarea);

    }

    free(buffer);
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