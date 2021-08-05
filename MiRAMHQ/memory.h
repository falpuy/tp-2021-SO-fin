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
    #include <commons/config.h>
    #include <nivel-gui/nivel-gui.h>
    #include <nivel-gui/tad_nivel.h>

    pthread_mutex_t m_memoria;
    pthread_mutex_t m_virtual;
    pthread_mutex_t mbitmap;
    pthread_mutex_t mbitmapv;
    pthread_mutex_t mtablasegmentos;
    pthread_mutex_t mtablapaginas;
    pthread_mutex_t mdictionary;
    pthread_mutex_t madmin;

    pthread_mutex_t m_map;
    
    pthread_mutex_t m_global_page;
    pthread_mutex_t m_global_process;
    pthread_mutex_t m_global_lru_page;
    pthread_mutex_t m_global_clock_page;
    pthread_mutex_t m_global_clock_key;
    pthread_mutex_t m_global_clock_index;
    pthread_mutex_t m_global_index;
    pthread_mutex_t m_global_segment;
    pthread_mutex_t m_global_type;

    // pthread_mutex_t madmin;
    // pthread_mutex_t madmin;

    typedef struct {
        uint32_t pid;
        uint32_t tasks;
    } pcb_t;

    NIVEL* nivel;

    t_queue *segmentosLibres;

    typedef struct {
        uint32_t tid;
        uint32_t pid;
        char status;
        uint32_t xpos;
        uint32_t ypos;
        uint32_t next;
    } tcb_t;

    int timer;

    t_dictionary *table_collection;
    t_dictionary *admin_collection;

    typedef struct {
        uint32_t start;
        uint32_t page_number;
        uint32_t cantidad;
        uint8_t *tcb;
    } admin_data;

    char *esquema;
    int isBestFit;

    t_log *logger;
    t_log *loggerMap;
    t_config * config;

    void *admin;

    void *memory;
    int mem_size;
    int page_size;

    void *virtual_memory;
    int virtual_size;
    t_bitarray *virtual_bitmap;
    int frames_virtual;

    // Asigna espacio a memoria y setea el global 'mem_size' con el tamanio de memoria asignado
    void *memory_init(int size);

    void create_map();

    // Libera la memoria y los segmentos asociados
    // void memory_destroy(void *memory, t_list *segmentTable);

    // --------------------- PAGINATION ----------------------- //

    typedef struct {
        uint32_t number;
        uint32_t start; //se podria sacar
        uint8_t modified;
        uint8_t presence;
        int time;
    } frame_t;

    typedef struct {
        int number;
        frame_t *frame;
    } page_t;

    int global_page;
    char* global_process;

    uint8_t *bitmap;
    int frames_memory;

    page_t *global_lru_page;
    page_t *global_clock_page;

    int clock_flag;
    int global_clock_key;
    int global_clock_index;

    int hasLRU;

    uint32_t global_index;
    uint32_t global_segment;
    uint32_t global_type;

    char get_char_value(void *buffer, int index);

    uint32_t get_next_page(t_queue *page_table, uint32_t index);

    void *search_task(void *memory, uint32_t start_addr);

    int check_free_frames(int frames_count);

    bool lru_sorter(void *uno, void *dos);

    bool sort_by_addr(void *i1, void *i2);

    void lru_replacer(void *item);

    void lru_iterator(char *key, void *item);

    frame_t *get_next_lru_frame();

    void clock_replacer(void *item, int key_index, int frame_index);

    void clock_iterator(char *key, void *item, int key_index, int frame_index);

    frame_t *get_next_clock_frame();

    uint32_t get_frame();

    void set_bitmap(uint8_t *bitmap, int position);

    void unset_bitmap(uint8_t *bitmap, int position);

    int save_data_in_memory(void *memory, t_dictionary *table_collection, t_dictionary* admin_collection, void *buffer);

    int verificarCondicionDeMemoria(int frame_count);

    void admin_destroyer(void *item);

    void page_destroyer(void *item);

    void page_table_destroyer(void *item);

    int get_page_number(t_dictionary *self, uint32_t frame);

    uint8_t frame_is_empty(void *temp, uint32_t start, uint32_t limit);  

    int remove_tcb_from_page(void *memory, t_dictionary *admin_collection, t_dictionary *table_collection, char *key, int id_tcb);

    void update_position_from_page(void *memory, t_dictionary *admin_collection, t_dictionary *table_collection, char *key, int id_tcb, int posx, int posy);

    void update_status_from_page(void *memory, t_dictionary *admin_collection, t_dictionary *table_collection, char *key, int id_tcb, char status);

    char *get_task_from_page(void *memory, t_dictionary *admin_collection, t_dictionary *table_collection, char *key, int id_tcb);

    void remove_pcb_from_page(void *memory, t_dictionary *admin_collection, t_dictionary *table_collection, char *key);


    // --------------------- END PAGINATION ----------------------- //

    // --------------------- SEGMENTATION ----------------------- //

    enum tipo_segmento {
        PCB,
        TCB,
        TASK
    };

    int global_time;

    typedef struct {
        uint32_t id;
        uint32_t type;
        uint32_t nroSegmento;
        uint32_t baseAddr;
        uint32_t limit;
    } segment;

    // int tasks_size;

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

    int mem_space(void *admin, int mem_size, int total_size, t_dictionary *table_collection);

    int memory_best_fit(void *admin, int mem_size, t_dictionary *collection, int total_size);

    int memory_seek(void *admin, int mem_size, int total_size, t_dictionary *table_collection);

    int get_last_index (t_queue *segmentTable);

    void *find_tcb_segment(int id, char *key, t_dictionary *table);

    void *find_task_segment(char *key, t_dictionary *table);

    void *find_pcb_segment(char *key, t_dictionary *table);

    char get_char_value(void *buffer, int index);

    void *get_next_task(void *memory, int start_address, int limit_address, int task_start, t_log* logger);

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
    
    void page_dump(t_dictionary *table);
    void memory_dump(t_dictionary *self, void *memory);

#endif