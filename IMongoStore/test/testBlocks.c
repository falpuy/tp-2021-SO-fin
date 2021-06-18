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

char* str = "Se mueve de 8|9 a 10|1";

int main(){
    mkdir("Bitacoras", 0766);
    char* p_tripulante = string_from_format("%s/%s", path, "Bitacoras/Tripulante.ims");

    printf("\n%s\n", p_tripulante);

    if(access(p_tripulante,F_OK)<0){
        printf("No existe archivo\n");
        printf("%s\n", p_tripulante);
        printf("Creando file...");
        int creaFile = open(p_tripulante,O_RDWR,0664);
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
    

    return 0;
}