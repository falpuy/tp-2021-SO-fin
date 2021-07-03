#include "./headers/sabotajes.h"


void sabotaje(){
    char* strPosiciones = string_new();
    string_append(&strPosiciones, posicionesSabotajes[contadorListaSabotajes]);
    char** posicion = string_split(strPosiciones,"|");

    void* buffer = _serialize(sizeof(int)*2,"%d%d",posicion[0],posicion[1]);
    _send_message(socketDiscordiador,"IMS",COMIENZA_SABOTAJE,buffer,sizeof(int)*2,logger);

    free(strPosiciones);
    free(posicion[0]);
    free(posicion[1]);
    free(posicion);
    free(buffer);
}

void protocolo_fsck(){
    log_info(logger,"-----------------------------------------------------");
    log_info(logger, "Se comienza protocolo FSCK");

    validacionSuperBloque();
    validacionFiles();

    log_info(logger, "Finalizó protocolo FSCK");
    log_info(logger,"-----------------------------------------------------");
}

void validacionSuperBloque(){
    
    log_info(logger,"-----------------------------------------------------");
    log_info(logger, "Validacion SuperBloque:Cantidad de Bloques");
    int archSB = open("../Filesystem/SuperBloque.ims", O_CREAT | O_RDWR, 0664);
    void* superBloqueTemp = mmap(NULL, sizeof(uint32_t)*2, PROT_READ | PROT_WRITE, MAP_SHARED, archSB, 0);
    
    int cantidadDisco;
    memcpy(&cantidadDisco, superBloqueTemp + sizeof(uint32_t), sizeof(uint32_t));
    munmap(superBloqueTemp,sizeof(uint32_t)*2);

    if(cantidadBloques == cantidadDisco){
        log_info(logger,"Cantidad de bloques... OK");
    }else{
        log_info(logger, "Cantidad de Bloques... NOT OK");
        log_info(logger,"Comienza Reparacion de SuperBloque");

        void* sb_memoria = mmap(NULL, sizeof(uint32_t)*2 + cantidadBloques/8, PROT_READ | PROT_WRITE, MAP_SHARED, archSB, 0);
        memcpy(sb_memoria + sizeof(uint32_t),&cantidadBloques,sizeof(uint32_t));
        msync(sb_memoria,sizeof(uint32_t)*2 + cantidadBloques/8,0);
        munmap(sb_memoria,sizeof(uint32_t)*2 + cantidadBloques/8);        
        
        log_info(logger, "Finalizó reparación de SuperBloque");
    }
    log_info(logger,"-----------------------------------------------------");
    log_info(logger, "Validacion SuperBloque: Bitmap");

    log_info(logger, "Comienzo validación de directorio: Bitacora");

    superBloqueTemp = mmap(NULL, sizeof(uint32_t)*2 + cantidadBloques/8, PROT_READ | PROT_WRITE, MAP_SHARED, archSB, 0);
    t_bitarray* bitmapFalso;
    
    void* memoriaBitmap = malloc(cantidadBloques/8);
    memcpy(memoriaBitmap, superBloqueTemp + sizeof(uint32_t)*2, cantidadBloques/8);
    bitmapFalso = bitarray_create_with_mode((char*)memoriaBitmap, cantidadBloques / 8, MSB_FIRST);  
    
    log_info(logger, "Comienzo validación de Tripulantes..");
    validacionBitmapTripulantes(bitmapFalso);
    

    log_info(logger, "Comienzo validación de directorio: Files");

    //Faltan mutex?
    char* pathOxigeno =  pathCompleto("Files/Oxigeno.ims");
    validacionBitmapRecurso(pathOxigeno,bitmapFalso);
    free(pathOxigeno);
    
    char* pathComida =  pathCompleto("Files/Comida.ims");
    validacionBitmapRecurso(pathComida,bitmapFalso);
    free(pathComida);

    char* pathBasura = pathCompleto("Files/Basura.ims");
    validacionBitmapRecurso(pathBasura,bitmapFalso);
    free(pathBasura);



    bitarray_destroy(bitmapFalso);
    free(memoriaBitmap);
    munmap(superBloqueTemp,sizeof(uint32_t)*2 + cantidadBloques/8);
    close(archSB);
    log_info(logger,"-----------------------------------------------------");

}



void validacionFiles(){
    log_info(logger,"-----------------------------------------------------");
    log_info(logger, "Validacion Files: Size");

    //logger

    log_info(logger, "Validacion Files: Block-Count y Blocks");

    if(access(pathCompleto("Files/Oxigeno.ims"),F_OK) >= 0){
        log_info(logger, "Block-Count y Blocks de Oxígeno.ims");
        t_config* metadata = config_create(pathCompleto("Files/Oxigeno.ims"));
        char** listaBloques = config_get_array_value(metadata,"BLOCKS");
        int block_count = config_get_int_value(metadata,"BLOCK_COUNT");
        int contador = 0;

        while(listaBloques[contador]){ 
            contador++;
        }

        if(contador != block_count){
            char* temporal = string_new();
            temporal = string_itoa(block_count);
            config_set_value(metadata,"BLOCK_COUNT",temporal);
            config_save(metadata);
            free(temporal);
        }
        //Free de listaBloques
        for(int i = 0; i < contador; i++){
            free(listaBloques[i]);
        }
        free(listaBloques);
        config_destroy(metadata);
    }
    
    if(access(pathCompleto("Files/Comida.ims"),F_OK) >= 0){
        log_info(logger, "Block-Count y Blocks de Comida.ims");

        t_config* metadata = config_create(pathCompleto("Files/Comida.ims"));
        char** listaBloques = config_get_array_value(metadata,"BLOCKS");
        int block_count = config_get_int_value(metadata,"BLOCK_COUNT");
        int contador = 0;

        while(listaBloques[contador]){ 
            contador++;
        }

        if(contador != block_count){
            char* temporal = string_new();
            temporal = string_itoa(block_count);
            config_set_value(metadata,"BLOCK_COUNT",temporal);
            config_save(metadata);
            free(temporal);
        }
        //Free de listaBloques
        for(int i = 0; i < contador; i++){
            free(listaBloques[i]);
        }
        free(listaBloques);
        config_destroy(metadata);
    }

    if(access(pathCompleto("Files/Basura.ims"),F_OK) >= 0){
        log_info(logger, "Block-Count y Blocks de Basura.ims");

        t_config* metadata = config_create(pathCompleto("Files/Basura.ims"));
        char** listaBloques = config_get_array_value(metadata,"BLOCKS");
        int block_count = config_get_int_value(metadata,"BLOCK_COUNT");
        int contador = 0;

        while(listaBloques[contador]){ 
            contador++;
        }

        if(contador != block_count){
            char* temporal = string_new();
            temporal = string_itoa(block_count);
            config_set_value(metadata,"BLOCK_COUNT",temporal);
            config_save(metadata);
            free(temporal);
        }
        //Free de listaBloques
        for(int i = 0; i < contador; i++){
            free(listaBloques[i]);
        }
        free(listaBloques);
        config_destroy(metadata);
    }
    log_info(logger, "Finalización Validacion Files: Block-Count y Blocks");
    log_info(logger,"-----------------------------------------------------");

    //Falta blocks y restauracion de archivo
}

char* pathCompleto(char* strConcatenar){
    return string_from_format("%s/%s",datosConfig->puntoMontaje,strConcatenar);
}

void validacionBitmapRecurso(char* pathRecurso,t_bitarray* bitmapFalso){
    if(access(pathRecurso,F_OK) >= 0){
        t_config* metadata = config_create(pathRecurso);
        char** listaBloques = config_get_array_value(metadata,"BLOCKS");
        int contador = 0;
        int bloque;

        while(listaBloques[contador]){ 
            contador++;
        }

        for(int i = 0; i <= contador; i++){        
            bloque = atoi(listaBloques[i]);
            
            if(!bitarray_test_bit(bitmapFalso,bloque)){
                bitarray_set_bit(bitmapFalso,bloque);   
            };
        }

        for(int i = 0; i <= contador; i++){
            free(listaBloques[i]);
        }
        free(listaBloques);
        config_destroy(metadata);
    }

}

void validacionBitmapTripulantes(t_bitarray* bitmapFalso){
    char* tripulante = crearStrTripulante(testeoIDTripulante);
    char* bitacora = string_new();
    string_append(&bitacora, "Bitacoras/");
    string_append(&bitacora,tripulante);

    char* pathTripulante = pathCompleto(bitacora);
    free(bitacora);
    free(tripulante);

    while(access(pathTripulante,F_OK) >= 0){ 
        t_config* metadata = config_create(pathTripulante);
        char** listaBloques = config_get_array_value(metadata,"BLOCKS");
        int contador = 0;
        int bloquesHastaAhora = 0;
        int bloque;

        while(listaBloques[contador]){ 
            contador++;
        }

        for(int i = 0; i < contador; i++){        
            bloque = atoi(listaBloques[bloquesHastaAhora]);
            
            if(!bitarray_test_bit(bitmapFalso,bloque)){
                bitarray_set_bit(bitmapFalso,bloque);   
            };
        }

        //Free de listaBloques
        for(int i = 0; i < contador; i++){
            free(listaBloques[i]);
        }
        free(listaBloques);
        config_destroy(metadata);

        free(pathTripulante);
        testeoIDTripulante++;
        
        //Armo proximo path de tripulante
        tripulante = crearStrTripulante(testeoIDTripulante);
        bitacora = string_new();
        string_append(&bitacora, "Bitacoras/");
        string_append(&bitacora,tripulante);
        pathTripulante = pathCompleto(bitacora);
        
        log_info(logger, "Terminó validación de Tripulantes..");

    }

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