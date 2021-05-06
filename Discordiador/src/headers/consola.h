#ifndef CONSOLA_DISCORDIADOR_H
#define CONSOLA_DISCORDIADOR_H

    #include<stdio.h>
    #include<stdlib.h>
    #include<ctype.h>
    #include<commons/log.h>
    #include<commons/string.h>
    #include<commons/config.h>
    #include<readline/readline.h>
    #include<sys/socket.h>
    #include<unnamed/socket.h>

    #define LARGO 7 //es lo mismo que const int largo = 7
    void funcionConsola(t_log* logger);

#endif