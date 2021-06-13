#include <stdio.h>
#include <stdlib.h>
#include <unnamed/socket.h>
#include <commons/log.h>
#include <unnamed/serialization.h>

enum COMANDOS{
    OBTENER_BITACORA = 760 ,
    MOVER_TRIPULANTE ,
    COMIENZA_EJECUCION_TAREA,
    FINALIZA_TAREA,
    ATENDER_SABOTAJE,
    RESUELTO_SABOTAJE
};


int main(){

    t_log* log = log_create("cliente.log","clienteTest", 1,  LOG_LEVEL_INFO);
    int socket = _connect("127.0.0.1", "5001", log);

    //MoverTripulante
    void* bufferAMandar = _serialize(sizeof(int)*5, "%d%d%d%d%d", 1,2,8,3,5);
    
    //Comienza ejecutar tarea
    //15GENERAR_OXIGENO 3 2 3
    // char* tarea = "GENERAR_OXIGENO";
    // int tamStr = string_length(tarea);

    // void* bufferAMandar = _serialize(sizeof(int)*4 + tamStr , "%d%s%d%d%d", tamStr, tarea,3, 3,5);


    _send_message(socket, "DIS",MOVER_TRIPULANTE, bufferAMandar, sizeof(int)*5, log);
    return 0;
}