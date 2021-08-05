#include"headers/sabotaje.h"

void handler(int client, char* identificador, int comando, void* payload, t_log* logger){
    int hayTripulantesEnLaNave;

    switch (comando) {
        case COMIENZA_SABOTAJE:

        hayTripulantesEnLaNave = 1;

        if(hayTripulantesEnLaNave){
            pthread_mutex_lock(&mutexSabotajeActivado);
            sabotaje_activado=1;
            pthread_mutex_unlock(&mutexSabotajeActivado);
            sabotaje_terminado=0;

            pthread_mutex_lock(&mutexCiclosTranscurridosSabotaje);
            ciclos_transcurridos_sabotaje = 0;
            pthread_mutex_unlock(&mutexCiclosTranscurridosSabotaje);

            memcpy(&posSabotajeX, payload, sizeof(int));
            memcpy(&posSabotajeY, payload + sizeof(int), sizeof(int));

            log_info(logger, "Llego comando COMIENZA_SABOTAJE con posición en %d-%d",posSabotajeX,posSabotajeY);
        }

        else{
            log_error(logger, "No se puede resolver el sabotaje porque no hay ningún tripulante en la nave");
        }

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
	
	if(modulo1 == modulo2)
		return tcb1->tid < tcb2->tid;
	else
		return modulo1 < modulo2;
}
