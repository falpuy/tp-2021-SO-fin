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
    validacionEsValidaLista();
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
    validacionBlocks();
    validarSizeFile();
    validarBlocksBlockCount();
}

void validarCantidadBloques(){
    log_info(logger,"---------------------------------------------------------");
    log_info(logger, "Validando Cantidad de Bloques....");
    int archSB = open("./Filesystem/SuperBloque.ims", O_CREAT | O_RDWR, 0664);
    sb_memoria = mmap(NULL, sizeof(uint32_t)*2, PROT_READ | PROT_WRITE, MAP_SHARED, archSB, 0);
    
    uint32_t cantidadBloquesDisco;
    memcpy(&cantidadBloquesDisco, sb_memoria + sizeof(uint32_t), sizeof(uint32_t));

    int sizeBlocks;    
    struct stat st;
    stat("./Filesystem/Blocks.ims", &st);
    sizeBlocks= st.st_size;

    uint32_t cantidadTemporal = (uint32_t) sizeBlocks / tamanioBloque;

    if(cantidadTemporal == cantidadBloquesDisco){
        log_info(logger,"Cantidad de bloques... OK");
    }else{
        log_info(logger, "Cantidad de Bloques... NOT OK.");
        log_info(logger,"Reparación: Se sobreescribe la cantidadBloques");

        cantidadBloques = cantidadTemporal;
        // superBloqueTemp = mmap(NULL, sizeof(uint32_t)*2 + cantidadBloques/8, PROT_READ | PROT_WRITE, MAP_SHARED, archSB, 0);
        memcpy(sb_memoria + sizeof(uint32_t), &cantidadBloques, sizeof(uint32_t));
        msync(sb_memoria, sizeof(uint32_t)*2 + cantidadBloques/8, MS_SYNC);
        log_info(logger, "Finalizó reparación de cantidadBloques");
    }
    close(archSB);
    munmap(sb_memoria,sizeof(uint32_t)*2);
    log_info(logger,"---------------------------------------------------------");
    
}

void validacionEsValidaLista(){
    char* path_oxigeno = pathCompleto("Files/Oxigeno.ims");
    char* path_comida = pathCompleto("Files/Comida.ims");
    char* path_basura = pathCompleto("Files/Basura.ims");
    
    log_info(logger,"---------------------------------------------");
    log_info(logger,"Validando si las listas son de los Files son validas");

    validacionEsValidaListaRecursos(path_oxigeno);
    validacionEsValidaListaRecursos(path_comida);
    validacionEsValidaListaRecursos(path_basura);
    log_info(logger,"---------------------------------------------");


    free(path_basura);
    free(path_comida);
    free(path_oxigeno);
}


void validacionEsValidaListaRecursos(char* path){

    int archSuperBloque = open("./Filesystem/SuperBloque.ims", O_CREAT | O_RDWR, 0664);    
    sb_memoria = mmap(NULL, sizeof(uint32_t) * 2 + cantidadBloques/8, PROT_READ | PROT_WRITE, MAP_SHARED, archSuperBloque, 0);
    memBitmap = malloc(cantidadBloques/8);
    memcpy(memBitmap, sb_memoria + sizeof(uint32_t)*2,cantidadBloques/8);
    bitmap = bitarray_create_with_mode((char*)memBitmap, cantidadBloques / 8, MSB_FIRST);  

    if(access(path,F_OK) >= 0){
        log_info(logger, "Existe archivo metadata recursos");
        
        t_config* metadata = config_create(path);
        char** listaBloques = config_get_array_value(metadata,"BLOCKS");
        config_destroy(metadata);

        int contador = 0;
        char* temporal = string_new();
        string_append(&temporal,"[]");
        char* lista;

        while(listaBloques[contador]){ 
            contador++;
        }
    
        log_info(logger,"Validando estado de la lista de blocks...");
        int error = 0;
        
        for(int i = 0; i < contador; i++){
            int bloque = atoi(listaBloques[i]);
            log_info(logger,"Bloque:%d", bloque);
            
            if(bloque > cantidadBloques || !bitarray_test_bit(bitmap,bloque)){
                error = 1;
                log_error(logger,"Se encontró un bloque inválido: %d", bloque);
            }else {
                lista = crearNuevaListaBloques(temporal,bloque,1,path);
                // log_info(logger,"Lista:%s", lista);
                free(temporal);
                temporal = string_new();
                string_append(&temporal, lista);
                free(lista);
            }
        }

        if(!error){
            log_info(logger,"No hubo error en la lista de blocks");
        }else{
            metadata = config_create(path);
            log_info(logger,"La lista final:%s", temporal);
            config_set_value(metadata,"BLOCKS",temporal);
            config_save(metadata);
            config_destroy(metadata);
        }
        for(int i = 0; i < contador; i++){
            free(listaBloques[i]);
        }
        free(listaBloques);
        free(temporal);
    }   
    munmap(sb_memoria,sizeof(uint32_t)*2 + cantidadBloques/8);
    free(memBitmap);
    bitarray_destroy(bitmap);
    close(archSuperBloque);
}


void validarBitmapSabotaje(){
    log_info(logger,"---------------------------------------------------------");
    int _superBloque = open("./Filesystem/SuperBloque.ims", O_CREAT | O_RDWR, 0664);
    int _blocks = open("./Filesystem/Blocks.ims", O_CREAT | O_RDWR, 0664);
    sb_memoria = mmap(NULL, sizeof(uint32_t) * 2 + cantidadBloques/8, PROT_READ | PROT_WRITE, MAP_SHARED, _superBloque, 0);
    void* blocksTemp = mmap(NULL,tamanioBloque*cantidadBloques, PROT_READ | PROT_WRITE, MAP_SHARED, _blocks, 0);
    
    memBitmap = malloc(cantidadBloques/8);
    memcpy(memBitmap, sb_memoria + sizeof(uint32_t) * 2, cantidadBloques/8);
    bitmap = bitarray_create_with_mode((char*) memBitmap,cantidadBloques/8,MSB_FIRST);

    void* memBitmapFalso = malloc(cantidadBloques/8);
    t_bitarray* bitmapFalso = bitarray_create_with_mode((char*) memBitmapFalso, cantidadBloques/8,MSB_FIRST);

    log_info(logger, "Validando bitmap...");


    //BITACORAS
    int idTripulante = 0;
    char* temporal = string_new();
    string_append(&temporal, "Bitacoras/");
    char* temp2 = crearStrTripulante(idTripulante);
    string_append(&temporal,temp2);
    char* path_tripulante = pathCompleto(temporal);
    free(temp2);
    free(temporal);
    
    // log_info(logger, "Path del primer tripulante: %s", path_tripulante);
    // log_info(logger, "Bloques de todos los tripulantes");
    while(access(path_tripulante,F_OK) >= 0){
        t_config* metadata = config_create(path_tripulante);
        char** listaBloques = config_get_array_value(metadata,"BLOCKS");
        int contador = 0;
        config_destroy(metadata);

        while(listaBloques[contador]){ 
            contador++;
        }
        // log_info(logger,"El tripulante:%d tiene %d bloques", idTripulante, contador);
        for(int i = 0; i < contador; i++){
            // log_info(logger,"%d", atoi(listaBloques[i]));
            bitarray_set_bit(bitmapFalso,atoi(listaBloques[i]));
        }

        idTripulante++;
        free(path_tripulante);
        temporal = string_new();
        string_append(&temporal, "Bitacoras/");
        temp2 = crearStrTripulante(idTripulante);
        string_append(&temporal,temp2);
        path_tripulante = pathCompleto(temporal);
        free(temp2);
        free(temporal);

        for(int i = 0; i < contador; i++){
            free(listaBloques[i]);
        }
        free(listaBloques);
        
    }
    free(path_tripulante);

    char* path_oxigeno = pathCompleto("Files/Oxigeno.ims");
    char* path_comida = pathCompleto("Files/Comida.ims");
    char* path_basura = pathCompleto("Files/Basura.ims");


    // log_info(logger, "Bloques de Oxigeno.ims");
    if(access(path_oxigeno,F_OK) >= 0){
        t_config* metadata = config_create(path_oxigeno);
        char** listaBloques = config_get_array_value(metadata,"BLOCKS");
        int contador = 0;

        while(listaBloques[contador]){ 
            contador++;
        }
        for(int i = 0; i < contador; i++){
            // log_info(logger,"%d", atoi(listaBloques[i]));
            bitarray_set_bit(bitmapFalso,atoi(listaBloques[i]));
        }
        config_destroy(metadata);
        for(int i = 0; i < contador; i++){
            free(listaBloques[i]);
        }
        free(listaBloques);

    }
    free(path_oxigeno);

    // log_info(logger, "Bloques de Comida.ims");
    if(access(path_comida,F_OK) >= 0){
        t_config* metadata = config_create(path_comida);
        char** listaBloques = config_get_array_value(metadata,"BLOCKS");
        int contador = 0;

        while(listaBloques[contador]){ 
            contador++;
        }
        for(int i = 0; i < contador; i++){
            // // log_info(logger,"%d", atoi(listaBloques[i]));
            bitarray_set_bit(bitmapFalso,atoi(listaBloques[i]));
        }
        for(int i = 0; i < contador; i++){
            free(listaBloques[i]);
        }
        free(listaBloques);
        config_destroy(metadata);
    }
    free(path_comida);
    // log_info(logger, "Bloques de Basura.ims");
    if(access(path_basura,F_OK) >= 0){
        t_config* metadata = config_create(path_basura);
        char** listaBloques = config_get_array_value(metadata,"BLOCKS");
        int contador = 0;

        while(listaBloques[contador]){ 
            contador++;
        }

        for(int i = 0; i < contador; i++){
            // log_info(logger,"%d", atoi(listaBloques[i]));
            bitarray_set_bit(bitmapFalso,atoi(listaBloques[i]));
        }
        config_destroy(metadata);
        for(int i = 0; i < contador; i++){
            free(listaBloques[i]);
        }
        free(listaBloques);
    }
    free(path_basura);

    int errorBitmap = 0;
    for(int i = 0; i < cantidadBloques; i++){
        int bit1 = bitarray_test_bit(bitmap,i);
        int bit2 = bitarray_test_bit(bitmapFalso,i);

        if(bit1!= bit2){
            errorBitmap = 1;
            if(bit1){
                bitarray_clean_bit(bitmap,i);
            }else{
                bitarray_set_bit(bitmap,i);
            }
        }
    }

    memcpy(sb_memoria + sizeof(int) * 2,bitmap->bitarray,cantidadBloques/8);
    if(!errorBitmap){
        log_info(logger,"Validacion Bitmap... OK");
    }else{
        log_error(logger, "Error en el bitmap.. Se corrige");
    }

    msync(sb_memoria, sizeof(uint32_t) * 2 + cantidadBloques/8, MS_SYNC);
    munmap(sb_memoria,sizeof(uint32_t)*2 + cantidadBloques/8);
    munmap(blocksTemp,tamanioBloque*cantidadBloques);

    // free(copiaSuperBloque);

    bitarray_destroy(bitmap);
    free(memBitmap);
    bitarray_destroy(bitmapFalso);
    free(memBitmapFalso);
    close(_superBloque);
    close(_blocks);
    log_info(logger,"---------------------------------------------------------");
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

        int _blocks = open("./Filesystem/Blocks.ims", O_CREAT | O_RDWR, 0664);
        void* blocksTemp = mmap(NULL,tamanioBloque*cantidadBloques, PROT_READ | PROT_WRITE, MAP_SHARED, _blocks, 0);
        void* copiaBlocks2 = malloc(tamanioBloque*cantidadBloques);
        memcpy(copiaBlocks2,blocksTemp, tamanioBloque*cantidadBloques);
        munmap(blocksTemp,tamanioBloque*cantidadBloques);
        close(_blocks);

        int contador = 0;
        int bloque;

        while(listaBloques[contador]){ 
            contador++;
        }

        char* stringTemporal = malloc((contador* tamanioBloque) + 1);
        
        for(int i = 0; i < contador; i++){
            bloque = atoi(listaBloques[i]);
            memcpy(stringTemporal + i*tamanioBloque, copiaBlocks2 + bloque * tamanioBloque, tamanioBloque);
        }
        stringTemporal[contador*tamanioBloque] = '\0';

        log_info(logger, "String levantado entero:%s",stringTemporal);
        char** stringPartido = string_split(stringTemporal,"|");
        free(stringTemporal);
        char* stringFinal = string_new();
        string_append(&stringFinal,stringPartido[0]);
        string_append(&stringFinal,"|");
        
        for(int i = 0; stringPartido[i] != NULL; i++){
            free(stringPartido[i]);
        }
        free(stringPartido);
        log_info(logger, "String partido:%s", stringFinal);
        int sizeTemp = string_length(stringFinal) - 1;

        log_info(logger, "Size temp:%d", sizeTemp);
        log_info(logger, "Size:%d", size);

        if(sizeTemp != size){
            log_info(logger, "Los sizes son distintos. Se repara valor en metadata");
            char* temporal = string_itoa(sizeTemp);
            config_set_value(metadata,"SIZE",temporal);
            config_save(metadata);
            free(temporal);
        }else{
            log_info(logger, "Validacion de Size..OK");
        }
        free(copiaBlocks2);
        free(stringFinal);
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
    int archSuperBloque = open("./Filesystem/SuperBloque.ims", O_CREAT | O_RDWR, 0664);
    
    sb_memoria = mmap(NULL, sizeof(uint32_t) * 2 + cantidadBloques/8, PROT_READ | PROT_WRITE, MAP_SHARED, archSuperBloque, 0);
    void* falsoFS = mmap(NULL, tamanioBloque*cantidadBloques, PROT_READ | PROT_WRITE, MAP_SHARED, archBloques, 0);
    
    memBitmap = malloc(cantidadBloques/8);
    memcpy(memBitmap, sb_memoria + sizeof(uint32_t)*2,cantidadBloques/8);
    bitmap = bitarray_create_with_mode((char*)memBitmap, cantidadBloques / 8, MSB_FIRST);  
    
    //**********************************VALIDANDO DESORDEN DE LISTA********************************************
    if(access(path,F_OK) >= 0){
        log_info(logger, "Validando si los bloques estan desordenados..");
        
        char* md5Temporal = malloc(32 + 1); //32 + \0
        t_config* metadata = config_create(path);
        char** listaBloques = config_get_array_value(metadata,"BLOCKS");
        char* _md5 = config_get_string_value(metadata,"MD5");
        char* md5 = string_new();
        string_append(&md5,_md5);
        char* charLlenado = config_get_string_value(metadata,"CARACTER_LLENADO");
        char charPegar = charLlenado[0];
        int size = config_get_int_value(metadata,"SIZE");
        log_info(logger,"Size:%d", size);
        config_destroy(metadata);

        int bloque;
        int bloquesHastaAhora;

        int contador = 0;

        while(listaBloques[contador]){ 
            contador++;
        }

        char* stringTemporal = malloc((contador* tamanioBloque) + 1);
        
        for(int i = 0; i < contador; i++){
            bloque = atoi(listaBloques[i]);
            memcpy(stringTemporal + i*tamanioBloque, falsoFS + bloque * tamanioBloque, tamanioBloque);
        }
        stringTemporal[contador*tamanioBloque] = '\0';

        log_info(logger, "String levantado entero:%s",stringTemporal);
        char** stringPartido = string_split(stringTemporal,"|");
        free(stringTemporal);
        char* stringFinal = string_new();
        string_append(&stringFinal,stringPartido[0]);
        string_append(&stringFinal,"|");
        
        for(int i = 0; stringPartido[i] != NULL; i++){
            free(stringPartido[i]);
        }
        free(stringPartido);
        log_info(logger, "String partido:%s", stringFinal);


        //creo el nuevo MD5 
        FILE* archivo = fopen("temporal.txt","w");
        fprintf(archivo,"%s",stringFinal);
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

        log_info(logger, "Comparando MD5: %s", md5);        
        log_info(logger, "Comparando MD5 Temporal: %s", md5Temporal);

        free(stringFinal);

        if(!strcmp(md5, md5Temporal)){
            log_info(logger, "Validacion Blocks ....Todo OK");
        }else{
            log_info(logger, "Validacion Blocks...NOT OK");

            char* temporal = string_repeat(charPegar,size);
            int tamanioTemporal = string_length(temporal);
            string_append(&temporal,"|");
            log_info(logger, "Pegando nuevo string...%s", temporal);
            int bloqueLeido = 0;

            t_config* metadata2 = config_create(path);
            char** lista2 = config_get_array_value(metadata2,"BLOCKS");
            config_destroy(metadata2);
            int contador2=0;

            while(lista2[contador2]){
                contador2++;
            }


            log_info(logger, "Muestro la puta lista");
            for(int i = 0; i < contador2; i++){
                log_info(logger, "El bloque es:%d", atoi(lista2[i]));
            }

            for(int i = 0; i < contador2; i++){
                int bloque = atoi(lista2[i]);
                log_info(logger,"Se va a pegar en el bloque:%d",bloque);
                if(tamanioTemporal >= tamanioBloque){
                    log_info(logger,"No soy ultimo bloque");
                    
                    memcpy(falsoFS + tamanioBloque*bloque,temporal + bloqueLeido*tamanioBloque,tamanioBloque);
                    bloqueLeido++;
                    tamanioTemporal -= tamanioBloque;
                }else{
                    log_info(logger,"soy ultimo bloque");                    
                    memcpy(falsoFS + tamanioBloque*bloque,temporal + bloqueLeido*tamanioBloque,tamanioTemporal +1);
                    tamanioTemporal = 0;
                    bloqueLeido++;
                }
            }
            msync(falsoFS, tamanioBloque*cantidadBloques,MS_SYNC);
            memcpy(copiaBlocks,falsoFS,tamanioBloque*cantidadBloques);
            free(temporal);
            for(int i = 0; i < contador2; i++){
                free(lista2[i]);
            }
            free(lista2);

        }

        for(int i = 0; i < contador; i++){
            free(listaBloques[i]);
        }
        free(listaBloques);
        free(md5Temporal);
        free(md5);
    }
    free(memBitmap);
    bitarray_destroy(bitmap);
    munmap(sb_memoria, sizeof(uint32_t) * 2 + cantidadBloques/8);
    munmap(falsoFS, tamanioBloque * cantidadBloques);
    // free(copiaBlocks2);
    close(archSuperBloque);
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

int cantidad_bloques(char* string){
    double cantidad;
    double tamanioString = string_length(string);
    cantidad = tamanioString / tamanioBloque;
    int valorFinal = (int) ceil(cantidad);

    return valorFinal; //round up
}

void guardarPorBloque(char* stringGuardar,int posEnString, int cantidadBloquesAUsar,char* path,int esRecurso, int flagEsGuardar){

    int cantidadBloquesUsados = 0;
    int tamanioString = string_length(stringGuardar);

    //Mapeo ahora para sacar el bitmap tmb
    char* pathSuperBloque = pathCompleto("SuperBloque.ims");
    int superBloque = open(pathSuperBloque, O_CREAT | O_RDWR, 0664);
    sb_memoria = mmap(NULL, sizeof(uint32_t) * 2 + cantidadBloques/8 , PROT_READ | PROT_WRITE, MAP_SHARED, superBloque, 0);
    memBitmap = malloc(cantidadBloques/8);
    memcpy(memBitmap, sb_memoria + sizeof(uint32_t)*2, cantidadBloques/8);
    bitmap = bitarray_create_with_mode((char*)memBitmap, cantidadBloques / 8, MSB_FIRST);  

    for(int i=0; i < bitarray_get_max_bit(bitmap) && cantidadBloquesUsados != cantidadBloquesAUsar; i++){
        if(bitarray_test_bit(bitmap,i) == 0){

            bitarray_set_bit(bitmap,i);
            memcpy(sb_memoria + sizeof(int) * 2,bitmap->bitarray,cantidadBloques/8);

            if((cantidadBloquesAUsar-cantidadBloquesUsados)==1){//ultimo bloque a escribir - posible fragmentación interna 

                log_info(logger, "Se guarda en el bloque.%d", i);
                //Me muevo al bloque en si a guardar | pego en string moviendome hasta donde guarde antes | Pego lo que me queda del string--> tamañoTotalStr - posicionAntEnStr*tamanioBloque
                memcpy(copiaBlocks + i*tamanioBloque,stringGuardar+posEnString*tamanioBloque,tamanioString-posEnString*tamanioBloque);                
                posEnString ++;    

                //--------------------------ACTUALIZO METADATA---------------------------
                t_config* metadata = config_create(path);
                actualizarSize(metadata,tamanioString-((posEnString-1)*tamanioBloque) - 1,flagEsGuardar);
                config_destroy(metadata);

                actualizarBlocks(i,flagEsGuardar,path);

                metadata = config_create(path);
                if(esRecurso){
                    actualizarBlockCount(metadata,flagEsGuardar);
                }
                config_destroy(metadata);
                cantidadBloquesUsados ++;
            }else{
                //Me muevo al bloque en si a guardar | pego en string moviendome hasta donde guarde antes | Pego todo el tamaño del bloque
                memcpy(copiaBlocks + i*tamanioBloque,stringGuardar+posEnString*tamanioBloque,tamanioBloque);
                posEnString ++;

                //--------------------------ACTUALIZO METADATA---------------------------
                t_config* metadata = config_create(path);
                actualizarSize(metadata,tamanioBloque,flagEsGuardar);
                config_destroy(metadata);

                actualizarBlocks(i,flagEsGuardar,path);

                metadata = config_create(path);
                if(esRecurso){
                    actualizarBlockCount(metadata,flagEsGuardar);
                }
                config_destroy(metadata); 

                cantidadBloquesUsados ++;
            }
        }
    }

    int err = munmap(sb_memoria, sizeof(uint32_t)*2 + cantidadBloques/8); 
    if (err == -1){
        log_error(logger, "[SuperBloque] Error al liberal la memoria mapeada ");
    }
    bitarray_destroy(bitmap);
    free(memBitmap);
    free(pathSuperBloque);
    close(superBloque);
}


int validarBitsLibre(int cantidadBloquesAUsar){
    int contador = 0;
    int err;
     //Mapeo ahora para sacar el bitmap tmb
    char* pathSuperBloque = pathCompleto("SuperBloque.ims");
    int superBloque = open(pathSuperBloque, O_CREAT | O_RDWR, 0664);
    sb_memoria = mmap(NULL, sizeof(uint32_t) * 2 + cantidadBloques/8 , PROT_READ | PROT_WRITE, MAP_SHARED, superBloque, 0);
    
     
    memBitmap = malloc(cantidadBloques/8);
    memcpy(memBitmap, sb_memoria + sizeof(uint32_t)*2, cantidadBloques/8);
    bitmap = bitarray_create_with_mode((char*)memBitmap, cantidadBloques / 8, MSB_FIRST);  

    for(int i=0; i < bitarray_get_max_bit(bitmap); i++){
        if(bitarray_test_bit(bitmap,i) == 0){
            contador++;
            if(contador == cantidadBloquesAUsar){
                err = munmap(sb_memoria, sizeof(uint32_t)*2 + cantidadBloques/8);
                        
                if (err == -1){
                    log_error(logger, "[SuperBloque] Error al liberal la memoria mapeada ");
                }
                bitarray_destroy(bitmap);
                free(memBitmap);
                free(pathSuperBloque);
                return 1;
            }
        }
    }
    err = munmap(sb_memoria, sizeof(uint32_t)*2 + cantidadBloques/8);
        
    if (err == -1){
        log_error(logger, "[SuperBloque] Error al liberal la memoria mapeada ");
    }
    bitarray_destroy(bitmap);
    free(memBitmap);
    free(pathSuperBloque);
    close(superBloque);
    return -1;
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
        if((contador - bloquesHastaAhora) > 1){ //no es el ultimo bloque-->no hay frag. interna
            log_info(logger, "Se levanta del bloque.%d", i);
            bloque = atoi(listaBloques[bloquesHastaAhora]);
            char* temporalBloque = malloc(tamanioBloque+1);
            memcpy(temporalBloque, copiaBlocks + bloque*tamanioBloque, tamanioBloque);
            temporalBloque[tamanioBloque] = '\0';
                
            string_append(&string_temp,temporalBloque);
            bloquesHastaAhora++;
            free(temporalBloque);
        }else{
            log_info(logger, "Se levanta del bloque.%d", i);
            bloque = atoi(listaBloques[bloquesHastaAhora]);

            int sizeVieja = config_get_int_value(metadata, "SIZE");
            int fragmentacion =tamanioBloque - (sizeVieja %  tamanioBloque);

            log_info(logger, "fragm interna:%d, sizeViejo:%d, Contador:%d",fragmentacion,sizeVieja,contador);
            char* temporalBloque = malloc(sizeVieja+2);
            memcpy(temporalBloque, copiaBlocks + bloque*tamanioBloque, sizeVieja + 1);
            temporalBloque[sizeVieja + 1] = '\0';
                
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