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
    #include "actualizarMetadata.h"

    enum TAREAS_TRIPULANTE{
        GENERAR_OXIGENO = 1,
        CONSUMIR_OXIGENO,
        GENERAR_COMIDA,
        CONSUMIR_COMIDA,
        GENERAR_BASURA,
        DESCARTAR_BASURA   
    };
    typedef struct{
        char* puntoMontaje;
        char* puerto;
        int tiempoSincronizacion;   
    }configIMS;

    
    void* p_bitmap;
    t_bitarray* bitmap;

    int flagEnd;
    configIMS* datosConfig;
    char* mapArchivo;

    int comandoTarea(char* tarea);
    void comienzaEjecutarTarea(int lenTarea,char* tarea,int parametro,t_log* log);
    void finalizaEjecutarTarea(int lenTarea,char* tarea,int parametro,t_log* log);
    void consumirOxigeno(t_log* log, int parametroTarea);
    void consumirComida(t_log* log, int parametroTarea);
    void descartarBasura(t_log* log, int parametroTarea);
    void generarOxigeno(t_log* log, int parametroTarea);
    void generarComida(t_log* log, int parametroTarea);
    void generarBasura(t_log* log, int parametroTarea);
    void crearMetadataBitacora(char* path_tripulante, t_log* logger);
    void crearMetadataFiles(char* path,char* charLlenado, t_log* logger);
    char* pathCompleto(char* strConcatenar);
    int cantidad_bloques(char* string, t_log* logger);
    int validarBitsLibre(int cantidadBloquesAUsar, t_log* log);
    char* crearStrTripulante(int idTripulante);
    char* strMoverTripultante(int idTripulante,int posX_v,int posY_v,int posX_n,int posY_n);
    

#endif