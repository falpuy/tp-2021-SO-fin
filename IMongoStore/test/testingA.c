#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unnamed/socket.h>
#include <unnamed/serialization.h>
#include <commons/log.h>
#include <string.h>
#include <unistd.h>
#include <commons/string.h>
#include <stdint.h>
#include <commons/bitarray.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdarg.h>
#include <sys/types.h>
#include <dirent.h>


int main(){

    char* listaBloques = string_new();
    string_append(&listaBloques, "[2,4,5,4]");
    int contador = 0;

    while(listaBloques[contador]){ //-->testear 
        contador++;
    }

    printf("\n%d", contador);
    free(listaBloques);
    

    return 0;
}