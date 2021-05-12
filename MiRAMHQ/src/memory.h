#ifndef _MEMORY_H_
#define _MEMORY_H_

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <commons/collections/list.h>

enum tipo_segmento {
    PCB,
    TCB,
    TASK
};

// type -> tcb | pcb | task
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

// Asigna espacio a memoria
void *memory_init(int size);

// Libera la memoria y los segmentos asociados
void memory_destroy(void *memory, t_list *segmentTable);

// Guarda un struct en la memoria | type -> PCB | TCB | TASK
void save_element(void *memory, int type, void *object);

// Devuele un buffer con los datos del struct SIN deserializar
void* get_element(void *memory, segment *address);

pcb *deserialize_pcb(void *buffer, int *tasks_size);

#endif