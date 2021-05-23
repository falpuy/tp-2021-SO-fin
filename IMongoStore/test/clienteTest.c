#include <stdio.h>
#include <stdlib.h>
#include <unnamed/socket.h>
#include <commons/log.h>
#include <unnamed/serialization.h>

#define COMANDO 761


int main(){

    t_log* log = log_create("cliente.log","clienteTest", 1,  LOG_LEVEL_INFO);
    int socket = _connect("127.0.0.1", "5001", log);
    void* bufferAMandar = _serialize(sizeof(int)*5, "%d%d%d%d%d", 1,2,8,3,5);
    
    _send_message(socket, "DIS",COMANDO , bufferAMandar, sizeof(int)*5, log);
    return 0;
}