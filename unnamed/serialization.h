#ifndef UNNAMED_SERIALIZATION_LIB
#define UNNAMED_SERIALIZATION_LIB

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <commons/string.h>

enum {
    STRING = 0,
    INT,
    CHAR,
    DOUBLE,
    UINT32,
};

/**
 * METHOD: _serialize
 * 
 * Permite serializar cualquier struct o valor dentro de un buffer
 * 
 * @params: 
 *      size -> tamanio total del buffer a enviar
 *          IMPORTANT:
 *              for type Int    ->  Use sizeof(int)
 *              for type String ->  Use strlen(your_string) + sizeof(int)
 *
 *      format -> Permite especificar que tipo de variables queremos agregar al buffer [notacion: %d, %s, etc]
 *      
 *      ... -> variables correspondientes al formato especificado
 *
 * @usage:
 *      Call with _serialize(sizeof(int), "%d", 25);
 * 
 * @format:
 * 
 *     "s" -> STRING
 *     "d" -> INT
 *     "c" -> CHAR
 *     "f" -> DOUBLE
 *     "u" -> UINT32
 * 
 */
void *_serialize(int size, char *format, ...);

int get_type(char *type);

#endif