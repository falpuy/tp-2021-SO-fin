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


void h_newReady(){
    while(1){
        sem_wait(&s1);
        printf("Se ejecuta hilo new->ready");
        sem_post(&s2);
    }
}

void liberarHilos(void *nodo) {
    tcb *tripulante;
    sem_post(semaforoTripulantes[tripulante -> tid]);
}

void h_readyExec(){
    while(1){
        sem_wait(&s2);
        printf("Se ejecuta hilo ready->exec");
        // nodo* tcb;

        // // Devuelvo el primero de exec pero no lo elimino de la lista
        // tcb * nodo = list_get(cola_exec -> elements, 0);
        // if (nodo) {
        //     sem_post(semaforoTripulantes[nodo -> tid]);
        // } else {
        //     sem_post(&s3);
        // }
        list_iterate(cola_exec -> elements, liberarHilos);
    }   
}

void * get_by_id(t_list * lista, int id) {
    tcb *nodo;

    t_link_element *element = self->head;
	t_link_element *aux = NULL;
	while (element != NULL) {
		aux = element->next;
		tcb *nodo = (tcb *) (element->data);
        if (nodo -> tid == id) {
            return nodo;
        }
		element = aux;
	}
    return NULL;
}

void _signal(int value, int maxValue, sem_t semaforo) {
    semaforoGlobal += value;
    if (semaforoGlobal == maxValue) {
        sem_post(&semaforo);
        semaforoGlobal = 0;
    }
}

void h_tripulante(void* item){
    parametrosThread* aux = (parametrosThread*) item;

    while(1){
        sem_wait(semaforoTripulantes[aux->idSemaforo]);

        lock(exec);
        tcb *nodo = get_by_id(cola_exec -> elements, aux->idSemaforo);
        unlock(exec);


        printf("Se ejecuta hilo tripulante %d", nodo -> tid);

        _signal(1, gradoMultiprocesamiento, s3);
    }
}

void h_blocked(){
    while(1){
        sem_wait(&s3);
        printf("Se ejecuta hilo blocked");
        sem_post(&s4);

    }
}


void h_exit(){
    while(1){
        sem_wait(&s4);
        printf("Se ejecuta hilo exit");
        sem_post(&s1);
    }
}

sem_t s1,s2,s3,s4;

int cantidadActual;
int cantidadVieja;
sem_t* semaforoTripulantes;
int cantidadTCBTotales = 0; // --> se va sumando a medida que se crean TCB

typedef struct{
    int tid; 
}tcb;

typedef struct{
    t_log* logger;
    int idSemaforo;
}parametrosThread;

t_queue* tripulantes;

int main(){
    pthread_t newReady;
    pthread_t readyExec;
    pthread_t blocked;
    pthread_t exit;

    tripulantes = queue_create();

    cantidadTCBTotales = 0;
    //tcb->tid = cantidadTCBTotales;-->agregar esto en cada struct tcb creado
    cantidadVieja = 0;

    sem_init(&s1, 0, 1);
    sem_init(&s2, 0, 0);
    sem_init(&s3, 0, 0);
    sem_init(&s4, 0, 0);

    t_log* logger = log_create("test.log","TEST",1,LOG_LEVEL_INFO);

    pthread_create(&newReady,NULL,(void*) h_newReady, logger);
    pthread_create(&readyExec,NULL,(void*) h_readyExec, logger);
    pthread_create(&blocked,NULL,(void*) h_blocked, logger);
    pthread_create(&exit,NULL,(void*) h_exit, logger);

    cantidadActual = 3;
    semaforoTripulantes = malloc(sizeof(sem_t)*cantidadActual);

    for(int i = cantidadVieja ; i < cantidadActual; i++){
        sem_init(semaforoTripulantes[i],0,0);
    }

    for(int i = cantidadVieja ; i < cantidadActual; i++){

        parametrosThread *parametros = malloc(sizeof(parametrosThread));
        parametros -> logger = logger;
        parametros -> idSemaforo = i;

        pthread_t tripulante;
        pthread_create(&tripulante,NULL,(void*) h_tripulante, parametros);
        pthread_detach(h_tripulante);
    }

    return 0;
}