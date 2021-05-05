#include <stdio.h>
#include <stdlib.h>
#include <commons/log.h>
#include <unnamed/server.h>
#include <signal.h>

t_log *logger;
int _pool_size;

pthread_t *_threads;

void liberar_memoria(int num) {
  validator = 0;

    log_info(logger, "Waiting for thread to end..");
    _thread_pool_destroy(_threads, _pool_size);

  for (int i = 0; i < _pool_size; i ++) {
      pthread_detach(_threads[i]);
  }
  free(_threads);

log_destroy(logger);
  queue_destroy(_socket_queue);

  exit(EXIT_FAILURE);
}

int main() {

    logger = log_create("../logs/test.log", "TEST", 1, LOG_LEVEL_TRACE);

    signal(SIGINT, liberar_memoria);

    _pool_size = 2;

    _threads = malloc(_pool_size * sizeof(pthread_t));
    
    _socket_queue = queue_create();

    log_info(logger,"Trying to start the thread pool...");
    
    _thread_pool_create(_thread_function, _threads, _pool_size, logger);

    log_info(logger,"Trying to start the server...");

    _start_server("9000", logger);

    return 0;
}