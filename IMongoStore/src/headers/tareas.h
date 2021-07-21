#ifndef TAREAS_H
#define TAREAS_H

    #include <stdio.h>
    #include <stdlib.h>
    #include <commons/string.h>
    #include <string.h>
    #include <unnamed/serialization.h>
    #include <unnamed/select.h>
    #include <unnamed/socket.h>
    #include <math.h>
    #include <commons/config.h>
    #include <commons/log.h>
    #include "guardarBlocks.h"

    enum TAREAS_TRIPULANTE{
        GENERAR_OXIGENO = 1,
        CONSUMIR_OXIGENO,
        GENERAR_BASURA,
        DESCARTAR_BASURA,
        GENERAR_COMIDA,
        CONSUMIR_COMIDA,
        TAREA_NO_IO,
        ERROR_NO_EXISTE_TAREA = -1
    };

    int comandoTarea(char* tarea);
    void finalizaEjecutarTarea(int lenTarea,char* tarea,int idTripulante);
    void comienzaEjecutarTarea(int lenTarea,char* tarea,int parametro,int idTripulante);
    
    void escribirEnBitacoraFinalizaTarea(char* tarea, int idTripulante);
    void escribirEnBitacoraComienzaTarea(char* tarea, int idTripulante);
    void consumirOxigeno( int parametroTarea);
    void consumirComida( int parametroTarea);
    void descartarBasura(int parametroTarea);
    void generarOxigeno(int parametroTarea);
    void generarComida(int parametroTarea);
    void generarBasura(int parametroTarea);


   
    

#endif