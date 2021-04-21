#include "thread.h"

void _thread_pool_create(void *( *func )(), pthread_t *_thread_pool, int _thread_pool_size, t_log *logger) {

    for (int i = 0; i < _thread_pool_size; i++) {
        pthread_create(&_thread_pool[i], NULL, func, logger);
    }
}

void _thread_pool_destroy(pthread_t *_thread_pool, int _thread_pool_size) {

    for (int i = 0; i < _thread_pool_size; i++) {
        pthread_join(_thread_pool[i], NULL);
    }
}

