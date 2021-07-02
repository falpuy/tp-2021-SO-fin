#include "./headers/sabotajes.h"


void sabotaje(){

    char* str = string_new();
    //falta la posicion y sumar en 1 el contador para saber el prox a mandar
    string_append(&str, "VACIO");
    void* buffer = _serialize(sizeof(int)+ string_length(str) ,"%s",str);
    _send_message(socket, "IMS",COMIENZA_SABOTAJE, buffer,sizeof(int) + string_length(str), logger);

    free(str);
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
    
    int cantidadMal;
    memcpy(cantidadMal, superBloqueTemp + sizeof(uint32_t), sizeof(uint32_t));
    munmap(superBloqueTemp,sizeof(uint32_t)*2);

    if(cantidadBloques == cantidadMal){
        log_info("Cantidad de bloques... OK");
    }else{
        log_info("Cantidad de Bloques... NOT OK");
        log_info("Comienza Reparacion de SuperBloque");

        void* sb_memoria = mmap(NULL, sizeof(uint32_t)*2 + cantidadBloques/8, PROT_READ | PROT_WRITE, MAP_SHARED, archSB, 0);
        memcpy(sb_memoria + sizeof(uint32_t),cantidadBloques,sizeof(uint32_t));
        msync(sb_memoria,sizeof(uint32_t)*2 + cantidadBloques/8,0);
        munmap(sb_memoria,sizeof(uint32_t)*2 + cantidadBloques/8);        
        
        log_info("Finalizó reparación de SuperBloque");
    }
    //------------------------------------------------------------------------------------------------------
    log_info(logger,"-----------------------------------------------------");
    log_info(logger, "Validacion SuperBloque: Bitmap");

    log_info(logger, "Comienzo validación de directorio: Bitacora");

    char* tripulante = crearStrTripulante(testeoIDTripulante);
    char* bitacora = string_new();
    string_append(&bitacora, "Bitacoras/");
    string_append(&bitacora,tripulante);

    char* path_fileTripulante = pathCompleto(bitacora);
    free(bitacora);
    free(tripulante);

    superBloqueTemp = mmap(NULL, sizeof(uint32_t)*2 + cantidadBloques/8, PROT_READ | PROT_WRITE, MAP_SHARED, archSB, 0);
    t_bitarray* bitmapFalso;
    
    void* memoriaBitamp = malloc(cantidadBloques/8);
    memcpy(memoriaBitmap, superBloqueTemp + sizeof(uint32_t)*2, cantidadBloques/8);
    bitmapFalso = bitarray_create_with_mode((char*)memoriaBitmap, cantidadBloques / 8, MSB_FIRST);  
    
    log_info(logger, "Comienzo validación de Tripulantes..");
    
    while(access(path_fileTripulante,F_OK) >= 0){ 
        t_config* metadata = config_create(path_fileTripulante);
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

        free(path_fileTripulante);
        testeoIDTripulante++;
        
        //Armo proximo path de tripulante
        tripulante = crearStrTripulante(testeoIDTripulante);
        bitacora = string_new();
        string_append(&bitacora, "Bitacoras/");
        string_append(&bitacora,tripulante);
        path_fileTripulante = pathCompleto(bitacora);
        
        log_info(logger, "Terminó validación de Tripulantes..");

    }

    log_info(logger, "Comienzo validación de directorio: Files");

    
    if(access(pathCompleto("Files/Oxigeno.ims"),F_OK) >= 0){
        log_info(logger, "Comienzo validacion de Oxigeno.ims ..");

        metadata = config_create(pathCompleto("Files/Oxigeno.ims"));
        listaBloques = config_get_array_value(metadata,"BLOCKS");
        contador = 0;
        bloquesHastaAhora = 0;

        while(listaBloques[contador]){ 
            contador++;
        }

        for(int i = 0; i < contador; i++){        
            bloque = atoi(listaBloques[bloquesHastaAhora]);
            
            if(!bitarray_test_bit(bitmapFalso,bloque)){
                bitarray_set_bit(bitmapFalso,bloque);   
            };
        }

        for(int i = 0; i < contador; i++){
            free(listaBloques[i]);
        }
        free(listaBloques);
        config_destroy(metadata);
    }

    if(access(pathCompleto("Files/Comida.ims"),F_OK) >= 0){
        log_info(logger, "Comienzo validacion de Comida.ims ..");
        metadata = config_create(pathCompleto("Files/Comida.ims"));
        listaBloques = config_get_array_value(metadata,"BLOCKS");
        contador = 0;
        bloquesHastaAhora = 0;
        
        while(listaBloques[contador]){ 
            contador++;
        }

        for(int i = 0; i < contador; i++){        
            bloque = atoi(listaBloques[bloquesHastaAhora]);
            
            if(!bitarray_test_bit(bitmapFalso,bloque)){
                bitarray_set_bit(bitmapFalso,bloque);   
            };
        }

        for(int i = 0; i < contador; i++){
            free(listaBloques[i]);
        }
        free(listaBloques);
        config_destroy(metadata);
    }

    if(access(pathCompleto("Files/Basura.ims"),F_OK) >= 0){
        log_info(logger, "Comienzo validacion de Basura.ims ..");
        metadata = config_create(pathCompleto("Files/Basura.ims"));
        listaBloques = config_get_array_value(metadata,"BLOCKS");
        contador = 0;
        bloquesHastaAhora = 0;
        
        while(listaBloques[contador]){ 
            contador++;
        }

        for(int i = 0; i < contador; i++){        
            bloque = atoi(listaBloques[bloquesHastaAhora]);
            
            if(!bitarray_test_bit(bitmapFalso,bloque)){
                bitarray_set_bit(bitmapFalso,bloque);   
            };
        }

        for(int i = 0; i < contador; i++){
            free(listaBloques[i]);
        }
        free(listaBloques);
        config_destroy(metadata);
    }

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
        int bloquesHastaAhora = 0;
        int bloque;

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
        int bloquesHastaAhora = 0;
        int bloque;

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
        int bloquesHastaAhora = 0;
        int bloque;

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