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
#include <commons/process.h>
#include <pthread.h>
#include <semaphore.h>
#include <commons/collections/queue.h>
#include <commons/config.h>


int main(){
    t_config* metadata = config_create("./c.config");
    char** listaBloques = config_get_array_value(metadata,"BLOCKS");

    int contador = 0;
    int bloque;
    int cantidadBloquesUsados = 0;
    char* string_temp = string_new();

int setearMD5(char* pathMetadata){
    char *comando = string_new();

    t_config* metadata = config_create(pathMetadata);
    char** listaBloques = config_get_array_value(metadata,"BLOCKS");
    int contador = 0;
    char* strTemporal = string_new();


    while(listaBloques[contador]){ 
        contador++;
    }

    int bloquesHastaAhora = 0;
    for(int i = 0; i <= contador; i++){
            
        if((contador - bloquesHastaAhora) != 1){
            bloque = atoi(listaBloques[bloquesHastaAhora]);

            char* temporalBloque = malloc(tamanioBloque+1);
            memcpy(copiaBlocks + bloque*tamanioBloque, temporalBloque, tamanioBloque);
            temporalBloque[tamanioBloque] = '\0';
                
            string_append(&string_temp,temporalBloque);
            bloquesHastaAhora++;
            free(temporalBloque);
        }else{
            bloque = atoi(listaBloques[bloquesHastaAhora]);

            int sizeVieja = config_get_int_value(metadata, "SIZE");
            int fragmentacion = contador*tamanioBloque - sizeVieja;

            char* temporalBloque = malloc(fragmentacion+1);
            memcpy(copiaBlocks + bloque*tamanioBloque, temporalBloque, fragmentacion);
            temporalBloque[fragmentacion] = '\0';
                
            string_append(&string_temp,temporalBloque);
            free(temporalBloque);
        }
            
    }


    FILE* archivo = fopen("temporal.txt","w");
    fprintf(fptr,"%d",num);


    string_append(&comando, "md5sum temporal.txt > resultado.txt");
    system(comando);



}

    return 0;
}

