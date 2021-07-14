#include"headers/sabotaje.h"

void handler(int client, char* identificador, int comando, void* payload, t_log* logger){
    char* buffer;
    switch (comando) {
        case COMIENZA_SABOTAJE:
            sabotaje_activado=1;
            ciclos_transcurridos_sabotaje = 0;
            memcpy(&posSabotajeX, payload, sizeof(int));
            memcpy(&posSabotajeY, payload + sizeof(int), sizeof(int));
            tripulanteFixer = malloc(sizeof(tcb));
            sem_post(&semERM);
        break;
    }
}

bool comparadorTid(void* tripulante1, void* tripulante2){
    tcb* tcb1 = (tcb *) tripulante1;
    tcb* tcb2 = (tcb *) tripulante2;
    return tcb1->tid < tcb2->tid;
}

void funcionhExecReadyaBloqEmer (t_log* logger){
    pthread_mutex_lock(&mutexValidador);
  	while(validador == 1){
        pthread_mutex_unlock(&mutexValidador);
        while (planificacion_viva && sabotaje_activado == 1) {
            sem_wait(&semERM);
            if(ciclos_transcurridos_sabotaje == duracion_sabotaje){
                pthread_mutex_lock(&mutexExec);
                tcb* aux_Fixer_Fin = queue_pop(exec);
                pthread_mutex_unlock(&mutexExec);
                aux_Fixer_Fin->status = 'M';
                aux_Fixer_Fin->ciclosCumplidos = ciclos_cumplidos_fixer_pre_sabotaje;
                pthread_mutex_lock(&mutexBloqEmer);
                queue_push(bloq_emer,(void*) aux_Fixer_Fin);
                pthread_mutex_unlock(&mutexBloqEmer);
                sem_post(&semMR);
            }
            else{
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
                list_add_sorted(bloq_emer_sorted->elements,(void*) aux_TCB,ordenarMasCercano);
                pthread_mutex_unlock(&mutexBloqEmer);
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
                pthread_mutex_lock(&mutexBloqEmerSorted);
                list_add_sorted(bloq_emer_sorted->elements,(void*) aux_TCB,ordenarMasCercano);
                pthread_mutex_unlock(&mutexBloqEmerSorted);
            }

            tripulanteFixer = queue_pop(bloq_emer_sorted);
            if(tripulanteFixer->ciclosCumplidos != 0){ 
                ciclos_cumplidos_fixer_pre_sabotaje = tripulanteFixer->ciclosCumplidos;
                tripulanteFixer->ciclosCumplidos = 0;
            }

            list_remove(bloq_emer->elements,tripulanteFixer->tid);
            pthread_mutex_lock(&mutexBloqEmer);
            queue_push(bloq_emer,(void*) tripulanteFixer);
            pthread_mutex_lock(&mutexBloqEmer);

            while (!queue_is_empty(bloq_emer_sorted))  
                queue_pop(bloq_emer_sorted);
            
            int tamanioBuffer = sizeof(int);
            void* buffer = malloc(tamanioBuffer);
			int idTripulante = tripulanteFixer->tid;
            buffer = _serialize(tamanioBuffer, "%d", idTripulante);
            _send_message(conexion_RAM, "DIS", ATIENDE_SABOTAJE, buffer, tamanioBuffer, logger);
            free(buffer);

            sem_post(&semFMR);
            }

        }
    }
}

bool ordenarMasCercano(void* tripulante1, void* tripulante2){
    tcb* tcb1 = (tcb*) tripulante1;
    tcb* tcb2 = (tcb*) tripulante2;

    double diferenciatcb1X = (double)posSabotajeX - (double)tcb1->posicionX;
    double diferenciatcb1Y = (double)posSabotajeY - (double)tcb1->posicionY;
    double diferenciatcb2X = (double)posSabotajeX - (double)tcb2->posicionX;
    double diferenciatcb2Y = (double)posSabotajeY - (double)tcb2->posicionY;

    double modulo1 = sqrt(pow(diferenciatcb1X, 2) + pow(diferenciatcb1Y, 2));
    double modulo2 = sqrt(pow(diferenciatcb2X, 2) + pow(diferenciatcb2Y, 2));

    return modulo1 < modulo2;
}

void funcionhFixerdeEmeraReady(t_log* logger){
    pthread_mutex_lock(&mutexValidador);
    while(validador == 1){
        pthread_mutex_unlock(&mutexValidador);
        while (planificacion_viva && sabotaje_activado == 1) {
            sem_wait(&semFMR);
            tcb* aux_Fixer = malloc(sizeof(tcb));
            pthread_mutex_lock(&mutexBloqEmer);
            aux_Fixer = list_remove(bloq_emer->elements,tripulanteFixer->tid);
            pthread_mutex_unlock(&mutexBloqEmer);
            aux_Fixer->status = 'R';
            pthread_mutex_lock(&mutexReady);
            queue_push(ready,(void*) aux_Fixer);
            pthread_mutex_unlock(&mutexReady);

            sem_post(&semRE);
        }
    }
}

void funcionhBloqEmeraReady (t_log* logger){
    pthread_mutex_lock(&mutexValidador);
  	while(validador == 1){
        pthread_mutex_unlock(&mutexValidador);
        while (planificacion_viva && sabotaje_activado == 1) {
            sem_wait(&semMR);
            while (!queue_is_empty(bloq_emer))
            {
                tcb* aux_TCB = malloc (sizeof(tcb));
                pthread_mutex_lock(&mutexBloqEmer);
                aux_TCB = queue_pop(bloq_emer);
                pthread_mutex_unlock(&mutexBloqEmer);
                aux_TCB->status = 'R';
                pthread_mutex_lock(&mutexReady);
                queue_push(ready, (void*) aux_TCB);
                pthread_mutex_unlock(&mutexReady);
            }
            sabotaje_activado = 0;
            char* bufferAEnviar = string_new();
            string_append(&bufferAEnviar, "Se resolvio el sabotaje");
            _send_message(conexion_IMS, "DIS", RESOLUCION_SABOTAJE, bufferAEnviar, strlen(bufferAEnviar), logger);
            free(bufferAEnviar);
            sem_post(&semNR);
        }
    }
}