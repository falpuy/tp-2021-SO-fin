#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <commons/log.h>
#include <commons/config.h>
#include <pthread.h>

#include <unnamed/socket.h>
#include <unnamed/serialization.h>

typedef struct {
    int idPat;
    char* tripulantes[];
}PCB;

int main(){

     //crear log 

     t_log* logger = log_create("test.log", "GTS", 1, LOG_LEVEL_INFO);
     
     //connectarme a memoria(puerto 5001) _connect

     int socket = _connect("127.0.0.1", "5001",logger);

     //crear pcb

     PCB* patota = malloc(sizeof(PCB));
     patota->idPat = 10;
     patota->tripulantes[0] = malloc(sizeof("delfi"));
     patota->tripulantes[1] = malloc(sizeof("agus"));
     strcpy(patota->tripulantes[0], "delfi");
     strcpy(patota->tripulantes[1], "agus");

     //serializar pcb en un buffer void* buffer=serialize()

     int tamanio = sizeof(patota->idPat)+ strlen(patota->tripulantes[0])+1 + strlen(patota->tripulantes[1]+1);
     void* buffer = _serialize(tamanio, "%d%s%s", patota->idPat, patota->tripulantes[0], patota->tripulantes[1]);
     

     //mandar mensaje con el comando 510(recibir tareas patota) int socket=_connect
     //_send_message(socket, "GTS", 510, "HOLA", 4, logger);

     _send_message(socket, "GTS", 510, buffer, tamanio, logger);


    return (0);
}