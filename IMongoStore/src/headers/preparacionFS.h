#ifndef PREPARACION_FS_H
#define PREPARACION_FS_H

    #include <stdio.h>
    #include <stdlib.h>
    #include <commons/log.h>
    #include <commons/config.h>
    #include <string.h>
    #include <unnamed/validation.h>
    #include <unistd.h>
    #include <commons/string.h>



    int cantidadBloques;
    int tamanioBloque;



    char* pathArchivo(char* puntoDeMontaje, char* nombreArchivo);
    void setearConfiguracion();
    void crearEstructurasFS();


#endif