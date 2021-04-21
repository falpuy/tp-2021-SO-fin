#ifndef UNNAMED_THREAD_LIB
#define UNNAMED_THREAD_LIB

// STANDARD LIBRARIES

#include <pthread.h>
#include <commons/log.h>

// FUNCTIONS

/**
 * METHOD: _thread_pool_create
 * 
 * Permite crear una thread pool del tamaño especificado en THREAD_POOL_SIZE
 * 
 * @params: 
 *      func -> Funcion custom a ejecutar en cada iteracion [use func()]
 *      _thread_pool -> Array de worker threads
 *      _thread_pool_size -> tamaño de la threadpool (tamaño del array de threads)
 *
 * @example:
 *      Call with _thread_pool_create(myFunction, threads, pool_size);
 */
void _thread_pool_create(void *( *func )(), pthread_t *_thread_pool, int _thread_pool_size, t_log *logger);

/**
 * METHOD: _thread_pool_destroy
 * 
 * Permite eliminar la thread pool una vez hayan terminado de trabajar los threads (Thread Join)
 *
 * @params: 
 *      _thread_pool -> Array de worker threads
 *      _thread_pool_size -> tamaño de la threadpool (tamaño del array de threads)
 *
 *
 * @example:
 *      Call with _thread_pool_destroy(threads, pool_size);
 */
void _thread_pool_destroy(pthread_t *_thread_pool, int _thread_pool_size);

#endif