    #include "./headers/sabotajes.h"

    void sabotaje(){
        char** strPosiciones = string_split(posicionesSabotajes[contadorSabotajeLeido],"|");
        int posicionX = atoi(strPosiciones[0]);
        int posicionY = atoi(strPosiciones[1]);

        // pthread_mutex_lock(&discordiador);
        // int idDiscordiador = _connect(ipDiscordiador,puertoDiscordiador,logger);
        // void* buffer = _serialize(sizeof(int)*2,"%d%d",posicionX,posicionY);
        // _send_message(idDiscordiador,"IMS",COMIENZA_SABOTAJE,buffer,sizeof(int)*2,logger);
        // pthread_mutex_unlock(&discordiador);

        protocolo_fsck();
        contadorSabotajeLeido++;
        // free(buffer);
        free(strPosiciones[0]);
        free(strPosiciones[1]);
        free(strPosiciones);

        // log_info(logger, "Envié mensaje de sabotaje a discordiador");
    
    }

    void protocolo_fsck(){
        log_info(logger,"-------------------------------------------------");
        log_info(logger, "Comenzando Protocolo:FSCK...");
        pthread_mutex_lock(&blocks_bitmap);
        validacionSuperBloque();
        validacionFiles();
        pthread_mutex_unlock(&blocks_bitmap);
        log_info(logger, "Finalizando Protocolo:FSCK..");
        log_info(logger,"-------------------------------------------------");
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
        int archSB = open("./Filesystem/SuperBloque.ims", O_CREAT | O_RDWR, 0664);
        void* superBloqueTemp = mmap(NULL, sizeof(uint32_t)*2, PROT_READ | PROT_WRITE, MAP_SHARED, archSB, 0);
        
        uint32_t cantidadBloquesDisco;
        memcpy(&cantidadBloquesDisco, superBloqueTemp + sizeof(uint32_t), sizeof(uint32_t));
        munmap(superBloqueTemp,sizeof(uint32_t)*2);

        if(cantidadBloques == cantidadBloquesDisco){
            log_info(logger,"Cantidad de bloques... OK");
        }else{
            log_info(logger, "Cantidad de Bloques... NOT OK");
            log_info(logger,"Reparación: Se sobreescribe la cantidadBloques en sistema con la del disco");

            cantidadBloques = cantidadBloquesDisco;
            log_info(logger, "Finalizó reparación de cantidadBloques");
        }
        close(archSB);
    }
    void validarBitmapSabotaje(){

        for(int i = 0; i< cantidadBloques; i++){
        
            int testBitmap = bitarray_test_bit(bitmap,i);
            char* strTestear = malloc(3);

            if(testBitmap){//Hay algo en bitmap
                memcpy(strTestear, copiaBlocks + i* tamanioBloque,2);
                strTestear[2] ='\0';

                if(!strcmp(strTestear,"  ")){
                    log_info(logger,"Error en el bloque:%d. Bloque realmente vacio. Se cambia en bitmap",i);
                    corregirBitmap(1);
                }
                free(strTestear);

            }else if(testBitmap == 0){ 
                memcpy(strTestear, copiaBlocks + i* tamanioBloque,2);   
                strTestear[2] ='\0';

                if(strcmp(strTestear,"  ")){
                    log_info(logger, "Error en el bloque:%d. Bloque no esta vacio. Se cambia en bitmap",i);
                    corregirBitmap(0);
                }
                free(strTestear);

            }
        }
        log_info(logger,"Validacion Bitmap... OK");
    }
    void corregirBitmap(int encontroVacio){
        corregirBitmapTripulantes(encontroVacio);
        corregirBitmapRecursos(encontroVacio);
    }
    void corregirBitmapTripulantes(int encontroVacio){
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
            t_config* metadata = config_create(pathCompleto(path_tripulante));
            char** listaBloques = config_get_array_value(metadata,"BLOCKS");
            int contador = 0;

            while(listaBloques[contador]){ 
                contador++;
            }

            for(int i = 0; i <= contador; i++){
                int posicion = atoi(listaBloques[i]);
                if(encontroVacio){
                    bitarray_clean_bit(bitmap,posicion);
                    memcpy(copiaSB+sizeof(int)*2,bitmap->bitarray,cantidadBloques/8);
                }else{
                    bitarray_set_bit(bitmap,posicion);
                    memcpy(copiaSB+sizeof(int)*2,bitmap->bitarray,cantidadBloques/8);
                }
            }

            config_destroy(metadata);
            for(int i = 0; i <= contador; i++){
                free(listaBloques[contador]);
            }
            free(listaBloques);

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

    }
    void corregirBitmapRecursos(int encontroVacio){
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
                    bitarray_clean_bit(bitmap,posicion);
                    memcpy(copiaSB+sizeof(int)*2,bitmap->bitarray,cantidadBloques/8);
                }else{
                    bitarray_set_bit(bitmap,posicion);
                    memcpy(copiaSB+sizeof(int)*2,bitmap->bitarray,cantidadBloques/8);
                }
            }

            config_destroy(metadata);
            for(int i = 0; i <= contador; i++){
                free(bloquesOxigeno[contador]);
            }
            free(bloquesOxigeno);

        }else if(access(path_comida,F_OK) >= 0){
            log_info(logger, "Correccion Bitmap: Comida");
            t_config* metadata1 = config_create(path_comida);
            char** listaBloques2 = config_get_array_value(metadata1,"BLOCKS");
            int contador = 0;

            while(listaBloques2[contador]){ 
                contador++;
            }

            for(int i = 0; i < contador; i++){
                int posicion = atoi(listaBloques2[i]);
                if(encontroVacio){
                    bitarray_clean_bit(bitmap,posicion);
                    memcpy(copiaSB+sizeof(int)*2,bitmap->bitarray,cantidadBloques/8);
                }else{
                    bitarray_set_bit(bitmap,posicion);
                    memcpy(copiaSB+sizeof(int)*2,bitmap->bitarray,cantidadBloques/8);
                }
            }

            config_destroy(metadata1);
            for(int i = 0; i <= contador; i++){
                free(listaBloques2[contador]);
            }
            free(listaBloques2);

        }else if(access(path_basura,F_OK) >= 0){
            log_info(logger, "Correccion Bitmap: Basura");
            t_config* metadata2 = config_create(path_basura);
            char** listaBloques3 = config_get_array_value(metadata2,"BLOCKS");
            int contador = 0;

            while(listaBloques3[contador]){ 
                contador++;
            }
            for(int i = 0; i < contador; i++){
                int posicion = atoi(listaBloques3[i]);
                if(encontroVacio){
                    bitarray_clean_bit(bitmap,posicion);
                    memcpy(copiaSB+sizeof(int)*2,bitmap->bitarray,cantidadBloques/8);
                }else{
                    bitarray_set_bit(bitmap,posicion);
                    memcpy(copiaSB+sizeof(int)*2,bitmap->bitarray,cantidadBloques/8);
                }
            }

            config_destroy(metadata2);
            for(int i = 0; i <= contador; i++){
                free(listaBloques3[contador]);
            }
            free(listaBloques3);
        }

        free(path_oxigeno);
        free(path_comida);
        free(path_basura);

    }

    void validarSizeFile(){
        char* path_oxigeno = pathCompleto("Files/Oxigeno.ims");
        char* path_comida = pathCompleto("Files/Comida.ims");
        char* path_basura = pathCompleto("Files/Basura.ims");
        
        validarSizeRecurso(path_oxigeno);
        validarSizeRecurso(path_comida);
        validarSizeRecurso(path_basura);

        free(path_basura);
        free(path_comida);
        free(path_oxigeno);
    }

    void validarSizeRecurso(char* path){
        
        if(access(path,F_OK) >= 0){

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
                    log_info(logger, "Size final:%d", sizeTemp);
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
            for(int i = 0; i <= contador; i++){
                free(listaBloques[i]);
            }
            free(listaBloques);

        }
    }

    void validarBlocksBlockCount(){
        char* path_oxigeno = pathCompleto("Files/Oxigeno.ims");
        char* path_comida = pathCompleto("Files/Comida.ims");
        char* path_basura = pathCompleto("Files/Basura.ims");


        validarBlockCountRecurso(path_oxigeno);
        validarBlockCountRecurso(path_comida);
        validarBlockCountRecurso(path_basura);

        free(path_comida);
        free(path_basura);
        free(path_oxigeno);
    }

    void validarBlockCountRecurso(char* path){
        if(access(path,F_OK) >= 0){
            log_info(logger, "Validacion Block-Block_Count:%s",path);

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
            for(int i = 0; i <= contador; i++){
                free(listaBloques[i]);
            }
            free(listaBloques);
        }
    }


    void validacionBlocks(){
        char* path_oxigeno = pathCompleto("Files/Oxigeno.ims");
        char* path_comida = pathCompleto("Files/Comida.ims");
        char* path_basura = pathCompleto("Files/Basura.ims");

        validarBlocksRecursos(path_oxigeno);
        validarBlocksRecursos(path_comida);
        validarBlocksRecursos(path_basura);

        free(path_oxigeno);
        free(path_comida);
        free(path_basura);
    }

    void validarBlocksRecursos(char* path){
        if(access(path,F_OK) >= 0){

            t_config* metadata = config_create(path);
            char** listaBloques = config_get_array_value(metadata,"BLOCKS");
            char* md5 = config_get_string_value(metadata,"MD5");
            int size = config_get_int_value(metadata,"SIZE");
            char* caracterLlenado = config_get_string_value(metadata,"CARACTER_LLENADO");
            int contador = 0;
            int bloquesHastaAhora = 0;
            int bloque;
            char* string_temp = string_new();

            while(listaBloques[contador]){ 
                contador++;
            }

            char* md5Temporal = malloc(32 + 1); //32 + \0   

            for(int i = 0; i < contador; i++){
                if((contador - bloquesHastaAhora) > 1){ //no es el ultimo bloque-->no hay frag. interna
                
                    bloque = atoi(listaBloques[bloquesHastaAhora]);
                    char* temporalBloque = malloc(tamanioBloque+1);
                    memcpy(temporalBloque, copiaBlocks + bloque*tamanioBloque, tamanioBloque);
                    temporalBloque[tamanioBloque] = '\0';
                        
                    string_append(&string_temp,temporalBloque);
                    bloquesHastaAhora++;
                    free(temporalBloque);
                }else{
                    bloque = atoi(listaBloques[bloquesHastaAhora]);
                    int fragmentacion = contador*tamanioBloque - size;

                    char* temporalBloque = malloc(fragmentacion+1);
                    memcpy(temporalBloque, copiaBlocks + bloque*tamanioBloque, fragmentacion);
                    temporalBloque[fragmentacion] = '\0';
                        
                    string_append(&string_temp,temporalBloque);
                    free(temporalBloque);
                }
            }
            
                
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

            // int err = remove("temporal.txt");
            // if (err < 0){
            //     log_error(logger, "Error al remover archivo temporal.txt");
            // }
            
            // err = remove("resultado.txt");
            // if(err < 0 ){
            //     log_error(logger, "Error al remover archivo resultado.txt");
            // }

            log_info(logger, "Comparando MD5: %s - %s", md5, md5Temporal);

            if(!strcmp(md5, md5Temporal)){
                log_info(logger, "Validacion Blocks...OK [MD5 iguales]");
            }else{  
                log_info(logger, "Validacion Blocks...NOT OK [MD5 no son iguales]");
                bloquesHastaAhora = 0;
                int sizeTemporal = size;
                for(int i = 0; i < contador; i++){
                    bloque = atoi(listaBloques[i]);
                    if((contador - bloquesHastaAhora) > 1){ //no es el ultimo bloque-->no hay frag. interna
                        memset(copiaBlocks + bloque * tamanioBloque ,caracterLlenado[0],tamanioBloque);
                        sizeTemporal -= tamanioBloque;
                        bloquesHastaAhora++;
                    }else{
                        memset(copiaBlocks + bloque * tamanioBloque ,' ',tamanioBloque);
                        memset(copiaBlocks + bloque * tamanioBloque ,caracterLlenado[0],sizeTemporal);
                        bloquesHastaAhora++;
                    }
                }

                // config_set_value(metadata,"MD5",md5Temporal);
                log_info(logger, "Se Restauró Blocks de la metadata.");

            }

            config_destroy(metadata);
            free(string_temp);
            for(int i = 0; i <= contador; i++){
                free(listaBloques[i]);
            }
            free(listaBloques);
            free(md5Temporal);
        
            
        }
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