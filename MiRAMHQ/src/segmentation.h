#ifndef _SEGMENT_H_
#define _SEGMENT_H_

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
#include<ctype.h>

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

uint32_t global_index;
uint32_t global_segment;
uint32_t global_type;

#endif