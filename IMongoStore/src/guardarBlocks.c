#include "./headers/guardarBlocks.h"


void guardarEnBlocks(char* stringGuardar,char* path,int esRecurso){ 
    //agregar checkeo de si en el ultimo bloque anterior hay fragmentacion interna debo rellenarla

    int tamStr = string_length(stringGuardar);
    int cantidadBloquesUsados = 0;
    int posEnString = 0;
    int blockCount = 0;
                    
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
                        actualizarSize(metadata);
                        actualizarBlocks(metadata,i);
                        actualizarBlockCount(metadata);
                        //setearMD5(path);
                        config_destroy(metadata);                
                        cantidadBloquesUsados ++;
                    }else{
                        //Me muevo al bloque en si a guardar | pego en string moviendome hasta donde guarde antes | Pego todo el tamaño del bloque
                        memcpy(copiaBlocks + i*tamanioBloque,stringGuardar+posEnString*tamanioBloque,tamanioBloque);
                        posEnString ++;

                        //--------------------------ACTUALIZO METADATA---------------------------
                        t_config* metadata = config_create(path);
                        actualizarSize(metadata);
                        actualizarBlocks(metadata,i);
                        actualizarBlockCount(metadata);
                        //setearMD5(path);
                        config_destroy(metadata); 

                        cantidadBloquesUsados ++;
                    }
                }
            }
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
                            
                for(int i=0; i < bitarray_get_max_bit(bitmap) && cantidadBloquesUsados != cantidadBloquesAUsar; i++){
                    if(bitarray_test_bit(bitmap,i) == 0){
                        
                        bitarray_set_bit(bitmap,i);
                        memcpy(copiaSB+sizeof(int)*2,bitmap->bitarray,cantidadBloques/8);
                        
                        if((cantidadBloquesAUsar-cantidadBloquesUsados)==1){//ultimo bloque a escribir - posible fragmentación interna

                        //Me muevo al bloque en si a guardar | pego en string moviendome hasta donde guarde antes | Pego lo que me queda del string--> tamañoTotalStr - posicionAntEnStr*tamanioBloque
                            memcpy(copiaBlocks + i*tamanioBloque,stringGuardar+posEnString*tamanioBloque,tamStr-posEnString*tamanioBloque);                
                            posEnString ++;                
                                
                            //--------------------------ACTUALIZO METADATA---------------------------
                            t_config* metadata = config_create(path_fileTripulante);

                            actualizarSize(metadata);
                            actualizarBlocks(metadata,i);
                            actualizarBlockCount(metadata);
                            //setearMD5(path_fileTripulante);
                            config_destroy(metadata);                
                            cantidadBloquesUsados ++;
 
                        }else{
                            //Me muevo al bloque en si a guardar | pego en string moviendome hasta donde guarde antes | Pego todo el tamaño del bloque
                            memcpy(copiaBlocks + i*tamanioBloque,stringGuardar+posEnString*tamanioBloque,tamanioBloque);    
                            posEnString ++;
                            //--------------------------Actualizo metadata---------------------------
                            t_config* metadata = config_create(path_fileTripulante);

                            actualizarSize(metadata);
                            actualizarBlocks(metadata,i);
                            actualizarBlockCount(metadata);
                            //setearMD5(path_fileTripulante);
                            config_destroy(metadata); 
                            cantidadBloquesUsados ++;
                            }
                        }
                    }
                }
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
                        actualizarSize(metadata);
                        actualizarBlocks(metadata,i);
                        config_destroy(metadata);                
                        cantidadBloquesUsados ++;
                    }else{
                        //Me muevo al bloque en si a guardar | pego en string moviendome hasta donde guarde antes | Pego todo el tamaño del bloque
                        memcpy(copiaBlocks + i*tamanioBloque,stringGuardar+posEnString*tamanioBloque,tamanioBloque);
                        posEnString ++;

                        //--------------------------ACTUALIZO METADATA---------------------------
                        t_config* metadata = config_create(path);
                        actualizarSize(metadata);
                        actualizarBlocks(metadata,i);
                        config_destroy(metadata); 

                        cantidadBloquesUsados ++;
                    }
                }
            }
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
                            
                for(int i=0; i < bitarray_get_max_bit(bitmap) && cantidadBloquesUsados != cantidadBloquesAUsar; i++){
                    if(bitarray_test_bit(bitmap,i) == 0){
                        
                        bitarray_set_bit(bitmap,i);
                        memcpy(copiaSB+sizeof(int)*2,bitmap->bitarray,cantidadBloques/8);
                        
                        if((cantidadBloquesAUsar-cantidadBloquesUsados)==1){//ultimo bloque a escribir - posible fragmentación interna

                            //Me muevo al bloque en si a guardar | pego en string moviendome hasta donde guarde antes | Pego lo que me queda del string--> tamañoTotalStr - posicionAntEnStr*tamanioBloque
                            memcpy(copiaBlocks + i*tamanioBloque,stringGuardar+posEnString*tamanioBloque,tamStr-posEnString*tamanioBloque);                
                            posEnString ++;                
                                
                            //--------------------------ACTUALIZO METADATA---------------------------
                            t_config* metadata = config_create(path_fileTripulante);

                            actualizarSize(metadata);
                            actualizarBlocks(metadata,i);
                            config_destroy(metadata);                
                            cantidadBloquesUsados ++;
 
                        }else{
                            //Me muevo al bloque en si a guardar | pego en string moviendome hasta donde guarde antes | Pego todo el tamaño del bloque
                            memcpy(copiaBlocks + i*tamanioBloque,stringGuardar+posEnString*tamanioBloque,tamanioBloque);    
                            posEnString ++;
                            //--------------------------Actualizo metadata---------------------------
                            t_config* metadata = config_create(path_fileTripulante);

                            actualizarSize(metadata);
                            actualizarBlocks(metadata,i);
                            config_destroy(metadata); 
                            cantidadBloquesUsados ++;
                            }
                        }
                    }
                }
            }
        }           
    }
}
/*
void borrarEnBlocks(char* strABorrar,char* path,int flag,t_log* log){

    t_config* metadata = config_create(path);
    char* blocks = config_get_string_value(metadata,"BLOCKS");
    int block_count = config_get_int_value(metadata,"BLOCK_COUNT");

    int cantidadBloquesNecesito = cantidad_bloques(strABorrar, log);
    int tamanioString = strlen(strABorrar);
    
    int posEnString = 0;
    
    if(block_count*tamanioBloque > cantidadBloquesNecesito*tamanioBloque){ //existen elementos para borrar
        
        log_error(logger, "Eliminando oxigenos..");
        
        char** bloquesAEliminar = string_get_string_as_array(strABorrar);
        //["2","3","3"]

        for(int i = 0; i < block_count; i++){
            int bloque = atoi(bloquesAEliminar[i]);

            pthread_mutex_lock(&m_bitmap);
            bitarray_clean_bit(bitmap,bloque);
            pthread_mutex_unlock(&m_bitmap);


            if(){ //np es ultimo bloque
                pthread_mutex_lock(&m_blocks);
                memcpy(blocks_memory + bloque*tamanioBloque,strABorrar+posEnString*tamanioBloque,tamanioBloque);
                pthread_mutex_unlock(&m_blocks);
                
                pthread_mutex_lock(&m_metadata);
                t_config* metadata = config_create(path_fileTripulante);

                actualizarSize(metadata);
                actualizarBlocks(metadata,i);
                
                if(flag){
                    actualizarBlockCount(metadata);
                    //setearMD5(path_fileTripulante);
                }

                config_destroy(metadata); 
                pthread_mutex_unlock(&m_metadata);

                posEnString ++;
                   
            
            }else{
                pthread_mutex_lock(&m_blocks);
                memcpy(blocks_memory + bloque*tamanioBloque,strABorrar+posEnString*tamanioBloque,tamanioString-posEnString*tamanioBloque);
                pthread_mutex_unlock(&m_blocks);
                
                posEnString ++;                
            }
        }

    
    else{
        // Borro metadata entera y borro cada bloque de blocks

    }



}
*/