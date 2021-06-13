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


    char* tarea = "COMIENZA_EJECUCION_TAREA";
    printf("\nTamaño string:%d",string_length(tarea));
    void* bufferAMandar = _serialize(sizeof(int)*6 + string_length(tarea), "%d%d%s%d%d%d%d",101,string_length(tarea)+sizeof(int),tarea,20,3,15,10);
    int c_tamanioTarea,c_posX,c_posY,c_tiempo,c_idTripulante,c_parametros;
    int c_offset = 0;
    char* c_tarea;

    memcpy(&c_idTripulante,bufferAMandar,sizeof(int)); //idTripulante
    c_offset +=sizeof(int);
    memcpy(&c_tamanioTarea,bufferAMandar+c_offset,sizeof(int)); //tareaLen
    c_offset += sizeof(int);
    c_tarea = malloc(c_tamanioTarea+1);
    memcpy(c_tarea,bufferAMandar + c_offset,c_tamanioTarea);
    c_tarea[c_tamanioTarea] = '\0';
    c_offset+= c_tamanioTarea+1;
    memcpy(&c_parametros,bufferAMandar + c_offset, sizeof(int));//parametros
    c_offset += sizeof(int);
    memcpy(&c_posX,bufferAMandar + c_offset, sizeof(int));//pos x
    c_offset += sizeof(int);
    memcpy(&c_posY,bufferAMandar + c_offset, sizeof(int)); //pos y
    c_offset += sizeof(int);
    memcpy(&c_tiempo,bufferAMandar + c_offset, sizeof(int)); //tiempo
    
    log_info(log, "ID Tripulante:\t%d", c_idTripulante);
    log_info(log, "Tam tarea:\t%d", c_tamanioTarea);
    log_info(log, "Tarea:\t%s", c_tarea);
    log_info(log, "Parametros tarea:\t%d", c_parametros);
    log_info(log, "Posicion en X:%\td", c_posX);
    log_info(log, "Posicion en Y:\t%d", c_posY);
    log_info(log, "Tiempo:\t%d", c_tiempo);
    _send_message(socket, "DIS",COMIENZA_EJECUCION_TAREA, bufferAMandar,sizeof(int)*6 + string_length(tarea), log);
    return 0;
}