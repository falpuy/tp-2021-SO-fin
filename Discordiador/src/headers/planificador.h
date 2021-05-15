#ifndef PLANIFICADOR_DISCORDIADOR_H
#define PLANIFICADOR_DISCORDIADOR_H

    #include<stdio.h>
    #include<stdlib.h>
    #include<ctype.h>
    #include<commons/log.h>
    #include<commons/string.h>
    #include<commons/config.h>
    #include<commons/collections/queue.h>
    #include<commons/collections/list.h>
    #include<readline/readline.h>
    #include<sys/socket.h>
    #include<unnamed/socket.h>

    int contadorPCBs = 0;
    void funcionPlanificador(t_log* logger);
    TCB* crear_TCB(int posX, int posY, int id, char* tarea);
    PCB* crear_PCB(char* buffer_tareas, char** parametros, &int contadorPCBs);


#endif