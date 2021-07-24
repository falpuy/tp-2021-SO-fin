#include "memory.h"

void *memory_init(int size) {
    void *temp = malloc(size);
    return temp;
}


// // ----------------------- PAGINATION ------------------------- //

bool lru_sorter(void *uno, void *dos) {
    page_t *page1 = uno;
    page_t *page2 = dos;

    return page1 -> frame -> time < page2 -> frame -> time;
}

void lru_replacer(void *item) {
    page_t *page = item;

    if (global_lru_page == NULL && page -> frame -> presence) {
        global_lru_page = page;
    } else {
        if (page -> frame -> presence && global_lru_page -> frame -> time > page -> frame -> time) {
            global_lru_page = page;
        }
    }
    
}

void lru_iterator(char *key, void *item) {
    t_queue *pages = item;

    // list_sort(pages -> elements, lru_sorter);
    list_iterate(pages -> elements, lru_replacer);
}

frame_t *get_next_lru_frame() {
    // // printf("Checkeando LRU %d\n", dictionary_is_empty(table_collection));
    global_lru_page = NULL;

    dictionary_iterator(table_collection, lru_iterator);

    return global_lru_page -> frame;

}

void clock_replacer(void *item, int key_index, int frame_index) {
    page_t *page = item;

    if (page -> frame -> presence && page -> frame -> modified == 0) {
        page -> frame -> modified = 1;
        global_clock_page = page;
        clock_flag = 1;
        global_clock_key = key_index;
        global_clock_index = frame_index;
    } else {
        if (page -> frame -> presence && page -> frame -> modified) {
            page -> frame -> modified = 0;
        }
    }
}

void clock_iterator(char *key, void *item, int key_index, int frame_index) {
    t_queue *pages = item;
    t_link_element *element = pages -> elements -> head;
	t_link_element *aux = NULL;
	while (element != NULL) {
		aux = element->next;
        if (frame_index >= global_clock_index) {
		    clock_replacer(element->data, key_index, frame_index);
        }
        if(clock_flag) {
            break;
        }
		element = aux;
	}
    if (frame_index >= global_clock_index) {
        global_clock_index = 0;
    }
    // list_iterate(pages -> elements, clock_replacer);
}

frame_t *get_next_clock_frame() {
    // // printf("Checkeando CLOCK %d\n", dictionary_is_empty(table_collection));
    global_clock_page = NULL;

    clock_flag = false;

    while(!clock_flag) {
        int table_index;
        int frame_index = 0;
        for (table_index = global_clock_key; table_index < table_collection->table_max_size; table_index++) {
            t_hash_element *element = table_collection->elements[table_index];

            while (element != NULL) {
                clock_iterator(element->key, element->data, table_index, frame_index);
                element = element->next;
                if(clock_flag) {
                    break;
                }
            }
            if(clock_flag) {
                break;
            }
        }
        if (!clock_flag) {
            global_clock_key = 0;
            global_clock_index = 0;
        }
        // dictionary_iterator(table_collection, clock_iterator);
    }

    return global_clock_page -> frame;

}

void set_bitmap(uint8_t *bitmap, int position) {
    bitmap[position] = 1;
}

void unset_bitmap(uint8_t *bitmap, int position) {
    bitmap[position] = 0;
}

uint32_t get_frame() {
    int value;
    int err;

    for (int i = 0; i < frames_memory; i++) {
        // // printf("GET FRAME Bit %d: %d\n", i, bitarray_test_bit(bitmap, i));
        if (!bitmap[i]) {
            return i;
        }
    }
    // // printf("Algoritmo %s\n", hasLRU ? "LRU" : "CLOCK");
    // Checkeo timestamp
    // frame_t *replacing_frame = get_next_lru_frame();
    frame_t *replacing_frame = hasLRU ? get_next_lru_frame() : get_next_clock_frame();

    // // printf("Reemplazo este frame: %d\n", replacing_frame -> number);

    if (replacing_frame != NULL) {
        // busco lugar en virtual
        for (int i = 0; i < frames_virtual; i++) {
            // // printf("GET FRAME Bit %d: %d\n", i, bitarray_test_bit(bitmap, i));
            if (!bitarray_test_bit(virtual_bitmap, i)) {
                // // printf("Guardo en virtual %d\n", i);
                // pego la data desde memoria
                memcpy(virtual_memory + i * page_size, memory + replacing_frame -> number * page_size, page_size);
                // err = msync(virtual_memory, virtual_size + frames_virtual / 8, MS_ASYNC);
                // if (err == -1){
                //     perror("Error de sincronizar a disco memoria");
                //     return -1;
                // }
                // seteo el bit de virtual
                bitarray_set_bit(virtual_bitmap, i);
                // unseteo el bit
                bitmap[replacing_frame -> number] = 0;
                // Actualizo el valor a devolver
                value = replacing_frame -> number;

                //updateo el frame que muevo
                replacing_frame -> number = i;
                replacing_frame -> start = i * page_size;
                replacing_frame -> presence = 0;
                replacing_frame -> time = timer++;
                // devuelvo el bit qe unsetie
                // // printf("Devuelvo: %d\n", value);
                return value;
            }
        }
    }

    return -1;
}

int check_free_frames(int frames_count) {
    int counter = 0;

    // Busco frames libres en el bitmap y agrego a un counter
    for (int i = 0; i < frames_memory; i++) {
        if (!bitmap[i]) {
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

int save_data_in_memory(void *memory, t_dictionary *table_collection, t_dictionary* admin_collection, void *buffer) {

    double val;

    void *temp;
    pcb_t *p_aux = malloc(sizeof(pcb_t));
    int task_size;
    int tcb_count;
    int offset = 0;
    int tid, xpos, ypos;

    int task_start = 8;

    p_aux -> tasks = 8; // Al leer un frame, las tareas siempre comienzan en la posicion 8 del frame leido (paginacion)
    memcpy(&p_aux -> pid, buffer + offset, sizeof(uint32_t));
    offset += sizeof(int);
    
    memcpy(&task_size, buffer + offset, sizeof(int));
    offset += sizeof(int);

    void *tasks = malloc(task_size + 1);
    memcpy(tasks, buffer + offset, task_size);
    memset(tasks + task_size, '\0', 1);
    offset += task_size;

    // // printf("ME LLEGO LA TAREA: %s\n", tasks);

    memcpy(&tcb_count, buffer + offset, sizeof(int));
    offset += sizeof(int);

    int memory_size = sizeof(pcb_t) + task_size + tcb_count * sizeof(tcb_t);

    admin_data *tcb_data = malloc(sizeof(admin_data));
    tcb_data -> cantidad = tcb_count;
    tcb_data -> start = sizeof(pcb_t) + task_size;
    tcb_data -> tcb = malloc(sizeof(uint8_t) * tcb_count);

    // Inicializo el bitmap de tcbs en 1 ya que son los que se crean al principio
    for(int i = 0; i < tcb_count; i++) {
        tcb_data -> tcb[i] = 1;
    }

    val = (double) memory_size / page_size;
    int frames_count = ceil(val);

    if (!check_free_frames(frames_count)) {
        return -1;
    }

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

    dictionary_put(admin_collection, pid, tcb_data);

    memcpy(temp + temp_off, tasks, task_size);
    temp_off += task_size;

    free(tasks);

    // // printf("Size until tcbs.. %d - %d\n", temp_off, offset);

    for( int i = 0; i < tcb_count; i++) {
        memcpy(temp + temp_off, buffer + offset, sizeof(uint32_t));
        memcpy(&tid, buffer + offset, sizeof(uint32_t));
        temp_off += sizeof(uint32_t);
        offset += sizeof(uint32_t);
        
        memcpy(temp + temp_off, buffer + offset, sizeof(uint32_t));
        temp_off += sizeof(uint32_t);
        offset += sizeof(uint32_t);
        
        memcpy(temp + temp_off, buffer + offset, sizeof(char));
        temp_off += sizeof(char);
        offset += sizeof(char);
        
        memcpy(temp + temp_off, buffer + offset, sizeof(uint32_t));
        memcpy(&xpos, buffer + offset, sizeof(uint32_t));
        temp_off += sizeof(uint32_t);
        offset += sizeof(uint32_t);
        
        memcpy(temp + temp_off, buffer + offset, sizeof(uint32_t));
        memcpy(&ypos, buffer + offset, sizeof(uint32_t));
        temp_off += sizeof(uint32_t);
        offset += sizeof(uint32_t);
        
        // Compio inicio de tareas en temp buffer
        memcpy(temp + temp_off, &task_start, sizeof(uint32_t));
        temp_off += sizeof(uint32_t);
        offset += sizeof(uint32_t);

        personaje_crear(nivel, tid, xpos, ypos);
        nivel_gui_dibujar(nivel);
    }

    // ---------------- GUARDO FRAMES ---------------- //

    // Busco si hay frames libres en memoria/disco
    if (check_free_frames(frames_count)) {
        // Creo tabla de paginas
        t_queue *tabla = queue_create();

        // Si hay, guardo la data
        int bytes_left = memory_size;

        for (int j = 0; j < frames_count; j++) {
            uint32_t n_frame = get_frame();

            set_bitmap(bitmap, n_frame);
            
            // Creo frame
            frame_t *frame = malloc(sizeof(frame_t));
            frame -> time = timer++;
            frame -> number = n_frame;
            frame -> start = n_frame * page_size;
            frame -> modified = 1;
            frame -> presence = 1;

            // // printf("%d TIME: %d\n", frame -> number, frame -> time);

            // Creo pagina
            page_t *page = malloc(sizeof(page_t));
            page -> frame = frame;

            if (bytes_left < page_size) {
                // copio bytes_left
                memcpy(memory + (n_frame * page_size), temp + (j * page_size), bytes_left);
            } else {
                // copio page_size
                memcpy(memory + (n_frame * page_size), temp + (j * page_size), page_size);
                bytes_left -= page_size;
            }

            queue_push(tabla, page);
            // Agrego tabla al diccionario
            dictionary_put(table_collection, pid, tabla);
        }
        free(temp);
        free(pid);
        return 1;

    } else {
        free(temp);
        free(pid);
        return -1;
    }

}


void *get_task_from_page(void *memory, t_dictionary *admin_collection, t_dictionary *table_collection, char *key, int id_tcb) {
    // // printf("Obtengo tablas del proceso..\n");
    t_queue *self = dictionary_get(table_collection, key);
    admin_data *data_tcb = dictionary_get(admin_collection, key);

    void *temp = malloc(queue_size(self) * page_size);

    page_t *page_aux;

    int original_size = queue_size(self);

    char *nextTask;

    void *recv_task;

    // // printf("Obtengo las paginas en memoria..: %d\n", original_size);

    int off = 0;
    while(queue_size(self) > 0) {
        page_aux = queue_pop(self);

        // // printf("STATUS: %d\n", page_aux -> frame -> presence);
        if (page_aux -> frame -> presence) {
            // // printf("PAGE IN MEMORY\n");
            memcpy(temp + (off * page_size), memory + (page_aux -> frame) -> start, page_size);
            unset_bitmap(bitmap, (page_aux -> frame) -> number);
        } else {
            // // printf("SWAP PAGE\n");
            // TODO: SWAPEAR
            memcpy(temp + (off * page_size), virtual_memory + (page_aux -> frame) -> start, page_size);

            bitarray_clean_bit(virtual_bitmap, (page_aux -> frame) -> number);
            
        }
        
        off++;

        // free((page_aux -> frame) -> time);
        free(page_aux -> frame);
        free(page_aux);
    }

    // // printf("Obtengo tcb a eliminar del buffer: %d\n", data_tcb -> start);

    // --- Getting tcb list from temp
    uint32_t temp_id;
    int tcb_left;
    int auxoff = 0;
    for (int i = 0; i < data_tcb -> cantidad; i++) {
        // Leo solo el primer int, que representa el tid
        memcpy(&temp_id, (temp + data_tcb -> start) + (i * sizeof(tcb_t)), sizeof(uint32_t));

        if (temp_id == id_tcb) {

            // // printf("\n\n ------ busco la tarea ------- \n\n");

            int task_counter = 0;

            int prevTask;

            memcpy(&prevTask, (temp + data_tcb -> start) + (i * sizeof(tcb_t)) + sizeof(uint32_t) * 4 + sizeof(char), sizeof(uint32_t));

            while (memcmp(temp + prevTask + task_counter, ";", 1) && temp + prevTask + task_counter != NULL && prevTask + task_counter < data_tcb -> start) {
                // // printf("CHAR: %c\n", get_char_value(temp + prevTask, task_counter));
                task_counter++;
            }
            while (!isalpha(get_char_value(temp + prevTask, task_counter)) && temp + prevTask + task_counter != NULL && prevTask + task_counter < data_tcb -> start){
                // // printf("CHAR: %c\n", get_char_value(temp + prevTask, task_counter));
                task_counter++;
            }

            // printf("Tarea previa: %d - %d\n", prevTask, prevTask + task_counter);

            if (prevTask < data_tcb -> start && prevTask + task_counter <= data_tcb -> start) {
                recv_task = malloc(task_counter + 1);
                memcpy(recv_task, temp + prevTask, task_counter);
                memset(recv_task + task_counter, '\0', 1);
            } else {
                recv_task = NULL;
            }

            int next_addr = prevTask + task_counter;

            memcpy((temp + data_tcb -> start) + (i * sizeof(tcb_t)) + sizeof(uint32_t) * 4 + sizeof(char), &next_addr, sizeof(uint32_t));

            break;
        }
    }


    int size_a_copiar = data_tcb -> start + data_tcb -> cantidad * sizeof(tcb_t);

    // // printf("Actualizo frames en memoria: %d - %d\n", data_tcb -> cantidad, size_a_copiar);
    // int size_until_task = data_tcb -> start;
    int posicion_temp = 0;
    uint32_t n_frame;

    while(size_a_copiar) {
        if (size_a_copiar >= page_size) {
            n_frame = get_frame();
            set_bitmap(bitmap, n_frame);

            // Creo frame
            frame_t *frame = malloc(sizeof(frame_t));
            frame -> time = timer++;
            frame -> number = n_frame;
            frame -> start = n_frame * page_size;
            frame -> modified = 1;
            frame -> presence = 1;

            // Creo pagina
            page_t *page = malloc(sizeof(page_t));
            page -> frame = frame;

            memcpy(memory + (n_frame * page_size), temp + posicion_temp * page_size, page_size);

            queue_push(self, page);

            size_a_copiar -= page_size;
        } else {
            n_frame = get_frame();
            set_bitmap(bitmap, n_frame);

            // Creo frame
            frame_t *frame = malloc(sizeof(frame_t));
            frame -> time = timer++;
            frame -> number = n_frame;
            frame -> start = n_frame * page_size;
            frame -> modified = 1;
            frame -> presence = 1;

            // Creo pagina
            page_t *page = malloc(sizeof(page_t));
            page -> frame = frame;

            memcpy(memory + (n_frame * page_size), temp + posicion_temp * page_size, size_a_copiar);

            queue_push(self, page);

            size_a_copiar -= size_a_copiar;
        }
        posicion_temp++;
    }

    free(temp);

    dictionary_put(table_collection, key, self);

    return recv_task;
}

void remove_tcb_from_page(void *memory, t_dictionary *admin_collection, t_dictionary *table_collection, char *key, int id_tcb) {
    // // printf("Obtengo tablas del proceso..\n");
    t_queue *self = dictionary_get(table_collection, key);
    admin_data *data_tcb = dictionary_get(admin_collection, key);

    void *temp = malloc(queue_size(self) * page_size);

    page_t *page_aux;

    int original_size = queue_size(self);

    // // printf("Obtengo las paginas en memoria..: %d\n", original_size);

    int off = 0;
    while(queue_size(self) > 0) {
        
        page_aux = queue_pop(self);

        // printf("STATUS: %d\n", page_aux -> frame -> presence);
        if (page_aux -> frame -> presence) {
            // printf("PAGE IN MEMORY\n");
            memcpy(temp + (off * page_size), memory + (page_aux -> frame) -> start, page_size);
            unset_bitmap(bitmap, (page_aux -> frame) -> number);
        } else {
            // printf("SWAP PAGE\n");
            // TODO: SWAPEAR
            memcpy(temp + (off * page_size), virtual_memory + (page_aux -> frame) -> start, page_size);

            bitarray_clean_bit(virtual_bitmap, (page_aux -> frame) -> number);
            
        }
        
        off++;

        // free((page_aux -> frame) -> time);
        free(page_aux -> frame);
        free(page_aux);
    }

    // // printf("Obtengo tcb a eliminar del buffer: %d\n", data_tcb -> start);

    // --- Getting tcb list from temp
    uint32_t temp_id;
    int tcb_left;
    for (int i = 0; i < data_tcb -> cantidad; i++) {
        // Leo solo el primer int, que representa el tid
        memcpy(&temp_id, (temp + data_tcb -> start) + (i * sizeof(tcb_t)), sizeof(uint32_t));
        
        if (temp_id == id_tcb) {

            // // printf("Entre al if...\n");

            data_tcb -> tcb[i] = 0;
            tcb_left = data_tcb -> cantidad - i - 1;

            // Hago la compactacion de los tcb restantes
            memcpy((temp + data_tcb -> start) + (i * sizeof(tcb_t)), (temp + data_tcb -> start) + ((i + 1) * sizeof(tcb_t)), tcb_left * sizeof(tcb_t));
            data_tcb -> cantidad -= 1;

            break;
        }
    }


    int size_a_copiar = data_tcb -> start + data_tcb -> cantidad * sizeof(tcb_t);

    // // printf("Actualizo frames en memoria: %d - %d\n", data_tcb -> cantidad, size_a_copiar);
    // int size_until_task = data_tcb -> start;
    int posicion_temp = 0;
    uint32_t n_frame;

    while(size_a_copiar) {
        if (size_a_copiar >= page_size) {
            n_frame = get_frame();
            set_bitmap(bitmap, n_frame);

            // Creo frame
            frame_t *frame = malloc(sizeof(frame_t));
            frame -> time = timer++;
            frame -> number = n_frame;
            frame -> start = n_frame * page_size;
            frame -> modified = 1;
            frame -> presence = 1;

            // Creo pagina
            page_t *page = malloc(sizeof(page_t));
            page -> frame = frame;

            memcpy(memory + (n_frame * page_size), temp + posicion_temp * page_size, page_size);

            queue_push(self, page);

            size_a_copiar -= page_size;
        } else {
            n_frame = get_frame();
            set_bitmap(bitmap, n_frame);

            // Creo frame
            frame_t *frame = malloc(sizeof(frame_t));
            frame -> time = timer++;
            frame -> number = n_frame;
            frame -> start = n_frame * page_size;
            frame -> modified = 1;
            frame -> presence = 1;

            // Creo pagina
            page_t *page = malloc(sizeof(page_t));
            page -> frame = frame;

            memcpy(memory + (n_frame * page_size), temp + posicion_temp * page_size, size_a_copiar);

            queue_push(self, page);

            size_a_copiar -= size_a_copiar;
        }
        posicion_temp++;
    }

    free(temp);

    dictionary_put(table_collection, key, self);
}

void remove_pcb_from_page(void *memory, t_dictionary *admin_collection, t_dictionary *table_collection, char *key) {

    t_queue *self = dictionary_get(table_collection, key);
    admin_data *data_tcb = dictionary_get(admin_collection, key);

    dictionary_remove_and_destroy(admin_collection, key, admin_destroyer);

    page_t *page_aux;

    while(queue_size(self) > 0) {
        page_aux = queue_pop(self);

        unset_bitmap(bitmap, (page_aux -> frame) -> number);

        free(page_aux -> frame);
        free(page_aux);
    }

    dictionary_put(table_collection, key, self);
}

void update_position_from_page(void *memory, t_dictionary *admin_collection, t_dictionary *table_collection, char *key, int id_tcb, int posx, int posy) {
    // // printf("Obtengo tablas del proceso..\n");
    t_queue *self = dictionary_get(table_collection, key);
    admin_data *data_tcb = dictionary_get(admin_collection, key);

    void *temp = malloc(queue_size(self) * page_size);

    page_t *page_aux;

    int original_size = queue_size(self);

    // // printf("Obtengo las paginas en memoria..: %d\n", original_size);

    int off = 0;
    while(queue_size(self) > 0) {
        // page_aux = queue_pop(self);

        // // if (frame_in_memory(page)) {
        // //     memcpy(temp, memory + (page -> frame) -> start, page_size);
        // // } else {
        // //     swap_page(memory, page);
        // //     memcpy(temp, memory + (page -> frame) -> start, page_size);
        // // }
        // // // printf("Frame.. %d\n", page_aux -> frame -> start);
        // memcpy(temp + (off * page_size), memory + (page_aux -> frame) -> start, page_size);
        // unset_bitmap(bitmap, (page_aux -> frame) -> number);
        // off++;
        page_aux = queue_pop(self);

        // printf("STATUS: %d\n", page_aux -> frame -> presence);
        if (page_aux -> frame -> presence) {
            // printf("PAGE IN MEMORY\n");
            memcpy(temp + (off * page_size), memory + (page_aux -> frame) -> start, page_size);
            unset_bitmap(bitmap, (page_aux -> frame) -> number);
        } else {
            // printf("SWAP PAGE\n");
            // TODO: SWAPEAR
            memcpy(temp + (off * page_size), virtual_memory + (page_aux -> frame) -> start, page_size);

            bitarray_clean_bit(virtual_bitmap, (page_aux -> frame) -> number);
            
        }
        
        off++;

        // free((page_aux -> frame) -> time);
        free(page_aux -> frame);
        free(page_aux);
    }

    // // printf("Obtengo tcb a eliminar del buffer: %d\n", data_tcb -> start);

    // --- Getting tcb list from temp
    uint32_t temp_id;
    int tcb_left;
    for (int i = 0; i < data_tcb -> cantidad; i++) {
        // Leo solo el primer int, que representa el tid
        memcpy(&temp_id, (temp + data_tcb -> start) + (i * sizeof(tcb_t)), sizeof(uint32_t));
        
        if (temp_id == id_tcb) {

            memcpy((temp + data_tcb -> start) + (i * sizeof(tcb_t)) + sizeof(uint32_t) * 2 + sizeof(char), &posx, sizeof(uint32_t));
            memcpy((temp + data_tcb -> start) + (i * sizeof(tcb_t)) + sizeof(uint32_t) * 3 + sizeof(char), &posy, sizeof(uint32_t));

        }
    }


    int size_a_copiar = data_tcb -> start + data_tcb -> cantidad * sizeof(tcb_t);

    // // printf("Actualizo frames en memoria: %d - %d\n", data_tcb -> cantidad, size_a_copiar);
    // int size_until_task = data_tcb -> start;
    int posicion_temp = 0;
    uint32_t n_frame;

    while(size_a_copiar) {
        if (size_a_copiar >= page_size) {
            n_frame = get_frame();
            set_bitmap(bitmap, n_frame);

            // Creo frame
            frame_t *frame = malloc(sizeof(frame_t));
            frame -> time = timer++;
            frame -> number = n_frame;
            frame -> start = n_frame * page_size;
            frame -> modified = 1;
            frame -> presence = 1;

            // Creo pagina
            page_t *page = malloc(sizeof(page_t));
            page -> frame = frame;

            memcpy(memory + (n_frame * page_size), temp + posicion_temp * page_size, page_size);

            queue_push(self, page);

            size_a_copiar -= page_size;
        } else {
            n_frame = get_frame();
            set_bitmap(bitmap, n_frame);

            // Creo frame
            frame_t *frame = malloc(sizeof(frame_t));
            frame -> time = timer++;
            frame -> number = n_frame;
            frame -> start = n_frame * page_size;
            frame -> modified = 1;
            frame -> presence = 1;

            // Creo pagina
            page_t *page = malloc(sizeof(page_t));
            page -> frame = frame;

            memcpy(memory + (n_frame * page_size), temp + posicion_temp * page_size, size_a_copiar);

            queue_push(self, page);

            size_a_copiar -= size_a_copiar;
        }
        posicion_temp++;
    }

    free(temp);

    dictionary_put(table_collection, key, self);
}


void update_status_from_page(void *memory, t_dictionary *admin_collection, t_dictionary *table_collection, char *key, int id_tcb, char status) {
    // // printf("Obtengo tablas del proceso..\n");
    t_queue *self = dictionary_get(table_collection, key);
    admin_data *data_tcb = dictionary_get(admin_collection, key);

    void *temp = malloc(queue_size(self) * page_size);

    page_t *page_aux;

    int original_size = queue_size(self);

    // // printf("Obtengo las paginas en memoria..: %d\n", original_size);

    int off = 0;
    while(queue_size(self) > 0) {
        // page_aux = queue_pop(self);

        // // if (frame_in_memory(page)) {
        // //     memcpy(temp, memory + (page -> frame) -> start, page_size);
        // // } else {
        // //     swap_page(memory, page);
        // //     memcpy(temp, memory + (page -> frame) -> start, page_size);
        // // }
        // // // printf("Frame.. %d\n", page_aux -> frame -> start);
        // memcpy(temp + (off * page_size), memory + (page_aux -> frame) -> start, page_size);
        // unset_bitmap(bitmap, (page_aux -> frame) -> number);
        // off++;
        page_aux = queue_pop(self);

        // printf("STATUS: %d\n", page_aux -> frame -> presence);
        if (page_aux -> frame -> presence) {
            // printf("PAGE IN MEMORY\n");
            memcpy(temp + (off * page_size), memory + (page_aux -> frame) -> start, page_size);
            unset_bitmap(bitmap, (page_aux -> frame) -> number);
        } else {
            // printf("SWAP PAGE\n");
            // TODO: SWAPEAR
            memcpy(temp + (off * page_size), virtual_memory + (page_aux -> frame) -> start, page_size);

            bitarray_clean_bit(virtual_bitmap, (page_aux -> frame) -> number);
            
        }
        
        off++;

        // free((page_aux -> frame) -> time);
        free(page_aux -> frame);
        free(page_aux);
    }

    // // printf("Obtengo tcb a eliminar del buffer: %d\n", data_tcb -> start);

    // --- Getting tcb list from temp
    uint32_t temp_id;
    int tcb_left;
    for (int i = 0; i < data_tcb -> cantidad; i++) {
        // Leo solo el primer int, que representa el tid
        memcpy(&temp_id, (temp + data_tcb -> start) + (i * sizeof(tcb_t)), sizeof(uint32_t));
        
        if (temp_id == id_tcb) {

            memcpy((temp + data_tcb -> start) + (i * sizeof(tcb_t)) + sizeof(uint32_t) * 2, &status, sizeof(char));

        }
    }


    int size_a_copiar = data_tcb -> start + data_tcb -> cantidad * sizeof(tcb_t);

    // // printf("Actualizo frames en memoria: %d - %d\n", data_tcb -> cantidad, size_a_copiar);
    // int size_until_task = data_tcb -> start;
    int posicion_temp = 0;
    uint32_t n_frame;

    while(size_a_copiar) {
        if (size_a_copiar >= page_size) {
            n_frame = get_frame();
            set_bitmap(bitmap, n_frame);

            // Creo frame
            frame_t *frame = malloc(sizeof(frame_t));
            frame -> time = timer++;
            frame -> number = n_frame;
            frame -> start = n_frame * page_size;
            frame -> modified = 1;
            frame -> presence = 1;

            // Creo pagina
            page_t *page = malloc(sizeof(page_t));
            page -> frame = frame;

            memcpy(memory + (n_frame * page_size), temp + posicion_temp * page_size, page_size);

            queue_push(self, page);

            size_a_copiar -= page_size;
        } else {
            n_frame = get_frame();
            set_bitmap(bitmap, n_frame);

            // Creo frame
            frame_t *frame = malloc(sizeof(frame_t));
            frame -> time = timer++;
            frame -> number = n_frame;
            frame -> start = n_frame * page_size;
            frame -> modified = 1;
            frame -> presence = 1;

            // Creo pagina
            page_t *page = malloc(sizeof(page_t));
            page -> frame = frame;

            memcpy(memory + (n_frame * page_size), temp + posicion_temp * page_size, size_a_copiar);

            queue_push(self, page);

            size_a_copiar -= size_a_copiar;
        }
        posicion_temp++;
    }

    free(temp);

    dictionary_put(table_collection, key, self);
}

void admin_destroyer(void *item) {

    admin_data * aux = (admin_data *) item;
    free(aux -> tcb);
    free(aux);

}

// // --------------------- END PAGINATION ----------------------- //

// --------------------- SEGMENTATION ----------------------- //


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
    // printf("ID: %d\n", segmento -> id);
    // printf("Type: %s\n", get_segment_type(segmento -> type));
    // printf("Start: %d\n", segmento -> baseAddr);
    // printf("End: %d\n", segmento -> limit);
    // printf("Segment: %d\n\n", segmento -> nroSegmento);
}

void show_dictionary(t_dictionary *self) {

    t_queue *aux;

    // Recorro el diccionario
    int table_index;

	for (table_index = 0; table_index < self->table_max_size; table_index++) {
		t_hash_element *element = self->elements[table_index];
		t_hash_element *next_element = NULL;

        if (element != NULL)
            // printf("\nKEY: %s\n\n", element -> key);

		while (element != NULL) {

			next_element = element->next;

            aux = element -> data;

            list_iterate(aux -> elements, mostrarSegemento);

			element = next_element;
		}
	}

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
    segment *temp = (segment *) item;
    free(temp);
}

void table_destroyer(void *item) {

    t_queue *table = (t_queue *) item;

    queue_destroy_and_destroy_elements(table, destroyer);

}

void page_destroyer(void *item) {

    page_t * aux = (page_t *) item;
    if (aux) {
        free(aux -> frame);
        free(aux);
    }
}

void page_table_destroyer(void *item) {

    t_queue *table = (t_queue *) item;
    queue_destroy_and_destroy_elements(table, page_destroyer);

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

                    // printf("Copiando elementos.. %d - %d - %d\n", temp -> nroSegmento, temp -> baseAddr, temp -> limit);
                    
                    // Copio los datos del segmento en la memoria auxiliar
                    memcpy(aux_memory + offset, memory + temp -> baseAddr, data_size);
                    memset(admin + offset, 1, data_size);

                    new_base = offset;
                    new_limit = offset + data_size;

                    // printf("Creando nuevo Segmento.. %d - %d - %d\n", temp -> nroSegmento, new_base, new_limit);

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

            // // printf("Direccion vacia: %d\n", i);
            segment_counter ++;
            if (segment_counter == total_size) {
                // // printf("Hay espacio disponible en memoria..\n");
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

    // printf("Comparando: %d - %d\n", f -> counter, s -> counter);

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
            // // printf("Segmento Libre en: %d\n", i);
            start = i;
            j = i;
            while(j < mem_size && !memcmp(admin + j, "\0", 1)) {
                // // printf("siguiente en: %d\n", j);
                segment_counter ++;
                j++;
            }
            i = j;
            // // printf("Data: %d - %d - %d\n", i, segment_counter, mem_size);
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
        // // printf("Direccion a devolver: %d\n", result);
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
            // // printf("Segmento Libre en: %d\n", i);
            start = i;
            j = i;
            while(!memcmp(admin + j, "\0", 1) && j < mem_size) {
                // // printf("siguiente en: %d\n", j);
                segment_counter ++;
                j++;

                if (segment_counter >= total_size) {
                    // // printf("Direccion a devolver: %d\n", start);
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

    // // printf("Values - Start: %d - End: %d\n", start_address, limit_address);
    if (start_address >= limit_address) {
        return NULL;
    }
    // log_info(logger, "Limit address:%d", limit_address);

    void *tareas = malloc(limit_address - start_address + 1);
    memcpy(tareas, memory + start_address, limit_address - start_address);
    memset(tareas + (limit_address-start_address), '\0', 1);

    // // printf("Lista: %s\n",(char*) tareas);

    int cantidadLetrasLeidas = 0;
    
    // TOMAR AIRE;2,2,T|HCONSUMIR OXIGENO P;3;3;T|HOLA

    // Get one byte of the memory as a CHAR
    // char test_c = get_char_value(tareas, counter);
    int offset = 0;
    while (memcmp(tareas + offset, ";", 1) && tareas + offset != NULL && cantidadLetrasLeidas + start_address < limit_address ) {
        // // printf("CHAR: %c\n", get_char_value(tareas, cantidadLetrasLeidas));
        
        if(get_char_value(tareas,offset) != '\n'){
            cantidadLetrasLeidas++;
        }

        offset++;
    }
    // log_info(logger, "Cantidad letras hasta primer ;%d",cantidadLetrasLeidas);

    while (!isalpha(get_char_value(tareas, cantidadLetrasLeidas)) && cantidadLetrasLeidas + start_address < limit_address){
        // // printf("CHAR: %c - %d | %d\n", get_char_value(tareas, cantidadLetrasLeidas), cantidadLetrasLeidas + start_address, limit_address);

        cantidadLetrasLeidas++;
    }
    // log_info(logger, "Cantidad letras despues de segundo while: %d",cantidadLetrasLeidas);

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

    // // printf("El segmento que se intento eliminar es invalido.\n");
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
    tcb_t *temp;

    int offset = 0;

    if (segmento -> limit < mem_size) {
        temp = malloc(sizeof(tcb_t));
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

//         // // printf("Length: %d - String: %s", read, line);

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
    // printf("Key: %s\n", key);
    segment *temp;

    int index = 0;

    t_queue *self = dictionary_get(table_collection, key);

    // printf("CANTIDAD ELEMENTOS: %d\n", self -> elements -> elements_count);

	if (self -> elements -> elements_count > 0) {

		t_link_element *element = self -> elements -> head;
		temp = element -> data;

        while (element != NULL) {

            // Segment
            // element -> data;
            // printf("Data: %d", temp -> type);

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

void save_in_file (void *element, void *memory, FILE *file, char *key) {
    segment *segmento = element;

    char *line = string_new();
    string_append_with_format(&line, "Proceso: %s\t\tSegmento: %d\t\tInicio: 0x%d\t\tTam: %db\n", key, segmento -> nroSegmento, segmento -> baseAddr, segmento -> limit - segmento -> baseAddr);
    
    txt_write_in_file(file, line);

    free(line);

}

void process_iterate(t_list *self, void(*closure)(), void *memory, FILE *file, char *key) {
    t_link_element *element = self->head;
	t_link_element *aux = NULL;
	while (element != NULL) {
		aux = element->next;
		closure(element->data, memory, file, key);
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

            process_iterate(aux -> elements, save_in_file, memory, file, element -> key);

			element = next_element;
		}
	}

    txt_write_in_file(file, "--------------------------------------------------------------------------\n");

    txt_close_file(file);
}


// --------------------- END DUMP ----------------------- //