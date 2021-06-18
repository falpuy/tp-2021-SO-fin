#include "./headers/guardarBlocks.h"

void guardarPorBloque(char* stringGuardar,int posEnString, int tamStr, int cantidadBloquesAUsar, int cantidadBloquesUsados, char* path,int esRecurso, int flagEsGuardar){
    for(int i=0; i < bitarray_get_max_bit(bitmap) && cantidadBloquesUsados != cantidadBloquesAUsar; i++){
        if(bitarray_test_bit(bitmap,i) == 0){
                    
            bitarray_set_bit(bitmap,i);
            memcpy(copiaSB+sizeof(int)*2,bitmap->bitarray,cantidadBloques/8);
                
            if((cantidadBloquesAUsar-cantidadBloquesUsados)==1){//ultimo bloque a escribir - posible fragmentación interna 
                        
                //Me muevo al bloque en si a guardar | pego en string moviendome hasta donde guarde antes | Pego lo que me queda del string--> tamañoTotalStr - posicionAntEnStr*tamanioBloque
                memcpy(copiaBlocks + i*tamanioBloque,stringGuardar+posEnString*tamanioBloque,tamStr-posEnString*tamanioBloque);                
                posEnString ++;                
                //--------------------------ACTUALIZO METADATA---------------------------
                t_config* metadata = config_create(path);
                
                actualizarSize(metadata,tamStr-posEnString*tamanioBloque,flagEsGuardar);
                actualizarBlocks(metadata,i,flagEsGuardar);
                if(esRecurso){
                    actualizarBlockCount(metadata,flagEsGuardar);
                    //setearMD5(path);
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
                actualizarBlocks(metadata,i,flagEsGuardar);
                if(esRecurso){
                    actualizarBlockCount(metadata,flagEsGuardar);
                    //setearMD5(path);
                }
                config_destroy(metadata); 

                cantidadBloquesUsados ++;
            }
        }
    }
}

void guardarEnBlocks(char* stringGuardar,char* path,int esRecurso){ 
    int tamStr = string_length(stringGuardar);
    int cantidadBloquesUsados = 0;
    int posEnString = 0;
    int blockCount = 0;
    int flagEsGuardar = 1
                    
    if(esRecurso){
        t_config* metadata = config_create(path);
        int sizeGuardado = config_get_int_value(metadata, "SIZE"); 
        
        if(sizeGuardado == 0){//METADATA VACIA
            config_destroy(metadata); 
            int cantidadBloquesAUsar = cantidad_bloques(stringGuardar);
            int err = validarBitsLibre(cantidadBloquesAUsar);
            if(err < 0){
                log_error(logger, "No existe más espacio para guardar en filesystem");
                log_error(logger,"Finalizando programa...");
                exit(-1);
            }
            guardarPorBloque(stringGuardar,posEnString, tamStr,cantidadBloquesAUsar,cantidadBloquesUsados,path,esRecurso,flagEsGuardar);
        }
        else{ //HAY ALGO EN METADATA
            char** listaBloque = config_get_array_value(metadata,"BLOCKS");
            int contador = 0;

            while(listaBloques[contador] != NULL){ //-->testear 
              contador++;
            }
          	config_destroy(metadata);
          	int ultimoBloque = atoi(listaBloques[contador]);
          	int sizeTotal = (contador+1) * tamanioBloque; //+1 porque empieza de 0
          	int faltante = sizeTotal - sizeGuardado;
          	int posicion = tamanioBloque - faltante;


            //validar si hay lugar para pegar todo el string
            char* sobranteString = malloc((tamStr - faltante) + 1);
            memcpy(sobranteString,stringGuardar + faltante,tamStr - faltante);
            sobranteString[tamStr - faltante] = '\0';
            int cantidadBloquesAUsar = cantidad_bloques(sobranteString);

            if(cantidadBloquesAUsar == 0){
                log_info("[Reemplazando fragmentacion interna] No hay mas bloques que usar");
            }else{
                int err = validarBitsLibre(cantidadBloquesAUsar);
            
                if(err < 0){
                    log_error(logger, "No existe más espacio para guardar en filesystem");
                    log_error(logger,"Finalizando programa...");
                    exit(-1);
                }

                memcpy(copiaBlocks + (ultimoBloque * tamanioBloque) + posicion, strAGuardar, faltante);
                posEnString += faltante;
                sizeGuardado += faltante;
                guardarPorBloque(sobranteString,posEnString,tamStr,cantidadBloquesAUsar,cantidadBloquesUsados,path,esRecurso,flagEsGuardar);         

                free(sobranteString);
                for(int i = 0; i < block_count; i++){
                    free(listaBloques[i]);
                }
                free(listaBloques);
            }
        }  
    }
    else{   //FILE
        t_config* metadata = config_create(path);
        int sizeGuardado = config_get_int_value(metadata, "SIZE"); 
        
        if(sizeGuardado == 0){//METADATA VACIA
            config_destroy(metadata); 
            int cantidadBloquesAUsar = cantidad_bloques(stringGuardar);
            int err = validarBitsLibre(cantidadBloquesAUsar);
            if(err < 0){
                log_error(logger, "No existe más espacio para guardar en filesystem");
                log_error(logger,"Finalizando programa...");
                exit(-1);
            }
            guardarPorBloque(stringGuardar,posEnString,tamStr,cantidadBloquesAUsar,cantidadBloquesUsados,path,esRecurso,flagEsGuardar);
        }
        else{ //HAY ALGO EN METADATA
            char** listaBloque = config_get_array_value(metadata,"BLOCKS");
            int contador = 0;

            while(listaBloques[contador] != NULL){ //-->testear 
              contador++;
            }
          	config_destroy(metadata);
          	int ultimoBloque = atoi(listaBloques[contador]);
          	int sizeTotal = (contador+1) * tamanioBloque; //+1 porque empieza de 0
          	int faltante = sizeTotal - sizeGuardado;
          	int posicion = tamanioBloque - faltante;


            //validar si hay lugar para pegar todo el string
            char* sobranteString = malloc((tamStr - faltante) + 1);
            memcpy(sobranteString,stringGuardar + faltante,tamStr - faltante);
            sobranteString[tamStr - faltante] = '\0';
            int cantidadBloquesAUsar = cantidad_bloques(sobranteString);

            if(cantidadBloquesAUsar == 0){
                log_info("[Reemplazando fragmentacion interna] No hay mas bloques que usar");
            }else{
                int err = validarBitsLibre(cantidadBloquesAUsar);
            
                if(err < 0){
                    log_error(logger, "No existe más espacio para guardar en filesystem");
                    log_error(logger,"Finalizando programa...");
                    exit(-1);
                }

                memcpy(copiaBlocks + (ultimoBloque * tamanioBloque) + posicion, strAGuardar, faltante);
                posEnString += faltante;
                sizeGuardado += faltante;  
                guardarPorBloque(sobranteString,posEnString,tamStr,cantidadBloquesAUsar,cantidadBloquesUsados,path,esRecurso,flagEsGuardar);
            }
            free(sobranteString);
            for(int i = 0; i < block_count; i++){
                free(listaBloques[i]);
            }
            free(listaBloques);
        }           
    }
}


void borrarEnBlocks(char* stringABorrar,char* path,int esRecurso){
    
    int tamStr = string_length(stringABorrar);
    int cantidadBloquesUsados = 0;
    int posEnString = 0;
    int blockCount = 0;
    int flagEsGuardar = 0;


    if(esRecurso){
        t_config* metadata = config_create(path);
        char** bloques = config_get_array_value(metadata,"BLOCKS");
        int sizeBlock = config_get_int_value(metadata, "SIZE");
        int block_count = config_get_int_value(metadata, "BLOCK_COUNT");
        int contador = 0;
        
        if(block_count == 0){
            log_error(logger, "No existe Recurso para borrar..");
        }else{
            while(int i = 0; i < block_count ; i ++){ 
                contador++;
            }
            //|OOO    | --> TOTAL: 7
            int ultimoBloque = atoi(listaBloques[contador]);
            int faltante = (cantidadBloques - tamanioBloque) - sizeBlock; //4
            int empezarABorrar = tamanioBloque - faltante; //3
            int cantidadCharBorrados = 0;
            int posEnString = 0;

            //Borro los extra de fragmentacion interna
            for(int i = 0; cantidadCharBorrados != empezarABorrar; i++){
                char* charBorrar = string_new();
                string_append(&charBorrar," ");
                memcpy(copiaBlocks + ultimoBloque*tamanioBloque + empezarABorrar,charBorrar,1);                
                posEnString++;
                empezarABorrar--;
                free(charBorrar);
            }
            log_info(logger, "Se borraron los chars de fragmentacion interna..");
            bitarray_clean_bit(bitmap,ultimoBloque);
            memcpy(copiaSB+sizeof(int)*2,bitmap->bitarray,cantidadBloques/8);
            log_info(logger, "Se liberó bit en bitmap de la posicion:%d",ultimoBloque);
            

            int tamanio = strlen(stringABorrar) - empezarABorrar;
            char* nuevoStrBorrar = 

            actualizarSize(metadata,tamanio,flagEsGuardar);
            actualizarBlocks(metadata,i,flagEsGuardar);
            actualizarBlockCount(metadata,flagEsGuardar);
            // setearMD5(path);
            config_destroy(metadata);   
            borrarPorBloque(stringABorrar, posEnString,tamStr, cantidadBloquesAUsar,cantidadBloquesUsados,path, esRecurso,flagEsGuardar);
    }else{

    }
}


void borrarPorBloque(char* stringBorrar,int posEnString, int tamStr, int cantidadBloquesABorrar,char* path,int esRecurso){
    t_config* metadata = config_create(path);
    char** listaBloques = config_get_array_value(metadata, "BLOCKS");
    config_destroy(metadata);
    int block_count = 0;
    int cantidadBloquesUsados = 0;
    int flagEsGuardar = 0;

    for(int i = 0; listaBloques[i] != NULL; i++){
        block_count++;
    }

    int bloque;
    for(int i=0; cantidadBloquesABorrar != cantidadBloquesAUsar; i++){
        
        if((cantidadBloquesAUsar-cantidadBloquesUsados) == 1 ){ //Se podría no guardar entero
            char* charRestantes = string_new();
            string_append(&charRestantes, stringABorrar + posEnString);
            int cant_charRestantes = string_length(charRestantes);

            if(cant_charRestantes == tamanioBloque){ //se guarda entero
                bloque = atoi(listaBloques[i]);
                bitarray_clean_bit(bitmap,bloque);
                memcpy(copiaSB+sizeof(int)*2,bitmap->bitarray,cantidadBloques/8);
                        
                memcpy(copiaBlocks + bloque*tamanioBloque,stringGuardar+posEnString*tamanioBloque,tamanioBloque);                
                posEnString ++;                
                t_config* metadata = config_create(path);
                actualizarSize(metadata,flagEsGuardar);
                actualizarBlocks(metadata,i,flagEsGuardar);
                if(esRecurso){
                    actualizarBlockCount(metadata,flagEsGuardar);
                    //setearMD5(path,flagEsGuardar);
                }
                config_destroy(metadata);                
                cantidadBloquesUsados ++;
            }else{ //no se guarda entero el bloque
                bloque = atoi(listaBloques[i]);                        
                memcpy(copiaBlocks + bloque*tamanioBloque,stringGuardar+posEnString*tamanioBloque,cant_charRestantes);                
                posEnString ++;                
                t_config* metadata = config_create(path);
                
                actualizarSize(metadata,flagEsGuardar);
                actualizarBlocks(metadata,i,flagEsGuardar);
                if(esRecurso){
                    actualizarBlockCount(metadata,flagEsGuardar);
                    //setearMD5(path,flagEsGuardar);
                }
                config_destroy(metadata);                
                cantidadBloquesUsados ++;
                }
            free(charRestantes);
            
        }else{ //Se guarda entero 
            
            bloque = atoi(listaBloques[i]);
            bitarray_clean_bit(bitmap,bloque);
            memcpy(copiaSB+sizeof(int)*2,bitmap->bitarray,cantidadBloques/8);
                    
            memcpy(copiaBlocks + bloque*tamanioBloque,stringBorrar+posEnString*tamanioBloque,tamanioBloque);                
            posEnString ++;                
            
            //Borrar metadata
            t_config* metadata = config_create(path);
            actualizarSize(metadata,flagEsGuardar);
            actualizarBlocks(metadata,i,flagEsGuardar);
            if(esRecurso){
                actualizarBlockCount(metadata,flagEsGuardar);
                //setearMD5(path,flagEsGuardar);
            }
            config_destroy(metadata);                
            cantidadBloquesUsados ++;
        }
    }

    for(int i = 0; i < block_count; i++){
        free(listaBloques[i]);
    }
    free(listaBloques);
}


        