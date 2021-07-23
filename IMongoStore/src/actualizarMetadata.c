#include "./headers/actualizarMetadata.h"


char* crearNuevaListaBloques(char* listaVieja,int bloqueAModificar, int flagEsGuardar,char* path){
    int tamListaVieja = string_length(listaVieja);
    char *bloque = string_itoa(bloqueAModificar);
    char* listaNueva  = malloc(tamListaVieja);

    memcpy(listaNueva, listaVieja, tamListaVieja - 1); //copio hasta ']'
    listaNueva[tamListaVieja - 1] = '\0';

    if(flagEsGuardar){
        if (tamListaVieja > 2) { // [2,3,5
            string_append_with_format(&listaNueva, ",%s]", bloque);
        } 
        else{
            string_append_with_format(&listaNueva, "%s]", bloque);
        }
    }else{ //Es para borrar
        if (tamListaVieja > 2) { //[2,44
            int tamListaNueva = string_length(listaNueva);
            t_config* metadata = config_create(path);
            int bloques = config_get_int_value(metadata,"BLOCK_COUNT");
            config_destroy(metadata);

            if(bloques == 0){
                char* temporal = string_new();
                string_append(&temporal,"[]");
                free(bloque);
                free(listaNueva);
                return temporal;
            }

            int aux = tamListaNueva;
            
            while(aux != 0 && memcmp(listaNueva + aux, ",", 1)) { 
                aux--; 
            }

            char* listaTemporal = malloc(aux+2);
            memcpy(listaTemporal, listaNueva,aux);//copio hasta antes del numero y su coma
            listaTemporal[aux] = ']';
            listaTemporal[aux+1] = '\0';
            //string_append(&listaTemporal, "]"); 
            free(listaNueva);

            //vuelvo a crear la listaNueva para poder retornarla
            listaNueva = malloc(aux+2);
            memcpy(listaNueva, listaTemporal,aux+1);
            listaNueva[aux+1] = '\0';
            free(listaTemporal);

        }
    }
    free(bloque);
    return listaNueva;
}

void actualizarBlocks(int bloque,int flagEsGuardar,char* path){
    t_config* metadataBitacora = config_create(path);
    char* lista = config_get_string_value(metadataBitacora,"BLOCKS"); 
    char* bloquesNuevos = crearNuevaListaBloques(lista,bloque,flagEsGuardar,path);

    config_set_value(metadataBitacora,"BLOCKS",bloquesNuevos);
    config_save(metadataBitacora);
    config_destroy(metadataBitacora);

    free(bloquesNuevos);
}

void setearMD5(char* pathMetadata){
    char *comando = string_new();

    t_config* metadata = config_create(pathMetadata);
    char** listaBloques = config_get_array_value(metadata,"BLOCKS");
    int contador = 0;
    char* string_temp = string_new();
    int bloquesHastaAhora = 0;
    int bloque;

    while(listaBloques[contador]){ 
        contador++;
    }

    for(int i = 0; i < contador; i++){
        if((contador - bloquesHastaAhora) != 0){ //no es el ultimo bloque-->no hay frag. interna
            
            bloque = atoi(listaBloques[bloquesHastaAhora]);
            char* temporalBloque = malloc(tamanioBloque+1);
            memcpy(temporalBloque, copiaBlocks + bloque*tamanioBloque, tamanioBloque);
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
    log_info(logger, "String total para el MD5:%s", string_temp);
    
    FILE* archivo = fopen("temporal.txt","w");
    fprintf(archivo,"%s",string_temp);
    fclose(archivo);

    string_append(&comando, "md5sum temporal.txt > resultado.txt");
    system(comando);

    char* md5 = malloc(33); //32 + \0

    FILE* archivo2 = fopen("resultado.txt","r");
    fscanf(archivo2,"%s",md5);
    md5[32] = '\0';
    fclose(archivo2);

    int err = remove("temporal.txt");
    if (err < 0){
        log_error(logger, "Error al remover archivo temporal.txt");
    }
    
    err = remove("resultado.txt");
    if(err < 0 ){
        log_error(logger, "Error al remover archivo resultado.txt");
    }

    config_set_value(metadata,"MD5",md5);
    config_save(metadata);
    
    for(int i = 0; i < contador; i++){
        free(listaBloques[i]);
    }
    free(listaBloques);
    free(md5);
    free(string_temp);
    free(comando);
    config_destroy(metadata);
}

void actualizarBlockCount(t_config* metadataBitacora,int flagEsGuardar){
    int blockCount = config_get_int_value(metadataBitacora,"BLOCK_COUNT");
    if(flagEsGuardar){
        blockCount += 1;
    }else{
        blockCount -= 1;
    }
    //vuelvo a convertir en str de nuevo asi lo vuelvo a pegar
    char* str_blockCount = string_new();
    char* temp = string_itoa(blockCount);
    string_append(&str_blockCount, temp);
    free(temp);
    config_set_value(metadataBitacora,"BLOCK_COUNT",str_blockCount);
    free(str_blockCount);
    config_save(metadataBitacora);
}

void actualizarSize(t_config* metadataBitacora,int tamanio, int flagEsGuardar){
    int size = config_get_int_value(metadataBitacora,"SIZE");
    if(flagEsGuardar){
        size += tamanio;
    }else{
        size -= tamanio;
    }
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


