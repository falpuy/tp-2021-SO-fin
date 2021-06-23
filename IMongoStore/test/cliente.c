#include <stdio.h>
#include <stdlib.h>
#include <unnamed/socket.h>
#include <commons/log.h>
#include <commons/string.h>
#include <unnamed/serialization.h>
#include <pthread.h>
#include <unnamed/server.h>

enum COMANDOS{
    OBTENER_BITACORA = 760 ,
    MOVER_TRIPULANTE ,
    COMIENZA_EJECUCION_TAREA,
    FINALIZA_TAREA,
    ATENDER_SABOTAJE,
    RESUELTO_SABOTAJE,
    RESPUESTA_BITACORA
};


int main(){

   IdTripulante TamTarea Tarea ParametrosTarea X Y T
    t_log* log = log_create("cliente.log","clienteTest", 1,  LOG_LEVEL_INFO);
    int socket = _connect("127.0.0.1", "5001", log);

    void* buffer = _serialize(sizeof(int)*5 ,"%d%d%d%d%d",101,3,5,6,4);

    _send_message(socket, "DIS",MOVER_TRIPULANTE, buffer,sizeof(int)*5 , log);  

    return 0;
}