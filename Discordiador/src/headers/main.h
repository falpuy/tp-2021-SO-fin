#ifndef MAIN_DISCORDIADOR_H
#define MAIN_DISCORDIADOR_H

    #include<stdio.h>
    #include<stdlib.h>
    #include<ctype.h>
    #include<pthread.h>
    #include<commons/log.h>
    #include<commons/string.h>
    #include<commons/config.h>
    #include<readline/readline.h>
    #include<sys/socket.h>
    #include <unnamed/server.h>
    #include <signal.h>

    #include"consola.h"

    void setearConfiguraciones ();
    void servidor();
    


#endif 