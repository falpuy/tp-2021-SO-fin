#ifndef _MEMORY_H_
#define _MEMORY_H_

    #include <unnamed/serialization.h>
    #include <commons/log.h>
    #include <commons/collections/list.h>
    #include <commons/collections/queue.h>
    #include <commons/collections/dictionary.h>
    #include <commons/string.h>
    #include <commons/error.h>
    #include <commons/memory.h>
    #include <commons/process.h>
    #include <commons/txt.h>
    #include <commons/temporal.h>
    #include <ctype.h>
    #include <pthread.h>
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
    #include <unnamed/socket.h>

    typedef struct {
        uint32_t pid;
        uint32_t tasks;
    } pcb_t;

    typedef struct {
        uint32_t tid;
        uint32_t pid;
        char status;
        uint32_t xpos;
        uint32_t ypos;
        uint32_t next;
    } tcb_t;

    // Asigna espacio a memoria y setea el global 'mem_size' con el tamanio de memoria asignado
    void *memory_init(int size);

    // Libera la memoria y los segmentos asociados
    // void memory_destroy(void *memory, t_list *segmentTable);

    // --------------------- PAGINATION ----------------------- //

    typedef struct {
        uint32_t number;
        uint32_t start; //se podria sacar
        uint8_t modified;
        uint8_t presence;
    } frame_t;

    typedef struct {
        frame_t *frame;
    } page_t;

    // --------------------- END PAGINATION ----------------------- //

    // --------------------- SEGMENTATION ----------------------- //

    enum tipo_segmento {
        PCB,
        TCB,
        TASK
    };

    typedef struct {
        uint32_t id;
        uint32_t type;
        uint32_t nroSegmento;
        uint32_t baseAddr;
        uint32_t limit;
    } segment;

    // int tasks_size;
    // t_queue *segmentTable;


    // void mostrarInfo (void *element);

    char *get_segment_type(uint32_t segment_type);

    void mostrarSegemento (void *element);

    void show_dictionary(t_dictionary *self);

    // void *find_info_by_id(t_list* self, int id);

    void *find_segment_by_number(t_list* self, int index);

    int get_segment_limit(t_dictionary* self, int start);

    void destroyer(void *item);

    void table_destroyer(void *item);

    void memory_compaction(void *admin, void *memory, int mem_size, t_dictionary* self);

    int check_space_memory(void *admin, int mem_size, int total_size, t_dictionary *table_collection);

    int memory_best_fit(void *admin, int mem_size, t_dictionary *collection, int total_size);

    int memory_seek(void *admin, int mem_size, int total_size, t_dictionary *table_collection);

    int get_last_index (t_queue *segmentTable);

    void *find_tcb_segment(int id, char *key, t_dictionary *table);

    void *find_task_segment(char *key, t_dictionary *table);

    void *find_pcb_segment(char *key, t_dictionary *table);

    char get_char_value(void *buffer, int index);

    void *get_next_task(void *memory, int start_address, int limit_address);

    int remove_segment_from_memory(void *admin, int mem_size, segment *segmento);

    void remove_segment_from_table(t_dictionary* table_collection, char *key, segment *segmento);

    // void remove_pcb_from_memory(void *admin, int mem_size, t_dictionary *table_collection, char* key);

    int save_tcb_in_memory(void *admin, void *memory, int mem_size, segment *segmento, tcb_t *data);

    tcb_t *get_tcb_from_memory(void *memory, int mem_size, segment *segmento);

    int save_pcb_in_memory(void *admin, void *memory, int mem_size, segment *segmento, pcb_t *data);

    pcb_t *get_pcb_from_memory(void *memory, int mem_size, segment *segmento);

    // Hay que pasarle la lista completa de tareas, tal cual se guarda en memoria
    int save_task_in_memory(void *admin, void *memory, int mem_size, segment *segmento, void *data);

    segment *get_next_segment(t_dictionary *table_collection, char* key);

    // --------------------- END SEGMENTATION ----------------------- //

    // ------------------------- DUMP ------------------------------- //
    
    void memory_dump(t_dictionary *self, void *memory);

#endif