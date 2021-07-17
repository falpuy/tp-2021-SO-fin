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

    // char* tarea = string_new();
    // string_append(&tarea, "GENERAR_OXIGENO");
    // void* buffer = _serialize(sizeof(int)*3 + string_length(tarea),"%d%s%d",101,tarea,10);
    // _send_message(socket, "DIS",COMIENZA_EJECUCION_TAREA, buffer,sizeof(int)*3 + string_length(tarea), log);

    // char* tarea2 = string_new();
    // string_append(&tarea2, "GENERAR_OXIGENO");
    // void* buffer1 = _serialize(sizeof(int)*3 + string_length(tarea2),"%d%s%d",101,tarea2,10);
    // _send_message(socket, "DIS",COMIENZA_EJECUCION_TAREA, buffer1,sizeof(int)*3 + string_length(tarea2), log);


    // void* buffer = _serialize(sizeof(int),"%d",101);
    char* tarea3 = string_new();
    string_append(&tarea3, "CONSUMIR_OXIGENO");
    void* buffer2 = _serialize(sizeof(int)*3 + string_length(tarea3) ,"%d%s%d",101,tarea3,7);
    _send_message(socket, "DIS",COMIENZA_EJECUCION_TAREA, buffer2,sizeof(int)*3 + string_length(tarea3), log);

    // void* buffer = _serialize(sizeof(int)*6,"%d%d%d%d%d%d",101,3,4,5,6,7);



   
    
    return 0;
}