#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unnamed/socket.h>
#include <unnamed/serialization.h>
#include <commons/log.h>
#include <string.h>
#include <unistd.h>
#include <commons/string.h>
#include <stdint.h>
#include <commons/bitarray.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdarg.h>
#include <sys/types.h>
#include <dirent.h>
#include <commons/process.h>
#include <pthread.h>
#include <semaphore.h>
#include <commons/collections/queue.h>
#include <commons/config.h>

typedef struct{
    char* nombre;
    int edad;
    t_log* logger;
}pibe;

void funcionMostrarAlgo(pibe* human){
    while(1){
        log_info(human->logger,"\nNombre:%s, Edad:%d\n",human->nombre,human->edad);

    }
}


int main(){
    
    t_log* logger2 = log_create("a","a",1,LOG_LEVEL_INFO);
    pthread_t testHilo;
    pibe* human = malloc(sizeof(pibe));
    human->nombre = string_new();
    string_append(&human->nombre,"Guille");
    human->edad=23;
    human->logger = logger2;
    pthread_create(&testHilo,NULL,(void*) funcionMostrarAlgo,human);

    int contador = 0;
    while(1){
        contador++;
    }

    return 0;
}

