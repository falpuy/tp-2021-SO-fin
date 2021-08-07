#include "./headers/actualizarMetadata.h"

void crearMetadataFiles(char* path,char* charLlenado){
    int fd = creat(path,0666);
    if(fd < 0){
        perror("Error:");
    }else{
        close(fd);
        t_config* file = config_create(path);
        
        config_set_value(file, "SIZE", "0");
        config_set_value(file, "BLOCK_COUNT", "0");
        config_set_value(file, "BLOCKS", "[]");
        config_set_value(file, "CARACTER_LLENADO", charLlenado);
        config_set_value(file, "MD5", "0");

        config_save_in_file(file,path);
        config_destroy(file);

    }
}

void crearMetadataBitacora(char* path){
    int fd = creat(path,0666);
    if(fd < 0){
        perror("Error:");
    }else{
        close(fd);
        t_config* bitacoraTripulante = config_create(path);
        
        config_set_value(bitacoraTripulante, "SIZE", "0");
        config_set_value(bitacoraTripulante, "BLOCKS", "[]");

        config_save_in_file(bitacoraTripulante,path);
        config_destroy(bitacoraTripulante);

    }
}





char* strMoverTripultante(int idTripulante,int posX_v,int posY_v,int posX_n,int posY_n){
    char* temporal = string_new();
    char* posicion  = string_itoa(posX_v);
    string_append(&temporal,"Se mueve de ");
    string_append(&temporal,posicion);
    free(posicion);
    string_append(&temporal,"-");
    posicion  = string_itoa(posY_v);
    string_append(&temporal,posicion);
    free(posicion);
    string_append(&temporal," a ");
    posicion  = string_itoa(posX_n);
    string_append(&temporal,posicion);
    free(posicion);
    string_append(&temporal,"-");
    posicion = string_itoa(posY_n);
    string_append(&temporal,posicion);
    free(posicion);
    return temporal;
}







char* obtenerBitacora(int tripulante){
    char* buffer = string_new(); 
    char* strPath = string_new();

    string_append(&strPath, "Bitacoras/");
    char* strTripulante = crearStrTripulante(tripulante);
    string_append(&strPath,strTripulante);

    char* path = pathCompleto(strPath);
    free(strTripulante);
    free(strPath);

    if(access(path,F_OK) >= 0){
        t_config* metadata = config_create(path);
        char** listaBloques = config_get_array_value(metadata,"BLOCKS");
        int tamanioTotal = config_get_int_value(metadata,"SIZE");
        int contadorBloques = 0;
        int posicionBloque = 0;

        tamanioTotal +=1;

        while(listaBloques[contadorBloques]){
            contadorBloques++;
        }

        for(int i = 0; i < contadorBloques; i++){
            int bloque = atoi(listaBloques[posicionBloque]);

            if((contadorBloques - posicionBloque) != 1){//no es ultimo bloque
                char* temporal = malloc(tamanioBloque + 1);
                memcpy(temporal,copiaBlocks+bloque*tamanioBloque,tamanioBloque);
                temporal[tamanioBloque] = '\0';
                string_append(&buffer,temporal);
                free(temporal);
                
            }else{
                int sizeStr = string_length(buffer); //Size hasta ahora
                int faltaCopiar = tamanioTotal - sizeStr; 
                char* temporal = malloc(faltaCopiar + 1);
                memcpy(temporal,copiaBlocks+bloque*tamanioBloque,faltaCopiar);
                temporal[faltaCopiar] = '\0';
                string_append(&buffer,temporal);
                free(temporal);
            }
            posicionBloque++;
        }
        config_destroy(metadata);
        for(int i = 0; i <= contadorBloques; i++){
            free(listaBloques[i]);
        }
        free(listaBloques);

    }else{
        log_error(logger, "No existe bitácora para ese tripulante.");
        return buffer; //size 0
    }
    free(path);
    return buffer;
}


int cantidadBloquesUsados(char** listaBloques){
    int contador = 0;

    for(int i=0; listaBloques[i] != NULL; i++){
        contador++;
    }

    return contador;
}

void liberarArray (char** array, int posiciones){
    for(int i=0; i < posiciones; i++){
        free(array[i]);
    }

    free(array);
}


