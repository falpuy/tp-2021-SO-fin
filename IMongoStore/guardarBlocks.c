#include "./headers/guardarBlocks.h"

void guardarEnBlocks(char* stringGuardar,char* path,int esRecurso){ 
    int tamStr = string_length(stringGuardar);
    int flagEsGuardar = 1;
    int posEnString = 0;

    log_info(logger,"El path es: %s",path); 
    if(esRecurso){
        t_config* metadata = config_create(path);
        int sizeGuardado = config_get_int_value(metadata, "SIZE"); 
        config_destroy(metadata);
        log_info(logger, "El size guardado es:%d", sizeGuardado);
         

        if(sizeGuardado == 0){
            log_info(logger, "guardandose por primera vez el recurso");
            int cantidadBloquesAUsar = cantidad_bloques(stringGuardar);
            int err = validarBitsLibre(cantidadBloquesAUsar);
            if(err < 0){
                log_error(logger, "No existe más espacio para guardar en filesystem");
                log_error(logger,"Finalizando programa...");
                exit(-1);
            }
            log_info(logger, "El string nuevo a pegar es:%s", stringGuardar);
            guardarPorBloque(stringGuardar,posEnString,cantidadBloquesAUsar,path,esRecurso,flagEsGuardar);
            setearMD5(path);
        }
        else{ //HAY ALGO EN METADATA
            log_info(logger, "Guardandose en blocks con metadata existente..");
            metadata = config_create(path);
            char** listaBloques = config_get_array_value(metadata,"BLOCKS");
            config_destroy(metadata);
            int contador = 0;

            while(listaBloques[contador]){  //cuenta 1 de más
              contador++;
            }

            log_info(logger, "Hay %d bloques", contador);

            int ultimoBloque = atoi(listaBloques[contador-1]);
            log_info(logger, "Ultimo bloque de la lista es: %d", ultimoBloque);

          	int sizeTotal = contador * tamanioBloque; 
            log_info(logger, "El size guardado es:%d", sizeGuardado);
            int faltante = sizeTotal - sizeGuardado;
            log_info(logger, "Frag. Interna en ese bloque: %d", faltante);

          	int posicion = tamanioBloque - (faltante%tamanioBloque);
            log_info(logger, "Voy a comenzar a pegar el string en la posicion:%d", posicion);

            if(faltante >= tamStr){
                metadata = config_create(path);
                log_info(logger, "Se puede guardar dentro de una fragmentacion");

                memcpy(copiaBlocks + (ultimoBloque * tamanioBloque) + posicion, stringGuardar, tamStr);
                actualizarSize(metadata,tamStr - 1,flagEsGuardar);
                config_destroy(metadata);
                setearMD5(path);
            }else{
                char* sobranteString = malloc((tamStr - faltante) + 1);
                memcpy(sobranteString,stringGuardar + faltante,tamStr - faltante);
                sobranteString[tamStr - faltante] = '\0';
                int cantidadBloquesAUsar = cantidad_bloques(sobranteString);

                int err = validarBitsLibre(cantidadBloquesAUsar);
                if(err < 0){
                    log_error(logger, "No existe más espacio para guardar en filesystem");
                    log_error(logger,"Finalizando programa...");
                    exit(-1);
                }
                log_info(logger, "DATOS: ultimobloque*tamanio:%d, posicion-1:%d, faltante:%d", ultimoBloque*tamanioBloque, posicion-1, faltante);
                memcpy(copiaBlocks + (ultimoBloque * tamanioBloque) + posicion, stringGuardar, faltante);
                metadata = config_create(path);
                actualizarSize(metadata,faltante,flagEsGuardar);
                config_destroy(metadata);

                guardarPorBloque(sobranteString,0,cantidadBloquesAUsar,path,esRecurso,flagEsGuardar);  
                setearMD5(path);
                free(sobranteString);
            }
            for(int i = 0; i < contador; i++){
                free(listaBloques[i]);
            }
            free(listaBloques);
        }
    }else{   //BITACORA
        t_config* metadata = config_create(path);
        int sizeGuardado = config_get_int_value(metadata, "SIZE"); 
        config_destroy(metadata); 

        if(sizeGuardado == 0){//METADATA VACIA

            char* temporal = string_new();
            string_append(&temporal, stringGuardar);

            int cantidadBloquesAUsar = cantidad_bloques(temporal);
            int err = validarBitsLibre(cantidadBloquesAUsar);
            if(err < 0){
                log_error(logger, "No existe más espacio para guardar en filesystem");
                log_error(logger,"Finalizando programa...");
                exit(-1);
            }

            log_info(logger, "TEMPORAL NUEVO METADATA: %s", temporal);
            guardarPorBloque(temporal,posEnString,cantidadBloquesAUsar,path,esRecurso,flagEsGuardar);
            free(temporal);
        }
        else{ //HAY ALGO EN METADATA
            log_info(logger, "Guardandose en blocks con metadata existente..");

            metadata = config_create(path);
            char** listaBloques = config_get_array_value(metadata,"BLOCKS");
            config_destroy(metadata);

            int contador = 0;

            while(listaBloques[contador]){  //cuenta 1 de más
              contador++;
            }

            //Datos hasta ahora
          	int ultimoBloque = atoi(listaBloques[contador-1]);
            log_info(logger, "Ultimo bloque de la lista es: %d", ultimoBloque);

            int sizeTotal = contador * tamanioBloque;  //Hasta ese momento
            // log_info(logger, "SizeGuardado:%d", sizeGuardado);
            int faltante = sizeTotal - (sizeGuardado + 1); //fragmentacion interna del bloque
            log_info(logger, "Frag. Interna en ese bloque: %d", faltante);

          	int posicion = tamanioBloque - faltante;
            log_info(logger, "Voy a comenzar a pegar el string en la posicion:%d", posicion);

            //Comienzo a pegar nuestro string
            //checkeo si alcanza pegarlo en mi fragmentacion
            if(faltante >= tamStr){
                metadata = config_create(path);
                log_info(logger, "Se puede guardar dentro de una fragmentacion");
                // log_info(logger, "DATOS: posicion:%d, tamstr:%d,faltante:%d,stringGuardar:%s, sizeTotal:%d, sizeGuardado:%d",posicion, tamStr,faltante,stringGuardar,sizeTotal,sizeGuardado);
                memcpy(copiaBlocks + (ultimoBloque * tamanioBloque) + posicion, stringGuardar, tamStr);
                actualizarSize(metadata,tamStr,flagEsGuardar);
                config_destroy(metadata);
            }else{
                // log_info(logger, "DATOS2: posicion:%d, tamstr:%d,faltante:%d,stringGuardar:%s, sizeTotal:%d, sizeGuardado:%d",posicion, tamStr,faltante,stringGuardar,sizeTotal,sizeGuardado);
                
                // HOLAAAAAAAAAAAAA  -> 17 
                // |HOLA@HO|LAAAAAAAAAAAAA@

                // total: 22
                // pegamos:9 
                // tamanio:13

                char* sobranteString = malloc(tamStr - (faltante - 1) + 1);
                memcpy(sobranteString,stringGuardar + (faltante),tamStr - (faltante - 1));
                sobranteString[tamStr - (faltante -1)] = '\0';
                log_info(logger, "Sobrante de string:%s,",sobranteString);

                int cantidadBloquesAUsar = cantidad_bloques(sobranteString);

                int err = validarBitsLibre(cantidadBloquesAUsar);
                if(err < 0){
                    log_error(logger, "No existe más espacio para guardar en filesystem");
                    log_error(logger,"Finalizando programa...");
                    exit(-1);
                }

                memcpy(copiaBlocks + (ultimoBloque * tamanioBloque) + posicion,stringGuardar, faltante);
                metadata = config_create(path);
                actualizarSize(metadata,faltante + 1,flagEsGuardar);
                config_destroy(metadata);
                guardarPorBloque(sobranteString,0,cantidadBloquesAUsar,path,esRecurso,flagEsGuardar); 
                free(sobranteString);
            }

        for(int i = 0; i < contador; i++){
            free(listaBloques[i]);
        }
        free(listaBloques);

        }
    }           
}





void borrarEnBlocks(char* stringABorrar,char* path,int esRecurso,char recurso){

  	t_config* metadata;
  	int sizeAnterior;
  	char** listaBloques;
  	int contador;
  	int tamStrBorrar = string_length(stringABorrar);
  	int bloqueABorrar;
  	int fragmentacion;
	int posicion;
    log_info(logger, "--------------------------------------------");
    log_info(logger, "Tamaño del string a borrar:%d",tamStrBorrar);
    
    metadata = config_create(path);
    sizeAnterior = config_get_int_value(metadata, "SIZE");
    listaBloques = config_get_array_value(metadata,"BLOCKS");
    config_destroy(metadata);

    contador = 0;
    while(listaBloques[contador]){ 
        contador++;
    }

    if(sizeAnterior <= tamStrBorrar){

        char* pathSuperBloque = pathCompleto("SuperBloque.ims");
        int superBloque = open(pathSuperBloque, O_CREAT | O_RDWR, 0664);
        sb_memoria = mmap(NULL, sizeof(uint32_t) * 2 + cantidadBloques/8 , PROT_READ | PROT_WRITE, MAP_SHARED, superBloque, 0);
    
        memBitmap = malloc(cantidadBloques/8);
        memcpy(memBitmap, sb_memoria + sizeof(uint32_t)*2, cantidadBloques/8);
        bitmap = bitarray_create_with_mode((char*)memBitmap, cantidadBloques / 8, MSB_FIRST);  

        for(int i=0; i < contador; i++){
            int bloque = atoi(listaBloques[i]);
            log_info(logger, "Borrando el bloque:%d",bloque);
            bitarray_clean_bit(bitmap,bloque);
        }
        memcpy(sb_memoria + sizeof(int) * 2,bitmap->bitarray,cantidadBloques/8);
        int err = munmap(sb_memoria, sizeof(uint32_t)*2 + cantidadBloques/8);
        
        if (err == -1){
            log_error(logger, "[SuperBloque] Error al liberal la memoria mapeada ");
        }
        bitarray_destroy(bitmap);
        free(memBitmap);
        free(pathSuperBloque);
        close(superBloque);

        metadata = config_create(path);
        actualizarSize(metadata, sizeAnterior, 0);
        char* vacio = string_new();
        string_append(&vacio,"0");
        config_set_value(metadata,"BLOCK_COUNT",vacio);
        free(vacio);
        char* vacio2 = string_new();
        string_append(&vacio2,"[]");
        config_set_value(metadata,"BLOCKS",vacio2);
        free(vacio2);
        config_save(metadata);
        config_destroy(metadata);
        for(int i = 0; i <= contador; i++){
            free(listaBloques[i]);
        }
        free(listaBloques);
    }else{
        while(tamStrBorrar) {
            for(int i = 0; i <= contador; i++){
                free(listaBloques[i]);
            }
            free(listaBloques);
            metadata = config_create(path);
            sizeAnterior = config_get_int_value(metadata, "SIZE");
            listaBloques = config_get_array_value(metadata,"BLOCKS");
            config_destroy(metadata);

            contador = 0;

            while(listaBloques[contador]){ 
                contador++;
            }

            bloqueABorrar = atoi(listaBloques[contador-1]);
            fragmentacion = contador*tamanioBloque - sizeAnterior;
            posicion = tamanioBloque - fragmentacion;

            if(tamStrBorrar > posicion){ 
                log_info(logger, "El tamanio que hay en el bloque es menor a lo que yo quiero borrar");

                // memset(copiaBlocks + bloqueABorrar*tamanioBloque,' ', posicion); //borro esos 5 limpio todo
                tamStrBorrar -= posicion; //nuevo tamanio--> 1

                //Mapeo ahora para sacar el bitmap tmb
                char* pathSuperBloque = pathCompleto("SuperBloque.ims");
                int superBloque = open(pathSuperBloque, O_CREAT | O_RDWR, 0664);
                sb_memoria = mmap(NULL, sizeof(uint32_t) * 2 + cantidadBloques/8 , PROT_READ | PROT_WRITE, MAP_SHARED, superBloque, 0);
        
                memBitmap = malloc(cantidadBloques/8);
                memcpy(memBitmap, sb_memoria + sizeof(uint32_t)*2, cantidadBloques/8);
                bitmap = bitarray_create_with_mode((char*)memBitmap, cantidadBloques / 8, MSB_FIRST);  
                bitarray_clean_bit(bitmap,bloqueABorrar);
                memcpy(sb_memoria + sizeof(int) * 2,bitmap->bitarray,cantidadBloques/8);
                int err = munmap(sb_memoria, sizeof(uint32_t)*2 + cantidadBloques/8);
            
                if (err == -1){
                    log_error(logger, "[SuperBloque] Error al liberal la memoria mapeada ");
                }
                bitarray_destroy(bitmap);
                free(memBitmap);
                free(pathSuperBloque);
                close(superBloque);
                // memcpy(sb_memoria+sizeof(int)*2,bitmap->bitarray,cantidadBloques/8);

                metadata = config_create(path);
                actualizarSize(metadata, posicion, 0);
                actualizarBlockCount(metadata,0);
                config_destroy(metadata);
                actualizarBlocks(bloqueABorrar, 0,path);
                // log_info(logger, "Tamanio string luego de borrado es:%d", tamStrBorrar);
            }else{
                memset(copiaBlocks + bloqueABorrar*tamanioBloque + (posicion-tamStrBorrar) ,'|', 1);

                t_config* metadata2 = config_create(path);
                actualizarSize(metadata2, tamStrBorrar, 0);     
                config_destroy(metadata2);
                tamStrBorrar = 0;
            }
            
            for(int i = 0; i <= contador; i++){
                free(listaBloques[i]);
            }
            free(listaBloques);
            
            }
    
    }


    setearMD5(path);


}