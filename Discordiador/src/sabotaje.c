#include"headers/sabotaje.h"

/*void servidor(parametrosServer* parametros){
    while(validador){
        _start_server(parametros->puertoDiscordiador, handler, parametros->loggerDiscordiador);
    }
}*/

void handler(int client, char* identificador, int comando, void* payload, t_log* logger){
    char* buffer;
    switch (comando) {
        case 771: //SABOTAJE
            sabotaje_activado=1;
            ciclos_transcurridos_sabotaje = 0;
            memcpy(&posSabotajeX, payload, sizeof(int));
            memcpy(&posSabotajeY, payload + sizeof(int), sizeof(int));
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
  	while(validador == 1){
        while (planificacion_viva && sabotaje_activado == 1) {
            sem_wait(&semERM);
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
                list_add_sorted(bloq_emer_sorted->elements,(void*) aux_TCB,ordenarMasCercano);
                pthread_mutex_unlock(&mutexBloqEmer);
            }
            tcb *tripulanteFixer = malloc(sizeof(tcb));
            if(tripulanteFixer!=NULL){/*Como ya esta asignado el tripulante, no hace nada en esta parte*/}
            else{
            tripulanteFixer = queue_pop(bloq_emer_sorted);
            list_remove(bloq_emer->elements,tripulanteFixer->tid); //--->O tid+1??
            queue_push(bloq_emer,(void*) tripulanteFixer);

            while (!queue_is_empty(bloq_emer_sorted))  
                queue_pop(bloq_emer_sorted);
            }
            sem_post(&semTripulantes[tripulanteFixer->tid]);
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

void funcionhAtenderSabotaje (t_log* logger){
    while(validador == 1){
        while (planificacion_viva && sabotaje_activado == 1) {
            sem_wait(&semTripulantes[queue_size(bloq_emer)]);
            tcb *tripulanteFixer = malloc(sizeof(tcb));
            tripulanteFixer = list_get(bloq_emer->elements,queue_size(bloq_emer));

            /*if (!llegoAPosicion((tripulanteFixer->posicionX,tripulanteFixer->posicionY,posSabotajeX,posSabotajeY))){
                moverTripulanteUno(tripulanteFixer,posSabotajeX,posSabotajeY);
                sem_post(&semERM);
            }    
            else if(ciclos_transcurridos_sabotaje<duracion_sabotaje){
                //fsck(); --> Como un send o una funcion cualquiera??
                ciclos_transcurridos_sabotaje++;
                sem_post(&semERM);
            }
            else
                sem_post(&semMR);*/

        }
    }
}


void funcionhBloqEmeraReady (t_log* logger){
  	while(validador == 1){
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
            sem_post(&semNR);
        }
    }
}

/*COMIENZA_SABOTAJE 768
ATIENDE_SABOTAJE 769
INVOCAR_FSCK 771
RESOLUCION_SABOTAJE 772
ESPERANDO_SABOTAJE 767 (aca es donde le mandamos el Socket a IMS, antes que nada)

"Una vez elegido el tripulante, ahi enviamos el ATIENDE_SABOTAJE"
================================EN FUNCION TRIPULANTE(exec)
if(sabotaje_activado == 1){
sem_wait(&semTripulante[t->tid]);
if(!llegoAPosicion(t->posicionX,t->posicionY,posSabotajeX,posSabotajeY){
	moverTripulanteUno(tripulante);
	sem_post(&semERM);
	}
else if(llegoAPosicion(t->posicionX,t->posicionY,posSabotajeX,posSabotajeY){
	_send_message(IMS,"DIS",INVOCAR_FSCK);
	ciclos_transcurridos++;
	sem_post(&semERM);
	}
else if(ciclos_transcurridos<duracion_sabotaje){
	ciclos_transcurridos++;
	sem_post(&semERM);
	}
else{
	_send_message(IMS,"DIS",RESOLUCION_SABOTAJE);
	}
}
================================
Es NECESARIO hacer una transicion primero a ready antes de ir para exec durante el sabotaje, entonces a donde va el tripulante.

HAY QUE TENER EN CUENTA RR, si el cuantum llega al maximo, hay que mandarlo de nuevo a ready.

Necesitariamos asegurar que no puedan haber nuevos tripulantes hasta que el sabotaje no haya finalizado, ya sea por un mensaje o guardandolos en un auxiliar hasta que termine.

#En conclusion, exec y ready solo pueden tener un miembro durante el sabotaje, y ese es el fixer. Una opcion seria que si ready recibe un nuevo miembro, si este difiere en su tid con el fixer, entonces se lo mueve a bloq_emer, de lo contrario se lo manda para exec.*/