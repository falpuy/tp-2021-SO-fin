#ifndef _MEMORY_H_
#define _MEMORY_H_

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <commons/collections/list.h>

void *memory;
void *virtualMemory;

typedef struct {
    uint32_t id;
    uint32_t nroSegmento;
} p_info;

t_list *segmentTable;

typedef struct {
    uint32_t baseAddr;
    uint32_t limit;
} segment;

typedef struct {
    uint32_t pid;
    void *tasks;
} pcb;

typedef struct {
    uint32_t tid;
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
void save_element(int type, void *object);

// Devuele un buffer con los datos del struct SIN deserializar
void* get_element(void *memory, segment *address);

#endif