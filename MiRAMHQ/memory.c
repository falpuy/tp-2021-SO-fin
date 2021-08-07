#include "memory.h"

void *memory_init(int size) {
    void *temp = malloc(size);
    return temp;
}


// // ----------------------- PAGINATION ------------------------- //

// bool lru_sorter(void *uno, void *dos) {
//     page_t *page1 = uno;
//     page_t *page2 = dos;

//     return page1 -> frame -> time < page2 -> frame -> time;
// }

void lru_replacer(void *item) {
    page_t *page = (page_t *) item;

    if (global_lru_page == NULL && page -> frame -> presence) {
        pthread_mutex_lock(&m_global_lru_page);
        global_lru_page = page;
        pthread_mutex_unlock(&m_global_lru_page);
    } else {
        if (page -> frame -> presence && global_lru_page -> frame -> time > page -> frame -> time) {
            pthread_mutex_lock(&m_global_lru_page);
            global_lru_page = page;
            pthread_mutex_unlock(&m_global_lru_page);
        }
    }
    
}

void lru_iterator(char *key, void *item) {
    t_queue *pages = (t_queue *) item;

    // list_sort(pages -> elements, lru_sorter);
    list_iterate(pages -> elements, lru_replacer);
}

frame_t *get_next_lru_frame() {
    // // printf("Checkeando LRU %d\n", dictionary_is_empty(table_collection));
    pthread_mutex_lock(&m_global_lru_page);
    global_lru_page = NULL;
    pthread_mutex_unlock(&m_global_lru_page);

    dictionary_iterator(table_collection, lru_iterator);

    return global_lru_page -> frame;

}

void clock_replacer(void *item, int key_index, int frame_index) {
    page_t *page = item;

    if (page -> frame -> presence && page -> frame -> modified == 0) {
        page -> frame -> modified = 1;
        pthread_mutex_lock(&m_global_clock_page);
        global_clock_page = page;
        pthread_mutex_unlock(&m_global_clock_page);
        clock_flag = 1;
        pthread_mutex_lock(&m_global_clock_key);
        global_clock_key = key_index;
        pthread_mutex_unlock(&m_global_clock_key);
        pthread_mutex_lock(&m_global_clock_index);
        global_clock_index = frame_index;
        pthread_mutex_unlock(&m_global_clock_index);
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
        pthread_mutex_lock(&m_global_clock_index);
        global_clock_index = 0;
        pthread_mutex_unlock(&m_global_clock_index);
    }
}

frame_t *get_next_clock_frame() {
    // // printf("Checkeando CLOCK %d\n", dictionary_is_empty(table_collection));
    pthread_mutex_lock(&m_global_clock_page);
    global_clock_page = NULL;
    pthread_mutex_unlock(&m_global_clock_page);

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
            pthread_mutex_lock(&m_global_clock_key);
            global_clock_key = 0;
            pthread_mutex_unlock(&m_global_clock_key);
            pthread_mutex_lock(&m_global_clock_index);
            global_clock_index = 0;
            pthread_mutex_unlock(&m_global_clock_index);
        }
    }

    return global_clock_page -> frame;

}

void set_bitmap(uint8_t *bitmap, int position) {
    pthread_mutex_lock(&mbitmap);
    bitmap[position] = 1;
    pthread_mutex_unlock(&mbitmap);
}

void unset_bitmap(uint8_t *bitmap, int position) {
    pthread_mutex_lock(&mbitmap);
    bitmap[position] = 0;
    pthread_mutex_unlock(&mbitmap);
}

uint32_t get_frame() {
    // int pr = 100;
    int value;
    int err;
    // printf("get %d\n", pr++);
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
    // printf("get %d\n", pr++);
    // // printf("Reemplazo este frame: %d\n", replacing_frame -> number);

    if (replacing_frame != NULL) {
        // busco lugar en virtual
        for (int i = 0; i < frames_virtual; i++) {
            // // printf("GET FRAME Bit %d: %d\n", i, bitarray_test_bit(bitmap, i));
            if (!bitarray_test_bit(virtual_bitmap, i)) {
                // // printf("Guardo en virtual %d\n", i);
                // pego la data desde memoria
                // printf("get v %d\n", pr++);
                pthread_mutex_lock(&m_memoria);
                pthread_mutex_lock(&m_virtual);
                memcpy(virtual_memory + i * page_size, memory + replacing_frame -> number * page_size, page_size);
                pthread_mutex_unlock(&m_virtual);
                pthread_mutex_unlock(&m_memoria);
                // printf("get m %d\n", pr++);
                // err = msync(virtual_memory, virtual_size + frames_virtual / 8, MS_ASYNC);
                // if (err == -1){
                //     perror("Error de sincronizar a disco memoria");
                //     return -1;
                // }
                // seteo el bit de virtual
                pthread_mutex_lock(&mbitmapv);
                bitarray_set_bit(virtual_bitmap, i);
                pthread_mutex_unlock(&mbitmapv);
                // unseteo el bit
                unset_bitmap(bitmap, replacing_frame -> number);
                // Actualizo el valor a devolver
                value = replacing_frame -> number;

                //updateo el frame que muevo
                replacing_frame -> number = i;
                replacing_frame -> start = i * page_size;
                replacing_frame -> presence = 0;
                // replacing_frame -> time = timer++;
                // replacing_frame -> modified = 1;
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
    int pnumber = 0;
    void *temp;
    pcb_t *p_aux = malloc(sizeof(pcb_t));
    int task_size;
    int tcb_count;
    int offset = 0;
    int tid, xpos, ypos;
    int task_start = 0;

    p_aux -> tasks = 0; // Al leer un frame, las tareas siempre comienzan en la posicion 8 del frame leido (paginacion)
    memcpy(&p_aux -> pid, buffer + offset, sizeof(uint32_t));
    offset += sizeof(int);
    
    memcpy(&task_size, buffer + offset, sizeof(int));
    offset += sizeof(int);

    void *tasks = malloc(task_size + 1);
    memcpy(tasks, buffer + offset, task_size);
    memset(tasks + task_size, '\0', 1);
    offset += task_size;

    memcpy(&tcb_count, buffer + offset, sizeof(int));
    offset += sizeof(int);

    int memory_size = sizeof(pcb_t) + task_size + tcb_count * 21;

    admin_data *tcb_data = malloc(sizeof(admin_data));
    tcb_data -> cantidad = tcb_count;
    tcb_data -> start = sizeof(pcb_t) + task_size;
    tcb_data -> page_number = (sizeof(pcb_t) + task_size) / page_size;
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

    temp = malloc(frames_count * page_size);
    int temp_off = 0;

    // Me guardo el ID del pcb como string para el diccionario
    char *pid = string_itoa(p_aux -> pid);
    int p_id = p_aux -> pid;
    
    // GUARDO TAREAS
    memcpy(temp + temp_off, tasks, task_size);
    temp_off += task_size;

    free(tasks);

    // GUARDO PCB
    memcpy(temp + temp_off, &p_aux -> pid, sizeof(uint32_t));
    temp_off += sizeof(uint32_t);

    memcpy(temp + temp_off, &p_aux -> tasks, sizeof(uint32_t));
    temp_off += sizeof(uint32_t);
    free(p_aux);

    pthread_mutex_lock(&madmin);
    dictionary_put(admin_collection, pid, tcb_data);
    pthread_mutex_unlock(&madmin);

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
        // printf("tcb %d\n", pr++);
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
        // offset += sizeof(uint32_t);

        pthread_mutex_lock(&m_map);
        personaje_crear(nivel, tid, xpos, ypos);
        nivel_gui_dibujar(nivel);
        pthread_mutex_unlock(&m_map);
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
            frame -> modified = 1;
            frame -> number = n_frame;
            frame -> start = n_frame * page_size;
            frame -> modified = 1;
            frame -> presence = 1;

            // Creo pagina
            page_t *page = malloc(sizeof(page_t));
            page -> number = pnumber++;
            page -> frame = frame;

            // if (bytes_left < page_size) {
                // copio bytes_left
            pthread_mutex_lock(&m_memoria);
            memcpy(memory + (n_frame * page_size), temp + (j * page_size), page_size);
            pthread_mutex_unlock(&m_memoria);
            // } else {
            //     // copio page_size
            //     pthread_mutex_lock(&m_memoria);
            //     memcpy(memory + (n_frame * page_size), temp + (j * page_size), page_size);
            //     pthread_mutex_unlock(&m_memoria);
            //     bytes_left -= page_size;
            // }

            queue_push(tabla, page);
            // Agrego tabla al diccionario
            pthread_mutex_lock(&mdictionary);
            dictionary_put(table_collection, pid, tabla);
            pthread_mutex_unlock(&mdictionary);
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

page_t * _list_find(t_list *self, int id) {
    t_link_element *element = self->head;
	t_link_element *aux = NULL;
	while (element != NULL) {
		aux = element->next;
		
        page_t *page = (page_t *) element -> data;

        if (page -> number == id) {
            return page;
        }

		element = aux;
	}

    return NULL;
}

bool sort_by_id(void *item1, void *item2) {
    page_t *page_aux = (page_t *)item1;
    page_t *page_aux2 = (page_t *)item2;

    return page_aux -> number < page_aux2 -> number;
}

void update_task_from_page(void *memory, t_dictionary *admin_collection, t_dictionary *table_collection, char *key, int id_tcb, int next) {
    // // printf("Obtengo tablas del proceso..\n");
    pthread_mutex_lock(&mdictionary);
    t_queue *self = dictionary_get(table_collection, key);
    pthread_mutex_unlock(&mdictionary);
    pthread_mutex_lock(&madmin);
    admin_data *data_tcb = dictionary_get(admin_collection, key);
    pthread_mutex_unlock(&madmin);

    page_t *page_aux;

    int original_size = queue_size(self);

    // // printf("Obtengo las paginas en memoria..: %d\n", original_size);

    int off = 0;
    int tcb_page_count = (queue_size(self) - data_tcb -> page_number);

    void *temp = malloc(tcb_page_count * page_size);

    // Traigo paginas de tcb
    for(int i = data_tcb -> page_number; i < queue_size(self); i++) {
        page_aux = list_remove(self -> elements, i);

        if (page_aux -> frame -> presence) {
            pthread_mutex_lock(&m_memoria);
            memcpy(temp + (off * page_size), memory + (page_aux -> frame) -> start, page_size);
            pthread_mutex_unlock(&m_memoria);
            page_aux -> frame -> time = timer++;
            page_aux -> frame -> modified = 1;

            // unset_bitmap(bitmap, (page_aux -> frame) -> number);
        } else {
            pthread_mutex_lock(&m_virtual);
            memcpy(temp + (off * page_size), virtual_memory + (page_aux -> frame) -> start, page_size);
            pthread_mutex_unlock(&m_virtual);
            pthread_mutex_lock(&mbitmapv);
            bitarray_clean_bit(virtual_bitmap, (page_aux -> frame) -> number);
            pthread_mutex_unlock(&mbitmapv);

            page_aux -> frame -> presence = 1;

            int new_frame = get_frame();
            pthread_mutex_lock(&m_memoria);
            pthread_mutex_lock(&m_virtual);
            memcpy(memory + (new_frame * page_size), temp + (off * page_size), page_size);
            pthread_mutex_unlock(&m_virtual);
            pthread_mutex_unlock(&m_memoria);
            set_bitmap(bitmap, new_frame);

            // Creo frame
            page_aux -> frame -> time = timer++;
            page_aux -> frame -> modified = 1;
            page_aux -> frame -> number = new_frame;
            page_aux -> frame -> start = new_frame * page_size;
            page_aux -> frame -> modified = 1;
            page_aux -> frame -> presence = 1;
        }

        list_add_sorted(self -> elements, page_aux, sort_by_id);

        off++;

    }

    // --- Getting tcb list from temp
    uint32_t temp_id;
    int tcb_left;
    int pos_tcb;
    int auxoff = 0;
    int falta_pagina = 0;
    int falta_pagina2 = 0;
    page_t *page2;
    for (int i = 0; i < data_tcb -> cantidad; i++) {
        // Leo solo el primer int, que representa el tid
        memcpy(&temp_id, temp + (data_tcb -> start % page_size)  + (i * 21), sizeof(uint32_t));

        if (temp_id == id_tcb) {

            pos_tcb = (data_tcb -> start % page_size) + (i * 21) + sizeof(uint32_t) * 4 + sizeof(char);

            // updateo el tcb

            memcpy(temp + pos_tcb, &next, sizeof(uint32_t));
            int ofset= 0;
            page_t *tcb_page;
            for(int i = data_tcb -> page_number; i < queue_size(self); i++) {
                tcb_page = _list_find(self -> elements, i);
                pthread_mutex_lock(&m_memoria);
                memcpy(memory + tcb_page -> frame -> number * page_size, temp + ofset, page_size);
                pthread_mutex_unlock(&m_memoria);
                ofset += page_size;
            }

            break;
        }
    }

    free(temp);
}

char *get_task_from_page(void *memory, t_dictionary *admin_collection, t_dictionary *table_collection, char *key, int id_tcb) {
    // // printf("Obtengo tablas del proceso..\n");
    int pagecount = 1;
    pthread_mutex_lock(&mdictionary);
    t_queue *self = dictionary_get(table_collection, key);
    pthread_mutex_unlock(&mdictionary);
    pthread_mutex_lock(&madmin);
    admin_data *data_tcb = dictionary_get(admin_collection, key);
    pthread_mutex_unlock(&madmin);

    page_t *page_aux;

    int original_size = queue_size(self);

    char *nextTask;

    char *recv_task;

    int off = 0;

    int tcb_page_count = (queue_size(self) - data_tcb -> page_number);

    void *temp = malloc(tcb_page_count * page_size);
    // Traigo paginas de tcb
    for(int i = data_tcb -> page_number; i < queue_size(self); i++) {
        // printf("NUMBERS: %d - %d\n", i, queue_size(self));
        page_aux = list_remove(self -> elements, i);
        // printf("Page: %d\n", page_aux -> number);

        if (page_aux -> frame -> presence) {
            pthread_mutex_lock(&m_memoria);
            memcpy(temp + (off * page_size), memory + (page_aux -> frame) -> start, page_size);
            pthread_mutex_unlock(&m_memoria);
            page_aux -> frame -> time = timer++;
            page_aux -> frame -> modified = 1;

            // unset_bitmap(bitmap, (page_aux -> frame) -> number);
        } else {
            pthread_mutex_lock(&m_virtual);
            memcpy(temp + (off * page_size), virtual_memory + (page_aux -> frame) -> start, page_size);
            pthread_mutex_unlock(&m_virtual);
            pthread_mutex_lock(&mbitmapv);
            bitarray_clean_bit(virtual_bitmap, (page_aux -> frame) -> number);
            pthread_mutex_unlock(&mbitmapv);

            page_aux -> frame -> presence = 1;

            int new_frame = get_frame();

            pthread_mutex_lock(&m_memoria);
            pthread_mutex_lock(&m_virtual);
            memcpy(memory + (new_frame * page_size), temp + (off * page_size), page_size);
            pthread_mutex_unlock(&m_virtual);
            pthread_mutex_unlock(&m_memoria);
            set_bitmap(bitmap, new_frame);

            // Creo frame
            page_aux -> frame -> time = timer++;
            page_aux -> frame -> modified = 1;
            page_aux -> frame -> number = new_frame;
            page_aux -> frame -> start = new_frame * page_size;
            page_aux -> frame -> modified = 1;
            page_aux -> frame -> presence = 1;
        }

        off++;

        list_add_sorted(self -> elements, page_aux, sort_by_id);

    }

    // --- Getting tcb list from temp
    uint32_t temp_id;
    int tcb_left;
    int pos_tcb;
    int auxoff = 0;
    int falta_pagina = 0;
    int falta_pagina2 = 0;
    page_t *page2;
    char *tareas = malloc(page_size * 2);
    for (int i = 0; i < data_tcb -> cantidad; i++) {
        // Leo solo el primer int, que representa el tid
        // printf("Searching.. %d - %d\n", data_tcb -> start , (data_tcb -> start % page_size));
        memcpy(&temp_id, temp + (data_tcb -> start % page_size) + (i * 21), sizeof(uint32_t));

        if (temp_id == id_tcb) {

            // printf("Encontre el id..\n");

            pos_tcb = (data_tcb -> start % page_size) + (i * 21) + (sizeof(uint32_t) * 4) + sizeof(char);

            int task_counter = 0;

            int prevTask;

            memcpy(&prevTask, temp + pos_tcb, sizeof(uint32_t));

            // busco la pagina de tareas

            int pnumber = prevTask / page_size;

            page2 = list_remove(self -> elements, pnumber);

            if (page2 -> frame -> presence) {
                pthread_mutex_lock(&m_memoria);
                memcpy(tareas, memory + (page2 -> frame) -> start, page_size);
                pthread_mutex_unlock(&m_memoria);
                page2 -> frame -> time = timer++;
                page2 -> frame -> modified = 1;
                
            } else {
                pthread_mutex_lock(&m_virtual);
                memcpy(tareas, virtual_memory + (page2 -> frame) -> start, page_size);
                pthread_mutex_unlock(&m_virtual);
                pthread_mutex_lock(&mbitmapv);
                bitarray_clean_bit(virtual_bitmap, (page2 -> frame) -> number);
                pthread_mutex_unlock(&mbitmapv);

                page2 -> frame -> presence = 1;

                int new_frame = get_frame();

                pthread_mutex_lock(&m_memoria);
                memcpy(memory + (new_frame * page_size), tareas, page_size);
                pthread_mutex_unlock(&m_memoria);
                set_bitmap(bitmap, new_frame);

                // Creo frame
                page2 -> frame -> time = timer++;
                page2 -> frame -> modified = 1;
                page2 -> frame -> number = new_frame;
                page2 -> frame -> start = new_frame * page_size;
                page2 -> frame -> modified = 1;

            }

            list_add_sorted(self -> elements, page2, sort_by_id);

            // printf("Empiezo en pos %d\n", (prevTask % page_size != 0 ? prevTask % page_size : prevTask));
            // printf("PAGINA: %d - %d - %d - %d\n", pnumber, prevTask + task_counter, prevTask, memcmp(tareas + (prevTask % page_size) + task_counter, ";", 1));

            // busco la tarea
            while (!falta_pagina && prevTask + task_counter < ((data_tcb -> start ) - 8) && memcmp(tareas + (prevTask % page_size) + task_counter, "|", 1)) {

                task_counter++;
                if ((prevTask + task_counter) % page_size == 0) {
                    falta_pagina = 1;
                }
            }

            if (falta_pagina) {
                // tareas = realloc(tareas, page_size * 2);
                page2 = list_remove(self -> elements, pnumber + pagecount++);

                if (page2 -> frame -> presence) {
                    pthread_mutex_lock(&m_memoria);
                    memcpy(tareas + page_size, memory + (page2 -> frame) -> start, page_size);
                    pthread_mutex_unlock(&m_memoria);
                    page2 -> frame -> time = timer++;
                    page2 -> frame -> modified = 1;
                    
                    // unset_bitmap(bitmap, (page2 -> frame) -> number);
                } else {
                    pthread_mutex_lock(&m_virtual);
                    memcpy(tareas + page_size, virtual_memory + (page2 -> frame) -> start, page_size);
                    pthread_mutex_unlock(&m_virtual);
                    pthread_mutex_lock(&mbitmapv);
                    bitarray_clean_bit(virtual_bitmap, (page2 -> frame) -> number);
                    pthread_mutex_unlock(&mbitmapv);

                    page2 -> frame -> presence = 1;

                    int new_frame = get_frame();
                    
                    pthread_mutex_lock(&m_memoria);
                    pthread_mutex_lock(&m_virtual);
                    memcpy(memory + (new_frame * page_size), tareas + page_size, page_size);
                    pthread_mutex_unlock(&m_virtual);
                    pthread_mutex_unlock(&m_memoria);
                    set_bitmap(bitmap, new_frame);

                    // Creo frame
                    page2 -> frame -> time = timer++;
                    page2 -> frame -> modified = 1;
                    page2 -> frame -> number = new_frame;
                    page2 -> frame -> start = new_frame * page_size;
                    page2 -> frame -> modified = 1;

                }

                list_add_sorted(self -> elements, page2, sort_by_id);

                while (prevTask + task_counter < ((data_tcb -> start ) - 8) && memcmp(tareas + (prevTask % page_size) + task_counter, "|", 1)) {
                    
                    task_counter++;
                }
            }

            if (prevTask < ((data_tcb -> start ) - 8) && prevTask + task_counter <= (data_tcb -> start )) {
                recv_task = malloc(task_counter + 1);
                memcpy(recv_task, tareas + (prevTask % page_size), task_counter);
                recv_task[task_counter] = '\0';
            } else {
                recv_task = NULL;
            }

            // updateo el tcb
            int next_addr = prevTask + task_counter + 1;

            int ofset= 0;
            page_t *tcb_page;

            update_task_from_page(memory, admin_collection, table_collection, key, id_tcb, next_addr);

            break;
        }
    }

    free(temp);
    free(tareas);

    return recv_task;
}

int remove_tcb_from_page(void *memory, t_dictionary *admin_collection, t_dictionary *table_collection, char *key, int id_tcb) {
    // // printf("Obtengo tablas del proceso..\n");
    pthread_mutex_lock(&mdictionary);
    t_queue *self = dictionary_get(table_collection, key);
    pthread_mutex_unlock(&mdictionary);
    pthread_mutex_lock(&madmin);
    admin_data *data_tcb = dictionary_get(admin_collection, key);
    pthread_mutex_unlock(&madmin);

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
            pthread_mutex_lock(&m_memoria);
            memcpy(temp + (off * page_size), memory + (page_aux -> frame) -> start, page_size);
            pthread_mutex_unlock(&m_memoria);
            page_aux -> frame -> time = timer++;
            page_aux -> frame -> modified = 1;
            unset_bitmap(bitmap, (page_aux -> frame) -> number);
        } else {
            // printf("SWAP PAGE\n");
            // TODO: SWAPEAR
            pthread_mutex_lock(&m_virtual);
            memcpy(temp + (off * page_size), virtual_memory + (page_aux -> frame) -> start, page_size);
            pthread_mutex_unlock(&m_virtual);
            pthread_mutex_lock(&mbitmapv);
            bitarray_clean_bit(virtual_bitmap, (page_aux -> frame) -> number);
            pthread_mutex_unlock(&mbitmapv);
            
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
        memcpy(&temp_id, (temp + data_tcb -> start) + (i * 21), sizeof(uint32_t));
        
        if (temp_id == id_tcb) {

            // // printf("Entre al if...\n");

            data_tcb -> tcb[i] = 0;
            tcb_left = data_tcb -> cantidad - i - 1;

            // Hago la compactacion de los tcb restantes
            memcpy((temp + data_tcb -> start) + (i * 21), (temp + data_tcb -> start) + ((i + 1) * 21), tcb_left * 21);
            data_tcb -> cantidad -= 1;

            break;
        }
    }

    if (temp_id != id_tcb) {
        return 0;
    }

    int size_a_copiar = data_tcb -> start + data_tcb -> cantidad * 21;

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
            frame -> modified = 1;
            frame -> number = n_frame;
            frame -> start = n_frame * page_size;
            frame -> modified = 1;
            frame -> presence = 1;

            // Creo pagina
            page_t *page = malloc(sizeof(page_t));
            page -> number = queue_size(self);
            page -> frame = frame;
            pthread_mutex_lock(&m_memoria);
            memcpy(memory + (n_frame * page_size), temp + posicion_temp * page_size, page_size);
            pthread_mutex_unlock(&m_memoria);

            queue_push(self, page);

            size_a_copiar -= page_size;
        } else {
            n_frame = get_frame();
            set_bitmap(bitmap, n_frame);

            // Creo frame
            frame_t *frame = malloc(sizeof(frame_t));
            frame -> time = timer++;
            frame -> modified = 1;
            frame -> number = n_frame;
            frame -> start = n_frame * page_size;
            frame -> modified = 1;
            frame -> presence = 1;

            // Creo pagina
            page_t *page = malloc(sizeof(page_t));
            page -> frame = frame;
            page -> number = queue_size(self);
            pthread_mutex_lock(&m_memoria);
            memcpy(memory + (n_frame * page_size), temp + posicion_temp * page_size, size_a_copiar);
            pthread_mutex_unlock(&m_memoria);

            queue_push(self, page);

            size_a_copiar -= size_a_copiar;
        }
        posicion_temp++;
    }

    free(temp);
    pthread_mutex_lock(&mdictionary);
    dictionary_put(table_collection, key, self);
    pthread_mutex_unlock(&mdictionary);

    return 1;
}

void remove_pcb_from_page(void *memory, t_dictionary *admin_collection, t_dictionary *table_collection, char *key) {
    pthread_mutex_lock(&mdictionary);
    t_queue *self = dictionary_get(table_collection, key);
    pthread_mutex_unlock(&mdictionary);
    pthread_mutex_lock(&madmin);
    admin_data *data_tcb = dictionary_get(admin_collection, key);
    dictionary_remove_and_destroy(admin_collection, key, admin_destroyer);
    pthread_mutex_unlock(&madmin);

    page_t *page_aux;

    while(queue_size(self) > 0) {
        page_aux = queue_pop(self);

        if (! (page_aux ->frame) -> presence) {
            int new_frame = get_frame();
            page_aux -> frame -> time = timer++;
            page_aux -> frame -> modified = 1;
            page_aux -> frame -> number = new_frame;
            page_aux -> frame -> start = new_frame * page_size;
            page_aux -> frame -> modified = 1;
            page_aux -> frame -> presence = 1;

            unset_bitmap(bitmap, new_frame);
        } else {
            unset_bitmap(bitmap, (page_aux -> frame) -> number);
        }

        free(page_aux -> frame);
        free(page_aux);
    }
    pthread_mutex_lock(&mdictionary);
    dictionary_put(table_collection, key, self);
    pthread_mutex_unlock(&mdictionary);
}

void update_position_from_page(void *memory, t_dictionary *admin_collection, t_dictionary *table_collection, char *key, int id_tcb, int posx, int posy) {
    // // printf("Obtengo tablas del proceso..\n");
    pthread_mutex_lock(&mdictionary);
    t_queue *self = dictionary_get(table_collection, key);
    pthread_mutex_unlock(&mdictionary);
    pthread_mutex_lock(&madmin);
    admin_data *data_tcb = dictionary_get(admin_collection, key);
    pthread_mutex_unlock(&madmin);

    page_t *page_aux;

    int original_size = queue_size(self);

    // // printf("Obtengo las paginas en memoria..: %d\n", original_size);

    int off = 0;
    int tcb_page_count = (queue_size(self) - data_tcb -> page_number);

    void *temp = malloc(tcb_page_count * page_size);

    // Traigo paginas de tcb
    for(int i = data_tcb -> page_number; i < queue_size(self); i++) {
        page_aux = list_remove(self -> elements, i);

        if (page_aux -> frame -> presence) {
            pthread_mutex_lock(&m_memoria);
            memcpy(temp + (off * page_size), memory + (page_aux -> frame) -> start, page_size);
            pthread_mutex_unlock(&m_memoria);
            page_aux -> frame -> time = timer++;
            page_aux -> frame -> modified = 1;

            // unset_bitmap(bitmap, (page_aux -> frame) -> number);
        } else {
            pthread_mutex_lock(&m_virtual);
            memcpy(temp + (off * page_size), virtual_memory + (page_aux -> frame) -> start, page_size);
            pthread_mutex_unlock(&m_virtual);
            pthread_mutex_lock(&mbitmapv);
            bitarray_clean_bit(virtual_bitmap, (page_aux -> frame) -> number);
            pthread_mutex_unlock(&mbitmapv);

            page_aux -> frame -> presence = 1;

            int new_frame = get_frame();
            pthread_mutex_lock(&m_memoria);
            pthread_mutex_lock(&m_virtual);
            memcpy(memory + (new_frame * page_size), temp + (off * page_size), page_size);
            pthread_mutex_unlock(&m_virtual);
            pthread_mutex_unlock(&m_memoria);
            set_bitmap(bitmap, new_frame);

            // Creo frame
            page_aux -> frame -> time = timer++;
            page_aux -> frame -> modified = 1;
            page_aux -> frame -> number = new_frame;
            page_aux -> frame -> start = new_frame * page_size;
            page_aux -> frame -> modified = 1;
            page_aux -> frame -> presence = 1;
        }
        list_add_sorted(self -> elements, page_aux, sort_by_id);

        off++;

    }

    // --- Getting tcb list from temp
    uint32_t temp_id;
    int tcb_left;
    int pos_tcb;
    int auxoff = 0;
    int falta_pagina = 0;
    int falta_pagina2 = 0;
    page_t *page2;
    for (int i = 0; i < data_tcb -> cantidad; i++) {
        // Leo solo el primer int, que representa el tid
        memcpy(&temp_id, temp + (data_tcb -> start % page_size)  + (i * 21), sizeof(uint32_t));

        if (temp_id == id_tcb) {

            pos_tcb = (data_tcb -> start % page_size) + (i * 21) + sizeof(uint32_t) * 2 + sizeof(char);

            // updateo el tcb

            memcpy(temp + pos_tcb, &posx, sizeof(uint32_t));
            memcpy(temp + pos_tcb + sizeof(uint32_t), &posy, sizeof(uint32_t));
            int ofset= 0;
            page_t *tcb_page;
            for(int i = data_tcb -> page_number; i < queue_size(self); i++) {
                tcb_page = _list_find(self -> elements, i);
                pthread_mutex_lock(&m_memoria);
                memcpy(memory + tcb_page -> frame -> number * page_size, temp + ofset, page_size);
                pthread_mutex_unlock(&m_memoria);
                ofset += page_size;
            }

            break;
        }
    }

    free(temp);
}


void update_status_from_page(void *memory, t_dictionary *admin_collection, t_dictionary *table_collection, char *key, int id_tcb, char status) {
    // // printf("Obtengo tablas del proceso..\n");
    pthread_mutex_lock(&mdictionary);
    t_queue *self = dictionary_get(table_collection, key);
    pthread_mutex_unlock(&mdictionary);
    pthread_mutex_lock(&madmin);
    admin_data *data_tcb = dictionary_get(admin_collection, key);
    pthread_mutex_unlock(&madmin);

    page_t *page_aux;

    int original_size = queue_size(self);

    // // printf("Obtengo las paginas en memoria..: %d\n", original_size);

    int off = 0;
    int tcb_page_count = (queue_size(self) - data_tcb -> page_number);

    void *temp = malloc(tcb_page_count * page_size);

    // Traigo paginas de tcb
    for(int i = data_tcb -> page_number; i < queue_size(self); i++) {
        page_aux = list_remove(self -> elements, i);

        if (page_aux -> frame -> presence) {
            pthread_mutex_lock(&m_memoria);
            memcpy(temp + (off * page_size), memory + (page_aux -> frame) -> start, page_size);
            pthread_mutex_unlock(&m_memoria);
            page_aux -> frame -> time = timer++;
            page_aux -> frame -> modified = 1;

            // unset_bitmap(bitmap, (page_aux -> frame) -> number);
        } else {
            pthread_mutex_lock(&m_virtual);
            memcpy(temp + (off * page_size), virtual_memory + (page_aux -> frame) -> start, page_size);
            pthread_mutex_unlock(&m_virtual);
            pthread_mutex_lock(&mbitmapv);
            bitarray_clean_bit(virtual_bitmap, (page_aux -> frame) -> number);
            pthread_mutex_unlock(&mbitmapv);

            page_aux -> frame -> presence = 1;

            int new_frame = get_frame();
            pthread_mutex_lock(&m_memoria);
            pthread_mutex_lock(&m_virtual);
            memcpy(memory + (new_frame * page_size), temp + (off * page_size), page_size);
            pthread_mutex_unlock(&m_virtual);
            pthread_mutex_unlock(&m_memoria);
            set_bitmap(bitmap, new_frame);

            // Creo frame
            page_aux -> frame -> time = timer++;
            page_aux -> frame -> modified = 1;
            page_aux -> frame -> number = new_frame;
            page_aux -> frame -> start = new_frame * page_size;
            page_aux -> frame -> modified = 1;
            page_aux -> frame -> presence = 1;
        }
        list_add_sorted(self -> elements, page_aux, sort_by_id);
        off++;

    }

    // --- Getting tcb list from temp
    uint32_t temp_id;
    int tcb_left;
    int pos_tcb;
    int auxoff = 0;
    int falta_pagina = 0;
    int falta_pagina2 = 0;
    page_t *page2;
    for (int i = 0; i < data_tcb -> cantidad; i++) {
        // Leo solo el primer int, que representa el tid
        memcpy(&temp_id, temp + (data_tcb -> start % page_size)  + (i * 21), sizeof(uint32_t));

        if (temp_id == id_tcb) {

            pos_tcb = (data_tcb -> start % page_size) + (i * 21) + sizeof(uint32_t) * 2;

            // updateo el tcb

            memcpy(temp + pos_tcb, &status, sizeof(char));
            int ofset= 0;
            page_t *tcb_page;
            for(int i = data_tcb -> page_number; i < queue_size(self); i++) {
                tcb_page = _list_find(self -> elements, i);
                pthread_mutex_lock(&m_memoria);
                memcpy(memory + tcb_page -> frame -> number * page_size, temp + ofset, page_size);
                pthread_mutex_unlock(&m_memoria);
                ofset += page_size;
            }

            break;
        }
    }

    free(temp);
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

// void memory_compaction(void *admin, void *memory, int mem_size, t_dictionary* self) {
//     void *aux_memory = malloc(mem_size);
//     memset(admin, 0, mem_size);

//     segment *temp;

//     t_queue *aux;

//     int data_size;

//     int offset = 0;

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

//                     // printf("Copiando elementos.. %d - %d - %d\n", temp -> nroSegmento, temp -> baseAddr, temp -> limit);
                    
//                     // Copio los datos del segmento en la memoria auxiliar
//                     memcpy(aux_memory + offset, memory + temp -> baseAddr, data_size);
//                     memset(admin + offset, 1, data_size);

//                     new_base = offset;
//                     new_limit = offset + data_size;

//                     // printf("Creando nuevo Segmento.. %d - %d - %d\n", temp -> nroSegmento, new_base, new_limit);

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

void memory_compaction(void *admin, void *memory, int mem_size, t_dictionary* self) {
    void *aux_memory = malloc(mem_size);
    // memset(admin, 0, mem_size);

    segment *temp;

    t_queue *aux;

    int data_size;

    int offset = 0;

    int new_base;
    int new_limit;

    t_queue *temp_list = queue_create();

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

                    // data_size = temp -> limit - temp ->baseAddr;

                    // // Copio los datos del segmento en la memoria auxiliar
                    // pthread_mutex_lock(&m_memoria);
                    // memcpy(aux_memory + offset, memory + temp -> baseAddr, data_size);
                    // pthread_mutex_unlock(&m_memoria);

                    list_add_sorted(temp_list -> elements, temp, sort_by_addr);

                    // new_base = offset;
                    // new_limit = offset + data_size;

                    // offset += data_size;

                    // temp -> baseAddr = new_base;
                    // temp -> limit = new_limit;

                    element = element->next;
                    if (element != NULL)
                        temp = element -> data;
                    
                }
            }
            element = next_element;
        }

	}

    // REACOMODO LOS SEGMENTOS AL PRINCIPIO, EN EL ORDEN QE ESTABAN
    t_link_element *_element = (temp_list -> elements) -> head;
	t_link_element *_aux = NULL;
	while (_element != NULL) {
		_aux = _element->next;

        temp = _element->data;

        data_size = temp -> limit - temp ->baseAddr;

        // Copio los datos del segmento en la memoria auxiliar
        pthread_mutex_lock(&m_memoria);
        memcpy(aux_memory + offset, memory + temp -> baseAddr, data_size);
        pthread_mutex_unlock(&m_memoria);

        new_base = offset;
        new_limit = offset + data_size;

        offset += data_size;

        temp -> baseAddr = new_base;
        temp -> limit = new_limit;

		// closure(_element->data);
		_element = _aux;
	}

    queue_destroy_and_destroy_elements(segmentosLibres, destroyer);
    segmentosLibres = queue_create();
    segment *first = malloc(sizeof(segment));
    first -> id = -1;
    first -> type = -1;
    first -> nroSegmento = -1;
    first -> baseAddr = offset;
    first -> limit = mem_size;

    pthread_mutex_lock(&mtablasegmentos);
    queue_push(segmentosLibres, first);
    pthread_mutex_unlock(&mtablasegmentos);

    if (!dictionary_is_empty(self)) {
        // memset(memory, 0, mem_size);
        pthread_mutex_lock(&m_memoria);
        memcpy(memory, aux_memory, mem_size);
        pthread_mutex_unlock(&m_memoria);
    }

    free(aux_memory);

    queue_clean(temp_list);
    queue_destroy(temp_list);
}

// int check_space_memory(void *admin, int mem_size, int total_size, t_dictionary *table_collection) {

//     if (total_size > mem_size) {
//         return 0;
//     }

//     // Contador de bytes libres en la memoria
//     int segment_counter = 0;

//     for(int i = 0; i < mem_size; i ++) {
//         if (!memcmp(admin + i, "\0", 1)) {

//             // // printf("Direccion vacia: %d\n", i);
//             segment_counter ++;
//             if (segment_counter == total_size) {
//                 // // printf("Hay espacio disponible en memoria..\n");
//                 return 1;
//             }
//         }
//     }
//     return 0;
// }

int mem_space(void *admin, int mem_size, int total_size, t_dictionary *table_collection) {

    if (total_size > mem_size) {
        return 0;
    }

    int sum = 0;

    segment *temp;

    t_list *self = segmentosLibres -> elements;

    t_link_element *element = self->head;
	t_link_element *aux = NULL;
	while (element != NULL) {
		aux = element->next;

        temp = (segment *) element -> data;
        
        sum += temp -> limit - temp -> baseAddr;

		element = aux;
	}

     if (sum >= total_size) {
        return 1;
    }

    return 0;
}

int check_space_memory(void *admin, int mem_size, int total_size, t_dictionary *table_collection) {

    if (total_size > mem_size) {
        return 0;
    }

    segment *temp;

    t_list *self = segmentosLibres -> elements;

    t_link_element *element = self->head;
	t_link_element *aux = NULL;
	while (element != NULL) {
		aux = element->next;

        temp = (segment *) element -> data;
        
        if (temp -> limit - temp -> baseAddr >= total_size) {
            return 1;
        }

		element = aux;
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

bool list_sorter(void *a, void *b) {
    segment *aux = (segment *) a;
    segment *temp = (segment *) b;

    return aux -> limit - aux -> baseAddr <= temp -> limit - temp -> baseAddr;
}

int memory_best_fit(void *admin, int mem_size, t_dictionary *collection, int total_size) {

    segment *temp;

    int index = 0;

    int start;

    t_list *self = segmentosLibres -> elements;

    list_sort(self, list_sorter);

    t_link_element *element = self->head;
	t_link_element *aux = NULL;
	while (element != NULL) {
		aux = element->next;
		
        temp = (segment *) element -> data;

        if (temp -> limit - temp -> baseAddr >= total_size) {
            if (temp -> limit - temp -> baseAddr == total_size) {
                start = temp -> baseAddr;
                pthread_mutex_lock(&mtablasegmentos);
                list_remove_and_destroy_element(segmentosLibres -> elements, index, destroyer);
                pthread_mutex_unlock(&mtablasegmentos);
                return start;
            } else {
                start = temp -> baseAddr;
                temp -> baseAddr += total_size;
                return start;
            }
        }

		element = aux;
        index++;
	}
    return -1;
}

// int memory_best_fit(void *admin, int mem_size, t_dictionary *collection, int total_size) {

//     t_list *temp = list_create();

//     int result;

//     int start = 0;

//     int j;

//     // Contador de bytes libres en la memoria
//     int segment_counter = 0;

//     // Busco espacio libre en la memoria
//     for(int i = 0; i < mem_size; i ++) {
//         if (!memcmp(admin + i, "\0", 1)) {
//             // // printf("Segmento Libre en: %d\n", i);
//             start = i;
//             j = i;
//             while(j < mem_size && !memcmp(admin + j, "\0", 1)) {
//                 // // printf("siguiente en: %d\n", j);
//                 segment_counter ++;
//                 j++;
//             }
//             i = j;
//             // // printf("Data: %d - %d - %d\n", i, segment_counter, mem_size);
//             if (i <= mem_size) {
//                 if (segment_counter >= total_size) {
//                     best_fit_data *data = malloc(sizeof(best_fit_data));
//                     data -> addr = start;
//                     data -> counter = segment_counter;
//                     list_add_sorted(temp, data, segment_cmp);
//                 }

//                 segment_counter = 0;
//             }

//         }

//     }

//     if (list_size(temp) > 0) {
//         best_fit_data *aux = list_get(temp, 0);
//         result = aux -> addr;
//         list_destroy_and_destroy_elements(temp, best_fit_destroyer);
//         // // printf("Direccion a devolver: %d\n", result);
//         return result;
//     }

//     return -1;
// }


// int memory_seek(void *admin, int mem_size, int total_size, t_dictionary *table_collection) {

//     // Contador de bytes libres en la memoria
//     int segment_counter = 0;

//     int start;
//     int j;

//     // Busco espacio libre en la memoria
//     for(int i = 0; i < mem_size; i ++) {
//         if (!memcmp(admin + i, "\0", 1)) {
//             // // printf("Segmento Libre en: %d\n", i);
//             start = i;
//             j = i;
//             while(!memcmp(admin + j, "\0", 1) && j < mem_size) {
//                 // // printf("siguiente en: %d\n", j);
//                 segment_counter ++;
//                 j++;

//                 if (segment_counter >= total_size) {
//                     // // printf("Direccion a devolver: %d\n", start);
//                     return start;
//                 }
//             }
//             i = j;

//         }
//     }
//     return -1;
// }
bool sort_by_addr(void *i1, void *i2) {
  segment *uno = (segment *) i1;
  segment *dos = (segment *) i2;

  return uno -> baseAddr < dos -> baseAddr;
}

int memory_seek(void *admin, int mem_size, int total_size, t_dictionary *table_collection) {

    segment *temp;

    int index = 0;

    int start;

    list_sort(segmentosLibres -> elements, sort_by_addr);

    t_list *self = segmentosLibres -> elements;

    t_link_element *element = self->head;
	t_link_element *aux = NULL;
	while (element != NULL) {
		aux = element->next;
		
        temp = (segment *) element -> data;

        if (temp -> limit - temp -> baseAddr >= total_size) {
            if (temp -> limit - temp -> baseAddr == total_size) {
                start = temp -> baseAddr;
                pthread_mutex_lock(&mtablasegmentos);
                list_remove_and_destroy_element(segmentosLibres -> elements, index, destroyer);
                pthread_mutex_unlock(&mtablasegmentos);
                return start;
            } else {
                start = temp -> baseAddr;
                temp -> baseAddr += total_size;
                return start;
            }
        }
        index ++;
		element = aux;
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

void *get_next_task(void *memory, int start_address, int limit_address, int task_start, t_log* logger) {

    // // printf("Values - Start: %d - End: %d\n", start_address, limit_address);
    if (start_address + task_start >= limit_address) {
        return NULL;
    }
    // log_info(logger, "Limit address:%d", limit_address);

    void *tareas = malloc(limit_address - task_start + 1);
    pthread_mutex_lock(&m_memoria);
    memcpy(tareas, memory + task_start, limit_address - task_start);
    pthread_mutex_unlock(&m_memoria);
    memset(tareas + (limit_address-task_start), '\0', 1);

    int cantidadLetrasLeidas = 0;
    
    // TOMAR AIRE;2,2,T|HCONSUMIR OXIGENO P;3;3;T|HOLA

    // Get one byte of the memory as a CHAR
    // char test_c = get_char_value(tareas, counter);
    int offset = start_address;
    while (cantidadLetrasLeidas + task_start + start_address < limit_address && memcmp(tareas + offset, "|", 1) && tareas + offset != NULL ) {
        // // printf("CHAR: %c\n", get_char_value(tareas, cantidadLetrasLeidas));
        
        if(get_char_value(tareas,offset) != '\n'){
            cantidadLetrasLeidas++;
        }

        offset++;
    }
    // log_info(logger, "Cantidad letras hasta primer ;%d",cantidadLetrasLeidas);

    // while (!isalpha(get_char_value(tareas, cantidadLetrasLeidas)) && cantidadLetrasLeidas + start_address < limit_address){
    //     // // printf("CHAR: %c - %d | %d\n", get_char_value(tareas, cantidadLetrasLeidas), cantidadLetrasLeidas + start_address, limit_address);

    //     cantidadLetrasLeidas++;
    // }
    // log_info(logger, "Cantidad letras despues de segundo while: %d",cantidadLetrasLeidas);

    void *recv_task = malloc(cantidadLetrasLeidas + 1);
    memcpy(recv_task, tareas + start_address, cantidadLetrasLeidas);
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
        pthread_mutex_lock(&m_memoria);
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
        pthread_mutex_unlock(&m_memoria);

        // memset(admin + segmento -> baseAddr, 1, segmento -> limit - segmento -> baseAddr);

        return 1;
    }

    return -1;
}

tcb_t *get_tcb_from_memory(void *memory, int mem_size, segment *segmento) {
    tcb_t *temp;

    int offset = 0;

    if (segmento -> limit < mem_size) {
        temp = malloc(sizeof(tcb_t));
        pthread_mutex_lock(&m_memoria);
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
        pthread_mutex_unlock(&m_memoria);
        offset = sizeof(uint32_t);

        return temp;
    }

    return NULL;
}

int save_pcb_in_memory(void *admin, void *memory, int mem_size, segment *segmento, pcb_t *data) {
    int offset = 0;
    if (segmento -> limit < mem_size) {
        pthread_mutex_lock(&m_memoria);
        memcpy(memory + segmento -> baseAddr + offset, &(data -> pid), sizeof(uint32_t));
        offset = sizeof(uint32_t);
        memcpy(memory + segmento -> baseAddr + offset, &(data -> tasks), sizeof(uint32_t));
        offset = sizeof(uint32_t);
        pthread_mutex_unlock(&m_memoria);

        // memset(admin + segmento -> baseAddr, 1, segmento -> limit - segmento -> baseAddr);

        return 1;
    }

    return -1;
}

pcb_t *get_pcb_from_memory(void *memory, int mem_size, segment *segmento) {
    int offset = 0;

    pcb_t *temp = malloc(sizeof(pcb_t));
    if (segmento -> limit < mem_size) {
        pthread_mutex_lock(&m_memoria);
        memcpy(&(temp -> pid), memory + segmento -> baseAddr + offset, sizeof(uint32_t));
        offset = sizeof(uint32_t);
        memcpy(&(temp -> tasks), memory + segmento -> baseAddr + offset, sizeof(uint32_t));
        pthread_mutex_unlock(&m_memoria);
        offset = sizeof(uint32_t);

        return temp;
    }

    return NULL;
}

// Hay que pasarle la lista completa de tareas, tal cual se guarda en memoria
int save_task_in_memory(void *admin, void *memory, int mem_size, segment *segmento, void *data) {

    if (segmento -> limit < mem_size) {
        pthread_mutex_lock(&m_memoria);
        memcpy(memory + segmento -> baseAddr, data, segmento -> limit - segmento -> baseAddr);
        pthread_mutex_unlock(&m_memoria);
        // memset(admin + segmento -> baseAddr, 1, segmento -> limit - segmento -> baseAddr);
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
    pthread_mutex_lock(&mdictionary);
    t_queue *self = dictionary_get(table_collection, key);
    pthread_mutex_unlock(&mdictionary);

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
    pthread_mutex_lock(&mdictionary);
    t_queue *self = dictionary_get(table_collection, key);
    pthread_mutex_unlock(&mdictionary);

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
    pthread_mutex_lock(&mdictionary);
    t_queue *self = dictionary_get(table_collection, key);
    pthread_mutex_unlock(&mdictionary);

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
                    pthread_mutex_lock(&m_global_process);
                    global_process = string_new();
                    string_append(&global_process, element -> key);
                    pthread_mutex_unlock(&m_global_process);
                    pthread_mutex_lock(&m_global_page);
                    global_page = index;
                    global_time = (page -> frame) -> time;
                    pthread_mutex_unlock(&m_global_page);
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

    txt_write_in_file(file, "Segmentos Libres:\n");

    t_link_element *freeHead = segmentosLibres->elements->head;
	t_link_element *freeTemp = NULL;
	while (freeHead != NULL) {
		freeTemp = freeHead->next;
		segment *freeAux = (segment *) freeHead->data;

        char *line = string_new();
        string_append_with_format(&line, "Proceso: %s\t\tInicio: 0x%d\t\tTam: %db\n", "N/A", freeAux -> baseAddr, freeAux -> limit - freeAux -> baseAddr);
        txt_write_in_file(file, line);
        free(line);

		freeHead = freeTemp;
	}

    txt_write_in_file(file, "--------------------------------------------------------------------------\n");

    txt_close_file(file);
}


// --------------------- END DUMP ----------------------- //

void create_map() {
    nivel_gui_inicializar();

    int pid = process_getpid();

    char *temp = string_itoa(pid);

    // Drawing test
    nivel = nivel_crear(temp);
    nivel_gui_dibujar(nivel);

    free(temp);

}