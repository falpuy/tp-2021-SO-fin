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

// nvalid write of size 1
// ==32462==    at 0x8049A57: setearMD5 (actualizarMetadata.c:125)
// ==32462==    by 0x804B249: guardarEnBlocks (guardarBlocks.c:113)
// ==32462==    by 0x804DE6C: generarOxigeno (tareas.c:291)
// ==32462==    by 0x804D531: comienzaEjecutarTarea (tareas.c:78)
// ==32462==    by 0x804AADD: handler (atenderTripulantes.c:160)
// ==32462==    by 0x405DC23: _thread_function (in /usr/lib/libunnamed.so)
// ==32462==    by 0x40BD294: start_thread (pthread_create.c:333)
// ==32462==    by 0x41BC0AD: clone (clone.S:114)
// ==32462==  Address 0x44b5de9 is 0 bytes after a block of size 33 alloc'd
// ==32462==    at 0x402C17C: malloc (in /usr/lib/valgrind/vgpreload_memcheck-x86-linux.so)
// ==32462==    by 0x8049A1C: setearMD5 (actualizarMetadata.c:121)
// ==32462==    by 0x804B249: guardarEnBlocks (guardarBlocks.c:113)
// ==32462==    by 0x804DE6C: generarOxigeno (tareas.c:291)
// ==32462==    by 0x804D531: comienzaEjecutarTarea (tareas.c:78)
// ==32462==    by 0x804AADD: handler (atenderTripulantes.c:160)
// ==32462==    by 0x405DC23: _thread_function (in /usr/lib/libunnamed.so)
// ==32462==    by 0x40BD294: start_thread (pthread_create.c:333)
// ==32462==    by 0x41BC0AD: clone (clone.S:114)


int main(){

   //COMIENZA EJECUCION TAREA: IdTripulante TamTarea Tarea ParametrosTarea 
    t_log* log = log_create("cliente.log","clienteTest", 1,  LOG_LEVEL_INFO);
    int socket = _connect("127.0.0.1", "5002", log);


    // for(int i = 0; i < 5; i++){
    // sleep(5);
    char* tarea = string_new();
    string_append(&tarea, "GENERAR_OXIGENO");
    void* buffer = _serialize(sizeof(int)*3 + string_length(tarea),"%d%s%d",1,tarea,15);
    _send_message(socket, "DIS",COMIENZA_EJECUCION_TAREA, buffer,sizeof(int)*3 + string_length(tarea), log);

    // sleep(5);
    char* tarea2 = string_new();
    string_append(&tarea2, "CONSUMIR_OXIGENO");
    void* buffer3 = _serialize(sizeof(int)*3 + string_length(tarea2),"%d%s%d",1,tarea2,15);
    _send_message(socket, "DIS",COMIENZA_EJECUCION_TAREA, buffer3,sizeof(int)*3 + string_length(tarea2), log);

    // // sleep(5);

    // char* tarea3 = string_new();
    // string_append(&tarea3, "GENERAR_COMIDA");
    // void* buffer2 = _serialize(sizeof(int)*3 + string_length(tarea3) ,"%d%s%d",0,tarea3,12);
    // _send_message(socket, "DIS",COMIENZA_EJECUCION_TAREA, buffer2,sizeof(int)*3 + string_length(tarea3), log);

    // // sleep(5);

    // // void* buffer = _serialize(sizeof(int)*6,"%d%d%d%d%d%d",101,3,4,5,6,7);
    // char* tarea4 = string_new();
    // string_append(&tarea4, "JUEGA_GEARS_OF_WAR");
    // void* buffer4 = _serialize(sizeof(int)*2 + string_length(tarea4),"%d%s",1,tarea4);
    // _send_message(socket, "DIS",COMIENZA_EJECUCION_TAREA, buffer4,sizeof(int)*2 + string_length(tarea4), log);

    // // sleep(5);

    // char* tarea5 = string_new();
    // string_append(&tarea5, "GENERAR_BASURA");
    // void* buffer5 = _serialize(sizeof(int)*3 + string_length(tarea5),"%d%s%d",1,tarea5,12);
    // _send_message(socket, "DIS",COMIENZA_EJECUCION_TAREA, buffer5,sizeof(int)*3 + string_length(tarea5), log);

    // char* tarea6 = string_new();
    // string_append(&tarea6, "JUEGA_HALO");
    // void* buffer6 = _serialize(sizeof(int)*2 + string_length(tarea6),"%d%s",1,tarea6);
    // _send_message(socket, "DIS",COMIENZA_EJECUCION_TAREA, buffer6,sizeof(int)*2 + string_length(tarea6), log);

    // char* tarea7 = string_new();
    // string_append(&tarea7, "GRITARLE_A_PATOTA_B");
    // void* buffer7 = _serialize(sizeof(int)*2 + string_length(tarea7),"%d%s",1,tarea7);
    // _send_message(socket, "DIS",COMIENZA_EJECUCION_TAREA, buffer5,sizeof(int)*2 + string_length(tarea7), log);

    // // }

    
    return 0;
}