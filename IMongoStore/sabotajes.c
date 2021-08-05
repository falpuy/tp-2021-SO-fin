    #include "./headers/sabotajes.h"

void sabotaje(){
    char** strPosiciones = string_split(posicionesSabotajes[contadorSabotajeLeido],"|");
    int posicionX = atoi(strPosiciones[0]);
    int posicionY = atoi(strPosiciones[1]); 

    pthread_mutex_lock(&discordiador);
    int idDiscordiador = _connect(ipDiscordiador,puertoDiscordiador,logger);
    void* buffer = _serialize(sizeof(int)*2,"%d%d",posicionX,posicionY);
    _send_message(idDiscordiador,"IMS",COMIENZA_SABOTAJE,buffer,sizeof(int)*2,logger);
    pthread_mutex_unlock(&discordiador);

    contadorSabotajeLeido++; 
    free(buffer);
    free(strPosiciones[0]);
    free(strPosiciones[1]);
    free(strPosiciones);
 
    log_info(logger, "Envié mensaje de sabotaje a discordiador");
    // pthread_mutex_lock(&blocks_bitmap);
    // protocolo_fsck();
    // pthread_mutex_unlock(&blocks_bitmap);
}

void protocolo_fsck(){
    log_info(logger,"//////////////////////////////////////////////////////");
    log_info(logger, "Comenzando Protocolo:FSCK...");
    validacionSuperBloque();
    validacionFiles();
    log_info(logger, "Finalizando Protocolo:FSCK..");
    log_info(logger,"//////////////////////////////////////////////////////");
}
void validacionSuperBloque(){
    validarCantidadBloques();
    validarBitmapSabotaje();
}
void validacionFiles(){
    validarSizeFile();
    validarBlocksBlockCount();
    validacionBlocks();
}

void validarCantidadBloques(){
    log_info(logger,"---------------------------------------------------------");
    int archSB = open("./Filesystem/SuperBloque.ims", O_CREAT | O_RDWR, 0664);
    void* superBloqueTemp = mmap(NULL, sizeof(uint32_t)*2, PROT_READ | PROT_WRITE, MAP_SHARED, archSB, 0);
    
    uint32_t cantidadBloquesDisco;
    memcpy(&cantidadBloquesDisco, superBloqueTemp + sizeof(uint32_t), sizeof(uint32_t));
    munmap(superBloqueTemp,sizeof(uint32_t)*2);

    int sizeBlocks;    
    struct stat st;
    stat("./Filesystem/Blocks.ims", &st);
    sizeBlocks= st.st_size;

    uint32_t cantidadTemporal = (uint32_t) sizeBlocks / tamanioBloque;

    if(cantidadBloques == cantidadBloquesDisco){
        log_info(logger,"Cantidad de bloques... OK");
    }else{
        log_info(logger, "Cantidad de Bloques... NOT OK.");
        log_info(logger,"Reparación: Se sobreescribe la cantidadBloques");

        cantidadBloques = cantidadTemporal;
        superBloqueTemp = mmap(NULL, sizeof(uint32_t)*2 + cantidadBloques/8, PROT_READ | PROT_WRITE, MAP_SHARED, archSB, 0);
        memcpy(superBloqueTemp + sizeof(uint32_t), &cantidadBloques, sizeof(uint32_t));
        msync(superBloqueTemp, sizeof(uint32_t)*2 + cantidadBloques/8, MS_SYNC);
        munmap(superBloqueTemp,sizeof(uint32_t)*2 + cantidadBloques/8);

        log_info(logger, "Finalizó reparación de cantidadBloques");
    }
    close(archSB);
    log_info(logger,"---------------------------------------------------------");
    
}


// void validarBitmapSabotaje(){
//     char* strTestear;
//     char* strVacio;

//     int _superBloque = open("./Filesystem/SuperBloque.ims", O_CREAT | O_RDWR, 0664);
//     void* superBloqueTemp = mmap(NULL, sizeof(uint32_t) * 2 + cantidadBloques/8, PROT_READ | PROT_WRITE, MAP_SHARED, _superBloque, 0);

//     memcpy(copiaSB, superBloqueTemp, sizeof(uint32_t ) * 2 + cantidadBloques/8);
//     memcpy(memBitmap, superBloqueTemp + sizeof(uint32_t) *2, cantidadBloques/8);
//     bitarray_destroy(bitmap);
//     bitmap = bitarray_create_with_mode((char*) memBitmap,cantidadBloques/8,MSB_FIRST);

//     // log_info(logger, "Lo levantado bitmap:\n");
//     // for(int i=0; i < cantidadBloques; i++){
//     //     printf("%d", bitarray_test_bit(bitmapFalso,i));
//     // }
   
//     for(int i = 0; i<cantidadBloques; i++){
//         int testBitmap = bitarray_test_bit(bitmap,i);
        
//         strTestear = malloc(tamanioBloque + 1);
//         strVacio = malloc(tamanioBloque + 1);
//         memset(strVacio,' ',tamanioBloque);
//         strVacio[tamanioBloque] = '\0';

//         int value;
//         if(testBitmap){//Hay algo en bitmap
//             memcpy(strTestear, copiaBlocks + i * tamanioBloque,tamanioBloque);
//             strTestear[tamanioBloque] = '\0';
//             value = strcmp(strTestear,strVacio);
//             // log_info(logger, "Lo levantado es:%s del bloque:%d,", strTestear,i);
        
//             if(value == 0){ //si es vacio lo que levante
//                 log_info(logger,"Error en el bloque:%d. Bloque realmente vacio. Se cambia en bitmap",i);
//                 bitarray_clean_bit(bitmap,i);
//                 memcpy(copiaSB + sizeof(uint32_t) * 2, bitmap->bitarray, cantidadBloques/8);
//                 // corregirBitmap(1,bitmap, copiaSB);
//             }
//         }else if(testBitmap == 0){ 
//             memcpy(strTestear, copiaBlocks + i*tamanioBloque,tamanioBloque);   
//             strTestear[tamanioBloque] ='\0';
//             value = strcmp(strTestear,strVacio);

//             // log_info(logger, "Lo levantado es:%s, del bloque:%d,", strTestear,i);
//             if(value != 0){ 
//                 log_info(logger, "Error en el bloque:%d. Bloque no esta vacio. Se cambia en bitmap",i);
//                 bitarray_set_bit(bitmap,i);
//                 memcpy(copiaSB + sizeof(uint32_t) * 2, bitmap->bitarray, cantidadBloques/8);
//                 // corregirBitmap(0,bitmap, copiaSB);
//             }
//         }
//         free(strTestear);
//         free(strVacio);
//     }
//     // log_info(logger,"Validacion Bitmap... OK");

//     // for(int i=0; i < cantidadBloques; i++){
//     //     printf("%d", bitarray_test_bit(bitmap,i));
//     // }

//     superBloqueTemp = mmap(NULL, sizeof(uint32_t) * 2 + cantidadBloques/8, PROT_READ | PROT_WRITE, MAP_SHARED, _superBloque, 0);
//     memcpy(superBloqueTemp, copiaSB, sizeof(uint32_t) * 2 + cantidadBloques/8);
//     msync(superBloqueTemp, sizeof(uint32_t) * 2 + cantidadBloques/8, MS_SYNC);
//     munmap(superBloqueTemp,sizeof(uint32_t)*2 + cantidadBloques/8);


//     // free(copiaSB);
//     // bitarray_destroy(bitmapFalso);
//     // free(memoriaBitmap);
//     close(_superBloque);


// }
void validarBitmapSabotaje(){
    char* strTestear;
    char* strVacio;

    int _superBloque = open("./Filesystem/SuperBloque.ims", O_CREAT | O_RDWR, 0664);
    int _blocks = open("./Filesystem/Blocks.ims", O_CREAT | O_RDWR, 0664);
    
    void* superBloqueTemp = mmap(NULL, sizeof(uint32_t) * 2 + cantidadBloques/8, PROT_READ | PROT_WRITE, MAP_SHARED, _superBloque, 0);
    void* blocksTemp = mmap(NULL,tamanioBloque*cantidadBloques, PROT_READ | PROT_WRITE, MAP_SHARED, _blocks, 0);
    
    void* copiaSuperBloque = malloc(sizeof(uint32_t ) * 2 + cantidadBloques/8);
    void* memoriaBitmap = malloc(cantidadBloques/8);

    memcpy(copiaSuperBloque, superBloqueTemp, sizeof(uint32_t ) * 2 + cantidadBloques/8);
    memcpy(memoriaBitmap, superBloqueTemp + sizeof(uint32_t) *2, cantidadBloques/8);
    t_bitarray* bitmapFalso = bitarray_create_with_mode((char*) memoriaBitmap,cantidadBloques/8,MSB_FIRST);

    munmap(superBloqueTemp,sizeof(uint32_t)*2 + cantidadBloques/8);

    // log_info(logger, "*********************************************\n\n");
    
    // log_info(logger, "Lo levantado bitmap:\n");
    // for(int i=0; i < cantidadBloques; i++){
    //     printf("%d", bitarray_test_bit(bitmapFalso,i));
    // }

    // log_info(logger, "*********************************************\n\n");
    
   
    for(int i = 0; i < cantidadBloques; i++){
        int testBitmap = bitarray_test_bit(bitmapFalso,i);
        
        strTestear = malloc(tamanioBloque + 1);
        strVacio = malloc(tamanioBloque + 1);
        memset(strVacio,' ',tamanioBloque);
        strVacio[tamanioBloque] = '\0';

        int value;
        if(testBitmap){//Hay algo en bitmap
            memcpy(strTestear, blocksTemp + i * tamanioBloque,tamanioBloque);
            strTestear[tamanioBloque] = '\0';
            value = strcmp(strTestear,strVacio);
            // log_info(logger, "Lo levantado es:%s del bloque:%d,", strTestear,i);
        
            if(value == 0){ //si es vacio lo que levante
                log_info(logger,"Error en el bloque:%d. Bloque realmente vacio. Se cambia en bitmap",i);
                bitarray_clean_bit(bitmapFalso,i);
                memcpy(copiaSuperBloque + sizeof(uint32_t) * 2, bitmapFalso->bitarray, cantidadBloques/8);
                // corregirBitmap(1,bitmapFalso, copiaSuperBloque);
            }
        }else if(testBitmap == 0){ 
            memcpy(strTestear, blocksTemp + i*tamanioBloque,tamanioBloque);   
            strTestear[tamanioBloque] ='\0';
            value = strcmp(strTestear,strVacio);

            // log_info(logger, "Lo levantado es:%s, del bloque:%d,", strTestear,i);
            if(value != 0){ 
                log_info(logger, "Error en el bloque:%d. Bloque no esta vacio. Se cambia en bitmap",i);
                bitarray_set_bit(bitmapFalso,i);
                memcpy(copiaSuperBloque + sizeof(uint32_t) * 2, bitmapFalso->bitarray, cantidadBloques/8);
                // corregirBitmap(0,bitmapFalso, copiaSuperBloque);
            }
        }
        free(strTestear);
        free(strVacio);
    }
    log_info(logger,"Validacion Bitmap... OK");

    // log_info(logger, "*********************************************\n\n");
    // for(int i=0; i < cantidadBloques; i++){
    //     printf("%d", bitarray_test_bit(bitmapFalso,i));
    // }
    // log_info(logger, "*********************************************\n\n");

    superBloqueTemp = mmap(NULL, sizeof(uint32_t) * 2 + cantidadBloques/8, PROT_READ | PROT_WRITE, MAP_SHARED, _superBloque, 0);
    memcpy(superBloqueTemp, copiaSuperBloque, sizeof(uint32_t) * 2 + cantidadBloques/8);
    msync(superBloqueTemp, sizeof(uint32_t) * 2 + cantidadBloques/8, MS_SYNC);
    munmap(superBloqueTemp,sizeof(uint32_t)*2 + cantidadBloques/8);
    munmap(blocksTemp,tamanioBloque*cantidadBloques);


    free(copiaSuperBloque);
    bitarray_destroy(bitmapFalso);
    free(memoriaBitmap);
    close(_superBloque);


}
void corregirBitmap(int encontroVacio, t_bitarray* bitmapFalso, void* copiaSuperBloque){
    corregirBitmapTripulantes(encontroVacio,bitmapFalso, copiaSuperBloque);
    corregirBitmapRecursos(encontroVacio,bitmapFalso, copiaSuperBloque);
}
void corregirBitmapTripulantes(int encontroVacio,t_bitarray* bitmapFalso, void* copiaSuperBloque){
    log_info(logger,"-----------------------------------------------------");
    log_info(logger, "Correccion Bitmap: Tripulantes");
    
    int idTripulante = 0;
    char* temporal = string_new();
    string_append(&temporal, "Bitacora/");
    char* temp2 = crearStrTripulante(idTripulante);
    string_append(&temporal,temp2);

    char* path_tripulante = pathCompleto(temporal);
    free(temp2);
    free(temporal);
    
    while(access(path_tripulante,F_OK) >= 0){
        t_config* metadata = config_create(path_tripulante);
        char** listaBloques = config_get_array_value(metadata,"BLOCKS");
        int contador = 0;

        while(listaBloques[contador]){ 
            contador++;
        }

        for(int i = 0; i < contador; i++){
            int posicion = atoi(listaBloques[i]);
            if(encontroVacio){
                bitarray_clean_bit(bitmapFalso,posicion);
                memcpy(copiaSuperBloque+sizeof(int)*2,bitmapFalso->bitarray,cantidadBloques/8);
            }else{
                bitarray_set_bit(bitmapFalso,posicion);
                memcpy(copiaSuperBloque+sizeof(int)*2,bitmapFalso->bitarray,cantidadBloques/8);
            }
        }
        for(int i = 0; i < contador; i++){
            free(listaBloques[contador]);
        }
        free(listaBloques);
        config_destroy(metadata);

        idTripulante++;
        free(path_tripulante);
        
        temporal = string_new();
        string_append(&temporal, "Bitacora/");
        temp2 = crearStrTripulante(idTripulante);
        string_append(&temporal,temp2);

        path_tripulante = pathCompleto(temporal);
        free(temp2);
        free(temporal);
    }
    free(path_tripulante);

}
void corregirBitmapRecursos(int encontroVacio,t_bitarray* bitmapFalso, void* copiaSuperBloque){
    log_info(logger,"-----------------------------------------------------");
    log_info(logger, "Correccion Bitmap: Recursos");
    
    char* path_oxigeno = pathCompleto("Files/Oxigeno.ims");
    char* path_comida = pathCompleto("Files/Comida.ims");
    char* path_basura = pathCompleto("Files/Basura.ims");
    
    if(access(path_oxigeno,F_OK) >= 0){
        log_info(logger, "Correccion Bitmap: Oxigeno");

        t_config* metadata = config_create(path_oxigeno);
        char** bloquesOxigeno = config_get_array_value(metadata,"BLOCKS");
        int contador = 0;

        while(bloquesOxigeno[contador]){ 
            contador++;
        }

        for(int i = 0; i < contador; i++){
            int posicion = atoi(bloquesOxigeno[i]);
            if(encontroVacio){
                bitarray_clean_bit(bitmapFalso,posicion);
                memcpy(copiaSuperBloque+sizeof(int)*2,bitmapFalso->bitarray,cantidadBloques/8);
            }else{
                bitarray_set_bit(bitmapFalso,posicion);
                memcpy(copiaSuperBloque+sizeof(int)*2,bitmapFalso->bitarray,cantidadBloques/8);
            }
        }

        for(int i = 0; i <= contador; i++){
            free(bloquesOxigeno[contador]);
        }
        free(bloquesOxigeno);
        config_destroy(metadata);

    }else if(access(path_comida,F_OK) >= 0){
        log_info(logger, "Correccion _Bitmap: Comida");
        t_config* metadata1 = config_create(path_comida);
        char** listaBloques2 = config_get_array_value(metadata1,"BLOCKS");
        int contador = 0;

        while(listaBloques2[contador]){ 
            contador++;
        }

        for(int i = 0; i < contador; i++){
            int posicion = atoi(listaBloques2[i]);
            if(encontroVacio){
                bitarray_clean_bit(bitmapFalso,posicion);
                memcpy(copiaSuperBloque+sizeof(int)*2,bitmapFalso->bitarray,cantidadBloques/8);
            }else{
                bitarray_set_bit(bitmapFalso,posicion);
                memcpy(copiaSuperBloque+sizeof(int)*2,bitmapFalso->bitarray,cantidadBloques/8);
            }
        }

        for(int i = 0; i <= contador; i++){
            free(listaBloques2[contador]);
        }
        free(listaBloques2);
        config_destroy(metadata1);

    }else if(access(path_basura,F_OK) >= 0){
        log_info(logger, "Correccion _Bitmap: Basura");
        t_config* metadata2 = config_create(path_basura);
        char** listaBloques3 = config_get_array_value(metadata2,"BLOCKS");
        int contador = 0;

        while(listaBloques3[contador]){ 
            contador++;
        }
        for(int i = 0; i < contador; i++){
            int posicion = atoi(listaBloques3[i]);
            if(encontroVacio){
                bitarray_clean_bit(bitmapFalso,posicion);
                memcpy(copiaSuperBloque+sizeof(int)*2,bitmapFalso->bitarray,cantidadBloques/8);
            }else{
                bitarray_set_bit(bitmapFalso,posicion);
                memcpy(copiaSuperBloque+sizeof(int)*2,bitmapFalso->bitarray,cantidadBloques/8);
            }
        }

        for(int i = 0; i <= contador; i++){
            free(listaBloques3[contador]);
        }
        free(listaBloques3);
        config_destroy(metadata2);
    }

    free(path_oxigeno);
    free(path_comida);
    free(path_basura);

}

void validarSizeFile(){
    char* path_oxigeno = pathCompleto("Files/Oxigeno.ims");
    char* path_comida = pathCompleto("Files/Comida.ims");
    char* path_basura = pathCompleto("Files/Basura.ims");
    
    log_info(logger,"---------------------------------------------");
    log_info(logger,"Validando los Sizes de los Files");

    validarSizeRecurso(path_oxigeno);
    validarSizeRecurso(path_comida);
    validarSizeRecurso(path_basura);
    log_info(logger,"---------------------------------------------");


    free(path_basura);
    free(path_comida);
    free(path_oxigeno);
}

void validarSizeRecurso(char* path){
    
    if(access(path,F_OK) >= 0){
        char* recurso = queRecurso(path);
        log_info(logger, "Validando Size del Recurso:  %s",recurso);
        free(recurso);
        t_config* metadata = config_create(path);
        int size = config_get_int_value(metadata,"SIZE");
        char** listaBloques = config_get_array_value(metadata,"BLOCKS");

        int contador = 0;
        int sizeTemp = 0;
        int cantidadBloquesUsados = 0;

        while(listaBloques[contador]){ 
            contador++;
        }

        for(int i = 0; i < contador; i++){
            int posicionLista = atoi(listaBloques[i]);
            int bloqueTemp = contador-cantidadBloquesUsados;
            
            if(bloqueTemp != 1){
                sizeTemp += tamanioBloque;
                cantidadBloquesUsados++;
            }else{
                char* _stringTemp= malloc(tamanioBloque + 1);
                memcpy(_stringTemp, copiaBlocks + posicionLista*tamanioBloque, tamanioBloque);
                _stringTemp[tamanioBloque] = '\0';
                
                int cont_charLlenos = 0;
                int contador2 = 0;

                while (contador2 < tamanioBloque) {
                    if (memcmp(_stringTemp + contador2, " ", 1)) {
                        cont_charLlenos++;
                    }
                    contador2++;
                }

                sizeTemp += cont_charLlenos;
                // log_info(logger, "Size final:%d", sizeTemp);
                free(_stringTemp);

                if(sizeTemp != size){
                    log_info(logger, "Los sizes son distintos. Se repara valor en metadata");
                    char* temporal = string_itoa(sizeTemp);
                    config_set_value(metadata,"SIZE",temporal);
                    config_save(metadata);
                    free(temporal);
                }else{
                    log_info(logger, "Validacion de Size..OK");
                }

                cantidadBloquesUsados++;
            }
        }

        config_destroy(metadata);
        for(int i = 0; i < contador; i++){
            free(listaBloques[i]);
        }
        free(listaBloques);

    }
}

void validarBlocksBlockCount(){
    char* path_oxigeno = pathCompleto("Files/Oxigeno.ims");
    char* path_comida = pathCompleto("Files/Comida.ims");
    char* path_basura = pathCompleto("Files/Basura.ims");

    log_info(logger,"---------------------------------------------");
    log_info(logger,"Validando los Blocks+BlockCount de los Files");

    validarBlockCountRecurso(path_oxigeno);
    validarBlockCountRecurso(path_comida);
    validarBlockCountRecurso(path_basura);
    log_info(logger,"---------------------------------------------");

    free(path_comida);
    free(path_basura);
    free(path_oxigeno);
}

void validarBlockCountRecurso(char* path){
    if(access(path,F_OK) >= 0){
        char* recurso = queRecurso(path);
        log_info(logger, "Validando Block-Block_Count del Recurso:  %s",recurso);
        free(recurso);

        t_config* metadata = config_create(path);
        char** listaBloques = config_get_array_value(metadata,"BLOCKS");
        int cantidadBloquesOxigeno = config_get_int_value(metadata,"BLOCK_COUNT");
        int contador = 0;

        while(listaBloques[contador]){ 
            contador++;
        }

        if(contador != cantidadBloquesOxigeno){
            log_info(logger, "Comienza reparacion de Blocks y BlockCount");
            char* temporal = string_itoa(contador);
            config_set_value(metadata, "BLOCK_COUNT", temporal);
            config_save(metadata);
            free(temporal);
        }else{
            log_info(logger, "Validacion BlockCount y Blocks...OK");
        }

        config_destroy(metadata);
        for(int i = 0; i < contador; i++){
            free(listaBloques[i]);
        }
        free(listaBloques);
    }
}


void validacionBlocks(){
    char* path_oxigeno = pathCompleto("Files/Oxigeno.ims");
    char* path_comida = pathCompleto("Files/Comida.ims");
    char* path_basura = pathCompleto("Files/Basura.ims");

    log_info(logger,"---------------------------------------------");
    log_info(logger,"Validando los Blocks de los Files");
    validarBlocksRecursos(path_oxigeno);
    validarBlocksRecursos(path_comida);
    validarBlocksRecursos(path_basura);
    log_info(logger,"---------------------------------------------");

    free(path_oxigeno);
    free(path_comida);
    free(path_basura);
}

void validarBlocksRecursos(char* path){
    char* recurso = queRecurso(path);
    log_info(logger, "Validando Blocks del Recurso:  %s",recurso);
    free(recurso);
    
    int archBloques = open("./Filesystem/Blocks.ims", O_CREAT | O_RDWR, 0664);
    void* falsoMemory = mmap(NULL, tamanioBloque*cantidadBloques, PROT_READ | PROT_WRITE, MAP_SHARED, archBloques, 0);
    
    void* copiaSB2 = malloc(tamanioBloque*cantidadBloques);
    memcpy(copiaSB2,falsoMemory,tamanioBloque * cantidadBloques);
    munmap(falsoMemory, tamanioBloque * cantidadBloques);

    if(access(path,F_OK) == 0){
        t_config* metadata = config_create(path);
        char** listaBloques = config_get_array_value(metadata,"BLOCKS");
        char* md5 = config_get_string_value(metadata,"MD5");
        int size = config_get_int_value(metadata,"SIZE");
        char* caracterLlenado = config_get_string_value(metadata,"CARACTER_LLENADO");
        char charPegar = caracterLlenado[0];
        int contador = 0;
        int bloquesHastaAhora = 0;
        int bloque;
        char* string_temp = string_new();

        while(listaBloques[contador]){ 
            contador++;
        }

        char* md5Temporal = malloc(32 + 1); //32 + \0  
        for(int i = 0; i < contador; i++){
            int contadorTamanio = 0;
            char* temporal;
            bloque = atoi(listaBloques[bloquesHastaAhora]);
            
            while(contadorTamanio != tamanioBloque){
                temporal = malloc(2);
                memcpy(temporal, copiaSB2 + bloque * tamanioBloque + contadorTamanio, 1);
                temporal[1] = '\0';

                string_append(&string_temp,temporal);
                free(temporal);
                contadorTamanio++;  
            }     
            bloquesHastaAhora++;
        }
        int tamanioStringTemporal = string_length(string_temp);
    
        //Borro los ultimos espacios
        while(string_temp[tamanioStringTemporal - 1] == ' '){
            tamanioStringTemporal--;
        }
        string_temp[tamanioStringTemporal] = '\0';

        //creo el nuevo MD5 
        FILE* archivo = fopen("temporal.txt","w");
        fprintf(archivo,"%s",string_temp);
        fclose(archivo);
            
        char* comando = string_new();
        string_append(&comando, "md5sum temporal.txt > resultado.txt");
        system(comando);   
        free(comando);

        FILE* archivo2 = fopen("resultado.txt","r");
        fscanf(archivo2,"%s",md5Temporal);
        md5Temporal[32] = '\0';
        fclose(archivo2);

        int err = remove("temporal.txt");
        if (err < 0){
            log_error(logger, "Error al remover archivo temporal.txt");
        }
        
        err = remove("resultado.txt");
        if(err < 0 ){
            log_error(logger, "Error al remover archivo resultado.txt");
        }

        log_info(logger, "Comparando MD5: %s - %s", md5, md5Temporal);
        if(tamanioStringTemporal == size && !strcmp(md5, md5Temporal)){
            log_info(logger, "Validacion Blocks ....Todo OK");
        }else{
            log_info(logger, "Validacion Blocks...NOT OK");
           
            bloquesHastaAhora = 0;
            int sizeTemporal = size;
            

            for(int i = 0; i < contador; i++){
                bloque = atoi(listaBloques[i]);
                if((contador - bloquesHastaAhora) > 1){ //no es el ultimo bloque-->no hay frag. interna
                    memset(copiaSB2 + bloque * tamanioBloque,charPegar,tamanioBloque);
                    sizeTemporal -= tamanioBloque;
                    bloquesHastaAhora++;
                }else{
                    memset(copiaSB2 + bloque * tamanioBloque ,' ',tamanioBloque);
                    memset(copiaSB2 + bloque * tamanioBloque,charPegar,sizeTemporal);
                    bloquesHastaAhora++;
                }
            }
            log_info(logger, "Se Restauró Blocks de la metadata.");
            falsoMemory = mmap(NULL, tamanioBloque*cantidadBloques, PROT_READ | PROT_WRITE, MAP_SHARED, archBloques, 0);
            memcpy(falsoMemory,copiaSB2,tamanioBloque * cantidadBloques);
            msync(falsoMemory, tamanioBloque*cantidadBloques,MS_SYNC);
            munmap(falsoMemory, tamanioBloque * cantidadBloques);
            

        }
        config_destroy(metadata);
        free(string_temp);
        for(int i = 0; i < contador; i++){
            free(listaBloques[i]);
        }
        free(listaBloques);
        free(md5Temporal);
        

    }
    free(copiaSB2);
    close(archBloques);   
}


char* pathCompleto(char* strConcatenar){
    return string_from_format("%s/%s",datosConfig->puntoMontaje,strConcatenar);
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

char* queRecurso(char* path){
    char* pathTemp = pathCompleto("Files/Oxigeno.ims");
    char* recurso = string_new();
    if(!strcmp(path,pathTemp)){
        string_append(&recurso,"Oxigeno");
        free(pathTemp);
        return recurso;
    }else{
        free(pathTemp);
        pathTemp = pathCompleto("Files/Comida.ims");
        if(!strcmp(path,pathTemp)){
            string_append(&recurso,"Comida");
            free(pathTemp);
            return recurso;
        }else{
            free(pathTemp);
            pathTemp = pathCompleto("Files/Basura.ims");
            if(!strcmp(path,pathTemp)){
                string_append(&recurso,"Basura");
                free(pathTemp);
                return recurso;
            }
            free(pathTemp);
            return "NoName";
        }
    }
    
    

    
}