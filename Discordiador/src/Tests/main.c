#include<stdio.h>
#include<stdlib.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include<unnamed/socket.h>
#include<unnamed/serialization.h>
#include <unnamed/server.h>

void handler(int client, char* identificador, int comando, void* payload, t_log* logger){
    char* str;
    char* buffer;
    void* buffercito;
    switch (comando) {
        case 610:
            log_info(logger, "me llegó para iniciar_patota :)");
            int offset = 0, contadorPCBs,tid,posx, posy;
            char status;
            //SEPARO LO QUE ME MANDA
            /*memcpy(&contadorPCBs,payload,sizeof(int));
            memcpy(&tid,payload + sizeof(int), sizeof(int));
            memcpy(&posx,payload + sizeof(int)*2, sizeof(int));
            memcpy(&posy,payload + sizeof(int)*3, sizeof(int));
            memcpy(&status,payload + sizeof(int)*4, sizeof(char));
            log_info(logger, "%d %d %d %d %c", contadorPCBs,tid,posx, posy,status);*/
            buffer = string_new();
            string_append(&buffer, "hola");
            _send_message(client, "RAM", 200, buffer, strlen(buffer), logger);
            free(buffer);
            break;
        case 520:
            str = string_new();
            string_append(&str, "CANTAR;1;2;3");
            buffercito = _serialize(sizeof(int)+string_length(str), "%s", str);
            _send_message(client, "RAM", 200, buffercito, string_length(str)+sizeof(int), logger);
            free(buffercito);
            free(str);
            break;
        case 521:
            str = string_new();
            string_append(&str, "CANTAR;1;2;3");
            buffercito = _serialize(sizeof(int)+string_length(str), "%s", str);
            _send_message(client, "RAM", 560, buffercito, string_length(str)+sizeof(int), logger);
            free(buffercito);
            free(str);
            break;
        case 530:
            log_info(logger, "me llegó un comando para expulsar un tripulante :(");
            break;
        case 750:
            log_info(logger, "me llegó un comando :)");
            break;
    }
}

int main ()
{
    
    t_log* logger;
    logger = log_create("test.log", "test", 1, LOG_LEVEL_INFO);
    
    t_config* config;
    config = config_create("test.config");
    char* puerto_RAM;
    puerto_RAM = config_get_string_value(config, "PUERTO");
    int conexion_RAM;
    char* ip = "127.0.0.1";
    _start_server(puerto_RAM, handler, logger);
    

    return 0;
}