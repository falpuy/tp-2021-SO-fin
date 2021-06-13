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

//char* listaBloques = "[3,4,5]";
char* listaBloques = "[]";

char* crearNuevaListaBloques(char* listaVieja,int bloqueAgregar,int charAgregar){
    char* listaNueva = malloc(sizeof(listaVieja)+charAgregar);
    char caracter;
    int i = 0;
    char* bloque = string_itoa(bloqueAgregar);
    int asciiChar = (int) listaVieja[0];

    while(asciiChar!=93){
        listaNueva[i] = listaVieja[i];
        i++;
        asciiChar = (int)listaVieja[i];
    
    }
    printf("%s",listaNueva);
    if(charAgregar==2){ //necesita "," + el numero en si
        string_append(&listaNueva,",");
    }
    string_append(&listaNueva,bloque);
    string_append(&listaNueva,"]");
    printf("\n[crearListaNueva] La lista nueva a mandar es: %s\n",listaNueva);
    free(bloque);
    return listaNueva;
}

int main(){
    char* bloquesNuevos = crearNuevaListaBloques(listaBloques,3,1);

    printf("%s",bloquesNuevos);
    free(bloquesNuevos);

    return 0;
}
