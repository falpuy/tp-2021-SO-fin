#ifndef _MEM_H_
#define _MEM_H_

#include <stdio.h>
#include <stdint.h>

void *memory;
void *virtualMemory;

typedef struct {
    int pid;
    char **segmentTable; // Base Adress | Limit
} p_info;

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

#endif