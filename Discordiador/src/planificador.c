#include "headers/planificador.h"

// hilos que pasan de estado
// algoritmos (FIFO y RR)

// crear hilo

void funcionPlanificador(t_log* logger) {
    // cola de estados (queue)
    t_queue* estadoNew;
    estadoNew = queue_create();
    t_queue* estadoReady;
    estadoReady = queue_create();
    t_queue* estadoExec;
    estadoExec = queue_create();
    t_queue* estadoBloqueadoIO;
    estadoBloqueadoIO = queue_create();
    t_queue* estadoBloqueadoEmergencia;
    estadoBloqueadoEmergencia = queue_create();
    t_queue* estadoExit;
    estadoExit = queue_create();

    // nuestra lista de PCB y TCB
    t_list* listaTCB;
    t_list* listaPCB;

    typedef struct
    {
	char* PID; //Identificador de la Patota
    char* bufferIDTrip; //Dirección lógica del inicio de las Tareas
    }PCB;

    typedef struct
    {
	char* TID; //Identificador del Tripulante
    char* status; //Estado del Tripulante
    int posicionX; // Posición del tripulante en el Eje X
    int posicionY; // Posición del tripulante en el Eje Y
    char* sig_instruccion; // Identificador de la próxima instrucción a ejecutar
    PCB* patota; // Dirección lógica del PCB del tripulante
    }TCB;

    listaTCB = list_create();
    listaPCB = list_create();

    nodoPatota* np1 = malloc (sizeof(nodoPatota));
    nodoTripulante* npt1 = malloc (sizeof(nodoTripulante));
}

void funcionEliminarListaPatotas(void* nodoPatota) {

    free (np1 -> PID);
    //free (np1 -> cantTrip); este no porque es int???
    free (np1 -> bufferIDTrip);
    free (np1);
}

void funcionEliminarListaTripulantes(void* nodoTripulante) {

    free (nt1 -> TID);
    free (nt1 -> PID);
    free (nt1 -> status);
    //free (nt1 -> posicionX; estos no porque son int???
    //free (nt1 -> posicionY);
    free (nt1);
}

void funcionTerminarPlanificacion() {
    list_destroy_and_destroy_elements (listaPCB, funcionEliminarListaPatotas);
    list_destroy_and_destroy_elements (listaTCB, funcionEliminarListaTripulantes);
}