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

   //COMIENZA EJECUCION TAREA: IdTripulante TamTarea Tarea ParametrosTarea 
    t_log* log = log_create("cliente.log","clienteTest", 1,  LOG_LEVEL_INFO);
    int socket = _connect("127.0.0.1", "5001", log);

    // char* tarea = string_new();
    // string_append(&tarea, "GENERAR_OXIGENO");
    // void* buffer = _serialize(sizeof(int)*3 + string_length(tarea),"%d%s%d",101,tarea,10);

    void* buffer = _serialize(sizeof(int),"%d",101);


    // string_append(&tarea, "CONSUMIR_OXIGENO");
    // void* buffer = _serialize(sizeof(int)*3 + string_length(tarea) ,"%d%s%d",101,tarea,4);

    // void* buffer = _serialize(sizeof(int)*6,"%d%d%d%d%d%d",101,3,4,5,6,7);


    // _send_message(socket, "DIS",COMIENZA_EJECUCION_TAREA, buffer,sizeof(int)*3 + string_length(tarea), log);  
    _send_message(socket, "DIS",OBTENER_BITACORA, buffer,sizeof(int), log);  

    return 0;
}