#include"headers/sabotaje.h"

void servidor(parametrosServer* parametros){
    while(validador){
        _start_server(parametros->puertoDiscordiador, handler, parametros->loggerDiscordiador);
    }
}

void handler(int client, char* identificador, int comando, void* payload, t_log* logger){
    char* buffer;
    switch (comando) {
        case 771: //SABOTAJE
            sabotaje_activado=1;
            memcpy(&posSabotajeX, payload, sizeof(int));
            memcpy(&posSabotajeY, payload + sizeof(int), sizeof(int));
        break;
    }
}

bool comparadorTid(void* tripulante1, void* tripulante2){
    tcb* tcb1 = (tcb *) tripulante1;
    tcb* tcb2 = (tcb *) tripulante2;
    return tcb1->tid < tcb2->tid;
}

void funcionhExecReadyaBloqEmer (t_log* logger){
  	while(validador == 1){
        while (planificacion_pausada == 0 && sabotaje_activado == 1) {
            list_sort(exec->elements, comparadorTid);
            while (!queue_is_empty(exec))
            {
                tcb* aux_TCB = malloc (sizeof(tcb));
                pthread_mutex_lock(&mutexExec);
                aux_TCB = queue_pop(exec);
                pthread_mutex_unlock(&mutexExec);
                aux_TCB->status = 'M';
                pthread_mutex_lock(&mutexBloqEmer);
                queue_push(bloq_emer, (void*) aux_TCB);
                pthread_mutex_unlock(&mutexBloqEmer);
                free(aux_TCB);
            }
            list_sort(ready->elements, comparadorTid);
            while (!queue_is_empty(ready))
            {
                tcb* aux_TCB = malloc (sizeof(tcb));
                pthread_mutex_lock(&mutexReady);
                aux_TCB = queue_pop(ready);
                pthread_mutex_unlock(&mutexReady);
                aux_TCB->status = 'M';
                pthread_mutex_lock(&mutexBloqEmer);
                queue_push(bloq_emer, (void*) aux_TCB);
                pthread_mutex_unlock(&mutexBloqEmer);
                free(aux_TCB);
            }
        }
    }
}

bool ordenarMasCercano(void* tripulante1, void* tripulante2){ //Tomamos como vectores
    tcb* tcb1 = (tcb*) tripulante1;
    tcb* tcb2 = (tcb*) tripulante2;

    int diferenciatcb1X = posSabotajeX - tcb1->posX;
    int diferenciatcb1Y = posSabotajeY - tcb1->posY;
    int diferenciatcb2X = posSabotajeX - tcb2->posX;
    int diferenciatcb2Y = posSabotajeY - tcb2->posX;

    int modulo1 = sqrt(pow(diferenciatcb1X, 2) + pow(diferenciatcb1Y, 2));
    int modulo2 = sqrt(pow(diferenciatcb2X, 2) + pow(diferenciatcb2Y, 2));

    return modulo1 < modulo2;
}

bool compDistancias(void* tripulante1, void* tripulante2){
    tcb* tcb1 = (tcb *) tripulante1;
    tcb* tcb2 = (tcb *) tripulante2;
    return tcb1->tid < tcb2->tid;
}

void funcionhBloqEmeraReady (t_log* logger){
  	while(validador == 1){
        while (planificacion_pausada == 0 && sabotaje_activado == 1) {
            while (!queue_is_empty(bloq_emer))
            {
                tcb* aux_TCB = malloc (sizeof(tcb));
                pthread_mutex_lock(&mutexBloqEmer);
                queue_pop(bloq_emer);
                pthread_mutex_unlock(&mutexBloqEmer);
                aux_TCB->status = 'R';
                pthread_mutex_lock(&mutexReady);
                queue_push(ready, (void*) aux_TCB);
                pthread_mutex_unlock(&mutexReady);
                free(aux_TCB);
            }
        }
    }
}
