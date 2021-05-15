#include "headers/planificador.h"

// hilos que pasan de estado
// algoritmos (FIFO y RR)

// crear hilo

void funcionPlanificador(t_log* logger) {
    // cola de estados (queue)
    //Poner en el main.h como globales las colas 
    t_queue* NEW; //main.h
    NEW = queue_create();
    t_queue* READY;
    READY = queue_create();
    t_queue* EXEC;
    EXEC = queue_create();
    t_queue* BLOQ_IO;
    BLOQ_IO = queue_create();
    t_queue* BLOQ_EMER;
    BLOQ_EMER = queue_create();
    t_queue* EXIT;
    EXIT = queue_create();

    // nuestra lista de PCB y TCB
    
    t_list* listaPCB;

    typedef struct
    {
	int TID = 0; //Identificador del Tripulante
    char* status; //Estado del Tripulante
    int posicionX = 0; // Posición del tripulante en el Eje X
    int posicionY = 0; // Posición del tripulante en el Eje Y
    char* instruccion_actual; // Nombre de la tarea que estamos ejecutando
    }TCB;

    typedef struct
    {
	int PID = 0; //Identificador de la Patota
    char* bufferTarea; //Buffer con las tareas de la patota -> el que le mandamos a RAM
    t_list* listaTCB;
    }PCB;

    listaPCB = list_create();
}

TCB* crear_TCB(int posX, int posY, int id, char* tarea)
{
    TCB* nuevoTCB = malloc (sizeof(TCB));
	nuevoTCB -> TID = id;
    nuevoTCB -> status = malloc (strlen("BLOCK_EMER") + 1); //BLOCK_EMER es el estado con el nombre largo en cuanto a caracteres
	strcpy(nuevoTCB -> status, "NEW");
    nuevoTCB -> posicionX = posX;
    nuevoTCB -> posicionY = posY;
    nuevoTCB -> instruccion_actual = malloc (strlen(tarea) + 1);
    strcpy(nuevoTCB -> instruccion_actual, tarea);
    return nuevoTCB;
}

PCB* crear_PCB(char* buffer_tareas, char** parametros, &int contadorPCBs)
{                        
    int cant_tripulantes = parametros[1];
    contadorPCBs++;
    PCB* nuevoPCB = malloc(sizeof(PCB));
    nuevoPCB -> PID = contadorPCBs;
    nuevoPCB -> listaTCB = list_create();
    int posX = 0;
    int posY = 0;
    bool hayParametros = true;
    for(int i = 1; i<=cant_tripulantes; i++)
    {
        if (hayParametros) {
            if (strcmp(parametros[2+i], NULL) == 0) {//Si no exite ese elemento/índice del vector, las posiciones son 0|0
                    hayParametros = false;
            }
            else {
                char** posicion = string_split(parametros[2+i], "|");
                posX = atoi(posicion[0]);
                posY = atoi(posicion[1]);
            }
        }
    int TID = (nuevoPCB -> PID) * 100 + i;
    //send y recv a memoria 
    nuevoTCB = crear_TCB(posX, posY, TID, /*tarea*/);
    list_add (nuevoPCB -> listaTCB, (void*) nuevoTCB);
    queue_push (NEW, nuevoTCB);
    }
    return nuevoPCB;
}

/*void funcionEliminarListaPatotas(void* nodoPatota) {

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
}*/