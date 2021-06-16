#include <stdio.h>
#include <stdlib.h>
#include <unnamed/socket.h>
#include <commons/log.h>
#include <commons/string.h>
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

    char* tarea = string_new();
    string_append(&tarea,"GENERAR_OXIGENO");

    int tamTarea,parametro, offset = 0;
    char* tarea2;
    void* buffer = _serialize(sizeof(int)*5 ,"%d%d%d%d%d",101,5,20,8,15);
    
    _send_message(socket, "DIS",MOVER_TRIPULANTE, buffer,sizeof(int)*5, log);
    return 0;
}