#include <stdio.h>
#include <stdlib.h>
#include <unnamed/socket.h>
#include <commons/log.h>
#include <unnamed/serialization.h>

int main(){

    t_log* log = log_create("cliente.kog","clienteTest", 1,  LOG_LEVEL_INFO);
    
    int socket = _connect("127.0.0.1", "5001", log);

    char* buffer = malloc(strlen("testingSerializacion"));
    memcpy(buffer, "testingSerializacion", strlen("testingSerializacion"));
    void* bufferAMandar = _serialize(sizeof(int)+strlen(buffer),"%s",buffer);

    _send_message(socket,"TST",1,bufferAMandar,sizeof(int)+strlen(buffer) , log);

    free(buffer);

    return 0;
}