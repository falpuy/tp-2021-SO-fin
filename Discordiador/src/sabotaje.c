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

void funcionhExecReadyaBloqEmer (t_log* logger) {
    
    pthread_mutex_lock(&mutexValidador);
    int temp_validador = validador;
    pthread_mutex_unlock(&mutexValidador);

  	while(temp_validador) {

        pthread_mutex_lock(&mutexPlanificacionViva);
        int temp_planificacion_viva = planificacion_viva;
        pthread_mutex_unlock(&mutexPlanificacionViva);

        pthread_mutex_lock(&mutexSabotajeActivado);
        int temp_sabotaje_activado = sabotaje_activado;
        pthread_mutex_unlock(&mutexSabotajeActivado);

        while (temp_planificacion_viva && temp_sabotaje_activado) {

            sem_wait(&semERM);

            pthread_mutex_lock(&mutexCiclosTranscurridosSabotaje);
            if(ciclos_transcurridos_sabotaje == duracion_sabotaje) // SI SE COMPLETÓ EL SABOTAJE
            {
                pthread_mutex_unlock(&mutexCiclosTranscurridosSabotaje);

                //SE SACA AL TRIPULANTE DE LA COLA DE EXEC Y SE LO COLOCA AL FINAL DE LA COLA DE BLOQ_EMER
                pthread_mutex_lock(&mutexExec);
                tcb* aux_Fixer_Fin = queue_pop(exec);
                pthread_mutex_unlock(&mutexExec);

                aux_Fixer_Fin->status = 'M';
                aux_Fixer_Fin->ciclosCumplidos = ciclos_cumplidos_fixer_pre_sabotaje;//para qué? Si es para RR, es innecesario

                pthread_mutex_lock(&mutexBloqEmer);
                queue_push(bloq_emer,(void*) aux_Fixer_Fin);
                pthread_mutex_unlock(&mutexBloqEmer);

                sem_post(&semMR);
            }
            else { // SI NO SE COMPLETÓ EL SABOTAJE
                pthread_mutex_unlock(&mutexCiclosTranscurridosSabotaje);

                list_sort(exec->elements, comparadorTid);
                while (!queue_is_empty(exec))// SE PASAN LOS TRIPULANTES DE EXEC A BLOCK_EMER
                {
                    pthread_mutex_lock(&mutexExec);
                    tcb* aux_TCB = queue_pop(exec);
                    pthread_mutex_unlock(&mutexExec);

                    aux_TCB->status = 'M';

                    pthread_mutex_lock(&mutexBloqEmer);
                    queue_push(bloq_emer, (void*) aux_TCB);
                    pthread_mutex_unlock(&mutexBloqEmer);

                    pthread_mutex_lock(&mutexBloqEmerSorted);
                    list_add_sorted(bloq_emer_sorted->elements,(void*) aux_TCB,ordenarMasCercano);
                    pthread_mutex_unlock(&mutexBloqEmerSorted);
                }

                list_sort(ready->elements, comparadorTid);
                while (!queue_is_empty(ready))// SE PASAN LOS TRIPULANTES DE READY A BLOCK_EMER
                {
                    pthread_mutex_lock(&mutexReady);
                    tcb* aux_TCB = queue_pop(ready);
                    pthread_mutex_unlock(&mutexReady);

                    aux_TCB->status = 'M';

                    pthread_mutex_lock(&mutexBloqEmer);
                    queue_push(bloq_emer, (void*) aux_TCB);
                    pthread_mutex_unlock(&mutexBloqEmer);

                    pthread_mutex_lock(&mutexBloqEmerSorted);
                    list_add_sorted(bloq_emer_sorted->elements,(void*) aux_TCB,ordenarMasCercano);
                    pthread_mutex_unlock(&mutexBloqEmerSorted);
                }

                tripulanteFixer = queue_pop(bloq_emer_sorted); // SE ELIGIÓ AL TRIPULANTE QUE VA A ARREGLAR EL SABOTAJE: FIXER

                while (!queue_is_empty(bloq_emer_sorted))// SE VACÍA LA COLA DE BLOQ_EMER_SORTED, YA NO ES NECESARIA
                    queue_pop(bloq_emer_sorted);

                if(tripulanteFixer->ciclosCumplidos != 0){ // SE GUARDA LA CANTIDAD DE CICLOS CUMPLIDOS DEL FIXER ANTES DE EMPEZAR EL SABOTAJE, Y SE LO COLOCA A 0
                    ciclos_cumplidos_fixer_pre_sabotaje = tripulanteFixer->ciclosCumplidos;
                    tripulanteFixer->ciclosCumplidos = 0;
                }

                //SE AVISA A IMS QUE SE ATENDERÁ EL SABOTAJE
                int idTripulante = tripulanteFixer->tid;
                
                pthread_mutex_lock(&mutexBuffer);
                buffer = _serialize(sizeof(int), "%d", idTripulante);
                _send_message(conexion_IMS, "DIS", ATIENDE_SABOTAJE, buffer, sizeof(int), logger);
                free(buffer);
                pthread_mutex_unlock(&mutexBuffer);

                // SACA AL FIXER DE BLOCK_EMER Y LO COLOCA EN READY
                pthread_mutex_lock(&mutexBloqEmer);
                tcb* aux_Fixer = list_remove(bloq_emer->elements,tripulanteFixer->tid);
                pthread_mutex_unlock(&mutexBloqEmer);

                aux_Fixer->status = 'R';

                pthread_mutex_lock(&mutexReady);
                queue_push(ready,(void*) aux_Fixer);
                pthread_mutex_unlock(&mutexReady);

                sem_post(&semRE);
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

void funcionhBloqEmeraReady (t_log* logger){// SE PASAN TODOS LOS TRIPULANTES QUE ESTÁN EN BLOQ_EMER, A READY
    
    pthread_mutex_lock(&mutexValidador);
    int temp_validador = validador;
    pthread_mutex_unlock(&mutexValidador);

  	while(temp_validador){
        
        pthread_mutex_lock(&mutexPlanificacionViva);
        int temp_planificacion_viva = planificacion_viva;
        pthread_mutex_unlock(&mutexPlanificacionViva);

        pthread_mutex_lock(&mutexSabotajeActivado);
        int temp_sabotaje_activado = sabotaje_activado;
        pthread_mutex_unlock(&mutexSabotajeActivado);

        while (temp_planificacion_viva && temp_sabotaje_activado) {

            sem_wait(&semMR);

            while (!queue_is_empty(bloq_emer))
            {
                pthread_mutex_lock(&mutexBloqEmer);
                tcb* aux_TCB = queue_pop(bloq_emer);
                pthread_mutex_unlock(&mutexBloqEmer);

                aux_TCB->status = 'R';

                pthread_mutex_lock(&mutexReady);
                queue_push(ready, (void*) aux_TCB);
                pthread_mutex_unlock(&mutexReady);
            }
            
            pthread_mutex_lock(&mutexSabotajeActivado);
            sabotaje_activado = 0;
            pthread_mutex_lock(&mutexSabotajeActivado);

            //SE ENVÍA A IMS QUE SE TERMINÓ EL SABOTAJE
            char* bufferAEnviar = string_new();
            string_append(&bufferAEnviar, "Se resolvio el sabotaje");
           
            pthread_mutex_lock(&mutexBuffer);
            buffer = _serialize(sizeof(int) + string_length(bufferAEnviar), "%s", bufferAEnviar);
            _send_message(conexion_IMS, "DIS", RESOLUCION_SABOTAJE, buffer, sizeof(int) + strlen(bufferAEnviar), logger);
            free(bufferAEnviar);
            free(buffer);
            pthread_mutex_unlock(&mutexBuffer);

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