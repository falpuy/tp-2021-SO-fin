#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <commons/config.h>
#include <math.h>
#include <commons/log.h>
#include <commons/bitarray.h>
#include <commons/string.h>
#include <unistd.h> 

char* puntoMontaje = "/home/utnso/Escritorio/TP/tp-2021-1c-Unnamed-Group/IMongoStore/test";

char* pathCompleto(char* strConcatenar){
    return string_from_format("%s/%s",puntoMontaje,strConcatenar);
}


int main(){
    mkdir("Bitacoras", 0766);
    char* pathNow = pathCompleto("Bitacoras/Tripulante2.ims");
    
    if(access(pathNow, F_OK)<0){
        printf("No existe archivo\n");
        printf("Creando file...");
        int creaFile = open(pathNow,O_CREAT | O_RDWR,0664);
        if(creaFile < 0){
            perror("Error: No se pudo crear el archivo");
        }else{
            printf("Se creo archivo con exito");
            close(creaFile);
        }
    }
    else{
        printf("Existe archivo");
    }

    free(pathNow);
    

    return 0;
}
