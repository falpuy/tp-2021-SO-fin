#include "./headers/actualizarMetadata.h"


char* crearNuevaListaBloques(char* listaVieja,int bloqueAgregar){
    int tamListaVieja = strlen(listaVieja);
    char *bloque = string_itoa(bloqueAgregar);
    char* listaNueva;

    listaNueva = malloc(tamListaVieja);
    memcpy(listaNueva, listaVieja, tamListaVieja - 1); //copio hasta ']'
    listaNueva[tamListaVieja - 1] = '\0';

    if (tamListaVieja > 2) {
        string_append_with_format(&listaNueva, ",%s]", bloque);
    } else {
        string_append_with_format(&listaNueva, "%s]", bloque);
    }

    free(bloque);
    
    return listaNueva;
}

void actualizarBlocks(t_config* metadataBitacora,int bloque){
    char* lista = config_get_string_value(metadataBitacora,"BLOCKS"); 

    log_info(logger,"[ActualizarBlocks] Muestro lista levantada vieja de metadata:%s\t",lista);
    char* bloquesNuevos = crearNuevaListaBloques(lista,bloque);
    log_info(logger,"\n[ActualizarBlocks] Lista de bloques nuevos: %s\n",bloquesNuevos);

    config_set_value(metadataBitacora,"BLOCKS",bloquesNuevos);
    config_save(metadataBitacora);

    free(bloquesNuevos);
}

int setearMD5(char* pathMetadata){
    char *comando = string_new();
    string_append(&comando, "md5sum ");
    string_append(&comando, pathMetadata);
    return system(comando);
}

void actualizarBlockCount(t_config* metadataBitacora){
    int blockCount = config_get_int_value(metadataBitacora,"BLOCK_COUNT");
    blockCount += 1;

    //vuelvo a convertir en str de nuevo asi lo vuelvo a pegar
    char* str_blockCount = string_new();
    char* temp = string_itoa(blockCount);
    string_append(&str_blockCount, temp);
    free(temp);
    config_set_value(metadataBitacora,"BLOCK_COUNT",str_blockCount);
    free(str_blockCount);
    config_save(metadataBitacora);
}

void actualizarSize(t_config* metadataBitacora){
    int size = config_get_int_value(metadataBitacora,"SIZE");
    size += tamanioBloque;

    //vuelvo a convertir en str de nuevo asi lo vuelvo a pegar
    char* str_size = string_new();
    char* temp = string_itoa(size);
    string_append(&str_size, temp);
    free(temp);
    config_set_value(metadataBitacora,"SIZE",str_size);
    free(str_size);
    config_save(metadataBitacora);

}


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

void crearMetadataBitacora(char* path_tripulante){
    int fd = creat(path_tripulante,0666);
    if(fd < 0){
        perror("Error:");
    }else{
        close(fd);
        t_config* bitacoraTripulante = config_create(path_tripulante);
        
        config_set_value(bitacoraTripulante, "SIZE", "0");
        config_set_value(bitacoraTripulante, "BLOCKS", "[]");

        config_save_in_file(bitacoraTripulante,path_tripulante);
        config_destroy(bitacoraTripulante);

    }
}


int cantidad_bloques(char* string){
    double cantidad;
    double tamanioString = string_length(string);
    cantidad = tamanioString / tamanioBloque;
    int valorFinal = (int) ceil(cantidad);

    return valorFinal; //round up
}



char* strMoverTripultante(int idTripulante,int posX_v,int posY_v,int posX_n,int posY_n){
    char* temporal = string_new();
    char* posicion  = string_itoa(posX_v);
    string_append(&temporal,"Se mueve de ");
    string_append(&temporal,posicion);
    free(posicion);
    string_append(&temporal,"|");
    posicion  = string_itoa(posY_v);
    string_append(&temporal,posicion);
    free(posicion);
    string_append(&temporal," a ");
    posicion  = string_itoa(posX_n);
    string_append(&temporal,posicion);
    free(posicion);
    string_append(&temporal,"|");
    posicion = string_itoa(posY_n);
    string_append(&temporal,posicion);
    free(posicion);
    return temporal;
}

char* crearStrTripulante(int idTripulante){
    char* posicion = string_itoa(idTripulante);
    char* tripulante = string_new();
    string_append(&tripulante,"Tripulante");
    string_append(&tripulante,posicion);
    free(posicion);
    string_append(&tripulante,".ims");
    return tripulante;
}

int validarBitsLibre(int cantidadBloquesAUsar){
    int contador = 0;

    for(int i=0; i < bitarray_get_max_bit(bitmap); i++){
        if(bitarray_test_bit(bitmap,i) == 0){
            contador++;
            if(contador == cantidadBloquesAUsar){
                return 1;
            }
        }
    }
    return -1;
}

char* pathCompleto(char* strConcatenar){
    return string_from_format("%s/%s",datosConfig->puntoMontaje,strConcatenar);
}


char* obtenerBitacora(int tripulante){
    char* buffer = string_new();
    char* strPath = string_new();
    string_append(&strPath, "Bitacoras/");
    char* strTripulante = crearStrTripulante(tripulante);
    string_append(&strPath,strTripulante);

    char* path = pathCompleto(strPath);
    if(access(path,F_OK)){
        t_config* metadata = config_create(path);
        char* lista = config_get_string_value(metadata,"BLOCKS");
        int tamanioTotal = config_get_int_value(metadata,"SIZE");

        char** listaBloques = string_get_string_as_array(lista); //["2","3","9"]
        int cantidadBloquesTotal = cantidadBloquesUsados(listaBloques);
        int contadorBloques = 0;

        for(int i = 0; i < cantidadBloquesTotal; i++){
            int bloque = atoi(listaBloques[i]);

            if((cantidadBloquesTotal - contadorBloques) !=1){//no es ultimo bloque
                char* temporal = malloc(tamanioBloque);
                memcpy(temporal,copiaBlocks+bloque*tamanioBloque,tamanioBloque);
                string_append(&buffer,temporal);
                free(temporal);
            }else{
                int sizeStr = string_length(buffer); //Size hasta ahora
                int faltaCopiar = tamanioTotal - sizeStr; 
                char* temporal = malloc(faltaCopiar);
                memcpy(temporal,copiaBlocks+bloque*tamanioBloque,faltaCopiar);
                string_append(&buffer,temporal);
                free(temporal);
            }
        }
        config_destroy(metadata);
        free(lista);
        liberarArray(listaBloques,cantidadBloquesTotal);

    }else{
        log_error(logger, "No existe bitácora para ese tripulante.");
        return buffer; //size 0
    }

    free(strTripulante);
    free(strPath);
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