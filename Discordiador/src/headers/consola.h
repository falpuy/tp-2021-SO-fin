#ifndef CONSOLA_DISCORDIADOR_H
#define CONSOLA_DISCORDIADOR_H

    #include<stdio.h>
    #include<stdlib.h>
    #include<ctype.h>
    #include<commons/log.h>
    #include<commons/string.h>
    #include<commons/config.h>
    #include<commons/temporal.h>
    #include<readline/readline.h>
    #include<sys/socket.h>
    #include<unnamed/socket.h>
    #include<unnamed/serialization.h>
    //#include "headers/planificador.h"

    #define LARGO 7 //es lo mismo que const int largo = 7
    //void mostrarTripulante(TCB* tripulante, t_log* logger);
    //void mostrarListaTripulantes(t_list* patotas, t_log* logger);
    void funcionConsola(t_log* logger, int conexion_RAM, int conexion_IMS);

#endif