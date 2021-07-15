#include "./headers/guardarBlocks.h"

void guardarPorBloque(char* stringGuardar,int posEnString, int cantidadBloquesAUsar,char* path,int esRecurso, int flagEsGuardar){
    
    int cantidadBloquesUsados = 0;
    int tamanioString = string_length(stringGuardar);

    for(int i=0; i < bitarray_get_max_bit(bitmap) && cantidadBloquesUsados != cantidadBloquesAUsar; i++){
        if(bitarray_test_bit(bitmap,i) == 0){
                    
            bitarray_set_bit(bitmap,i);
            memcpy(copiaSB+sizeof(int)*2,bitmap->bitarray,cantidadBloques/8);
                
            if((cantidadBloquesAUsar-cantidadBloquesUsados)==1){//ultimo bloque a escribir - posible fragmentación interna 
                
                //Me muevo al bloque en si a guardar | pego en string moviendome hasta donde guarde antes | Pego lo que me queda del string--> tamañoTotalStr - posicionAntEnStr*tamanioBloque
                memcpy(copiaBlocks + i*tamanioBloque,stringGuardar+posEnString*tamanioBloque,tamanioString-posEnString*tamanioBloque);                
                posEnString ++;    

                //--------------------------ACTUALIZO METADATA---------------------------
                t_config* metadata = config_create(path);
                
                actualizarSize(metadata,tamanioString-(posEnString-1)*tamanioBloque,flagEsGuardar);
                actualizarBlocks(metadata,i,flagEsGuardar);
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
                actualizarBlocks(metadata,i,flagEsGuardar);
                if(esRecurso){
                    actualizarBlockCount(metadata,flagEsGuardar);
                }
                config_destroy(metadata); 

                cantidadBloquesUsados ++;
            }
        }
    }
}

void guardarEnBlocks(char* stringGuardar,char* path,int esRecurso){ 
    int tamStr = string_length(stringGuardar);
    int flagEsGuardar = 1;
    int posEnString = 0;
                    
    if(esRecurso){
        log_info(logger, "Soy recurso. String a guardar:%s", stringGuardar);
        t_config* metadata = config_create(path);
        log_info(logger,"%s", stringGuardar);

        int sizeGuardado = config_get_int_value(metadata, "SIZE"); 
        config_destroy(metadata); 
        
        if(sizeGuardado == 0){//METADATA VACIA --> No hay nada guardado
            
            int cantidadBloquesAUsar = cantidad_bloques(stringGuardar);
            int err = validarBitsLibre(cantidadBloquesAUsar);
            if(err < 0){
                log_error(logger, "No existe más espacio para guardar en filesystem");
                log_error(logger,"Finalizando programa...");
                exit(-1);
            }
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
            
            int ultimoBloque = atoi(listaBloques[contador-1]);
          	int sizeTotal = contador * tamanioBloque; 
            int faltante = sizeTotal - sizeGuardado;
          	int posicion = tamanioBloque - faltante;

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
            
            if(cantidadBloquesAUsar == 0){ //REVISAR
                log_info(logger,"[Reemplazando fragmentacion interna] No hay mas bloques que usar");
            }else{
                memcpy(copiaBlocks + (ultimoBloque * tamanioBloque) + posicion, stringGuardar, faltante);
                log_info(logger, "%s", stringGuardar);
                metadata = config_create(path);
                actualizarSize(metadata,faltante,flagEsGuardar);
                config_destroy(metadata);

                guardarPorBloque(sobranteString,0,cantidadBloquesAUsar,path,esRecurso,flagEsGuardar);  
                setearMD5(path);
                free(sobranteString);
                for(int i = 0; i < contador; i++){
                    free(listaBloques[i]);
                }
                free(listaBloques);
            }
        }  
    }
    else{   //BITACORA
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
            
          	int ultimoBloque = atoi(listaBloques[contador-1]);
          	int sizeTotal = contador * tamanioBloque; 
            int faltante = sizeTotal - sizeGuardado;
          	int posicion = tamanioBloque - faltante;

            char* sobranteString = malloc((tamStr - faltante) + 2);
            memcpy(sobranteString,stringGuardar + faltante,tamStr - faltante);
            sobranteString[tamStr - faltante] = '\0';
            int cantidadBloquesAUsar = cantidad_bloques(sobranteString);

            int err = validarBitsLibre(cantidadBloquesAUsar);
            if(err < 0){
                log_error(logger, "No existe más espacio para guardar en filesystem");
                log_error(logger,"Finalizando programa...");
                exit(-1);
            }
            
            // |O   |

            if(cantidadBloquesAUsar == 0){
                log_info(logger,"[Reemplazando fragmentacion interna] No se necesitan mas bloques para pegar el string sobrante");
                //falta rellenar ese bloque
                // memcpy(copiaBlocks + (ultimoBloque * tamanioBloque) + posicion, stringGuardar, faltante);

            }else{
                memcpy(copiaBlocks + (ultimoBloque * tamanioBloque) + posicion, stringGuardar, faltante);
                metadata = config_create(path);
                actualizarSize(metadata,faltante,flagEsGuardar);
                config_destroy(metadata);
                // setearMD5(path);
                
                guardarPorBloque(sobranteString,0,cantidadBloquesAUsar,path,esRecurso,flagEsGuardar);  
                free(sobranteString);
                for(int i = 0; i < contador; i++){
                    free(listaBloques[i]);
                }
                free(listaBloques);
                
            }
        }           
    }
}




void borrarEnBlocks(char* stringABorrar,char* path,int esRecurso){
		
  	t_config* metadata = config_create(path);
  	int sizeAnterior = config_get_int_value(metadata, "SIZE");
  	char** listaBloques = config_get_array_value(metadata,"BLOCKS");
  	int contador = 0;
  	int tamStrBorrar = string_length(stringABorrar);
    config_destroy(metadata);
  
    while(listaBloques[contador]){ 
        contador++;
    }
  	
  	int bloqueABorrar;
  	int fragmentacion;
	int charUltimoBloque;
  
    while(tamStrBorrar) {
        bloqueABorrar = atoi(listaBloques[contador-1]);
        fragmentacion = contador*tamanioBloque - sizeAnterior;
  		charUltimoBloque = tamanioBloque - fragmentacion;
      
     	if (charUltimoBloque <= tamStrBorrar) {

            memset(copiaBlocks + bloqueABorrar*tamanioBloque,' ', charUltimoBloque);
            tamStrBorrar -= charUltimoBloque;

            bitarray_clean_bit(bitmap,bloqueABorrar);
            memcpy(copiaSB+sizeof(int)*2,bitmap->bitarray,cantidadBloques/8);
            metadata = config_create(path);
            actualizarSize(metadata, charUltimoBloque, 0);
            actualizarBlockCount(metadata,0);
            actualizarBlocks(metadata, bloqueABorrar, 0);
            config_destroy(metadata);
            contador--;
    }else{
            //Borrar "OO"
            //|OOOO| 
            //me posiciono en el bloque correspondiente y me muevo los chars de ese bloque - los que me faltan asi los borro desde ese principio
            memset(copiaBlocks + (bloqueABorrar*tamanioBloque) + charUltimoBloque - tamStrBorrar,' ', tamStrBorrar);
            metadata = config_create(path);
            actualizarSize(metadata, tamStrBorrar, 0);     
            config_destroy(metadata);
            tamStrBorrar = 0;
      }
    } 

    setearMD5(path);
    
    for(int i = 0; i <= contador; i++){
        free(listaBloques[i]);
    }
    free(listaBloques);
    
}


