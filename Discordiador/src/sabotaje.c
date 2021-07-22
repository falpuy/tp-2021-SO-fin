#include"headers/sabotaje.h"

void handler(int client, char* identificador, int comando, void* payload, t_log* logger){
    switch (comando) {
        case COMIENZA_SABOTAJE:
            pthread_mutex_lock(&mutexSabotajeActivado);
            sabotaje_activado=1;
            pthread_mutex_unlock(&mutexSabotajeActivado);

            pthread_mutex_lock(&mutexCiclosTranscurridosSabotaje);
            ciclos_transcurridos_sabotaje = 0;
            pthread_mutex_unlock(&mutexCiclosTranscurridosSabotaje);

            memcpy(&posSabotajeX, payload, sizeof(int));
            memcpy(&posSabotajeY, payload + sizeof(int), sizeof(int));
            log_info(logger, "Llego comando COMIENZA_SABOTAJE con posición en %d-%d",posSabotajeX,posSabotajeY);

            sem_post(&semERM);
        break;
    }
}

bool comparadorTid(void* tripulante1, void* tripulante2){
    tcb* tcb1 = (tcb *) tripulante1;
    tcb* tcb2 = (tcb *) tripulante2;
    return tcb1->tid < tcb2->tid;
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