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

int cantidadBloquesAUsar = 7;
int cantidadBloquesUsados = 0;

int main(){
    t_config* bloques = config_create("blocks.config");
    char* listaBloques=malloc(3);
    char* numero = "4";
    char* corchete1 = "[";
    char* corchete2 = "]";
    
    //listaBloques = realloc(listaBloques,3); //guardo 2 para el n° y coma

    memcpy(listaBloques,corchete1,1);
    memcpy(listaBloques+1,numero,1);
    memcpy(listaBloques+2,corchete2, 1);
    printf("%s",listaBloques);
    config_set_value(bloques,"BLOCKS",listaBloques);

    config_save(bloques);
    config_destroy(bloques);

    return 0;
}
