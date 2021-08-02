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
}

void    guardarEnBlocks(char* stringGuardar,char* path,int esRecurso){ 
    int tamStr = string_length(stringGuardar);
    int flagEsGuardar = 1;
    int posEnString = 0;

    log_info(logger, "Tamanio string a pegar:%d", tamStr);

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
            log_info(logger, "Ultimo bloque de la lista es: %d", ultimoBloque);

          	int sizeTotal = contador * tamanioBloque; 
            int faltante = sizeTotal - sizeGuardado;
            log_info(logger, "Frag. Interna en ese bloque: %d", faltante);

          	int posicion = tamanioBloque - faltante;
            log_info(logger, "Voy a comenzar a pegar el string en la posicion:%d", posicion);

            if(faltante >= tamStr){
                metadata = config_create(path);
                log_info(logger, "Se puede guardar dentro de una fragmentacion");
                memcpy(copiaBlocks + (ultimoBloque * tamanioBloque) + posicion, stringGuardar, tamStr);
                actualizarSize(metadata,tamStr,flagEsGuardar);
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

                memcpy(copiaBlocks + (ultimoBloque * tamanioBloque) + posicion, stringGuardar, faltante);
                log_info(logger, "%s", stringGuardar);
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
            int faltante = sizeTotal - sizeGuardado; //fragmentacion interna del bloque
            log_info(logger, "Frag. Interna en ese bloque: %d", faltante);

          	int posicion = tamanioBloque - faltante;
            log_info(logger, "Voy a comenzar a pegar el string en la posicion:%d", posicion);

            //Comienzo a pegar nuestro string
            //checkeo si alcanza pegarlo en mi fragmentacion
            if(faltante >= tamStr){
                metadata = config_create(path);
                log_info(logger, "Se puede guardar dentro de una fragmentacion");
                memcpy(copiaBlocks + (ultimoBloque * tamanioBloque) + posicion, stringGuardar, tamStr);
                actualizarSize(metadata,tamStr,flagEsGuardar);
                config_destroy(metadata);
            }else{
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

                memcpy(copiaBlocks + (ultimoBloque * tamanioBloque) + posicion, stringGuardar, faltante);
                metadata = config_create(path);
                actualizarSize(metadata,faltante,flagEsGuardar);
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
    config_destroy(metadata);

    if(sizeAnterior < tamStrBorrar){
        log_error(logger, "Se quiere intentar eliminar mas de lo que está guardado en FS");
        char* path;
        if(recurso == 'O'){
            path = pathCompleto("Files/Oxigeno.ims");
            remove(path);
            free(path);
            crearMetadataFiles(path,'O');
        }else if(recurso == 'C'){
            path = pathCompleto("Files/Comida.ims");
            remove(path);
            free(path);
            crearMetadataFiles(path,'C');
        }else if(recurso == 'B'){
            path = pathCompleto("Files/Basura.ims");
            remove(path);
            free(path);
            crearMetadataFiles(path,'B');
        }
    }else{
        while(tamStrBorrar) {
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


            if (posicion == tamStrBorrar) {
                // log_info(logger, "El tamanio que hay en el bloque es igual a lo que yo quiero borrar");

                memset(copiaBlocks + bloqueABorrar*tamanioBloque,' ', tamanioBloque);

                bitarray_clean_bit(bitmap,bloqueABorrar);
                memcpy(copiaSB+sizeof(int)*2,bitmap->bitarray,cantidadBloques/8);

                metadata = config_create(path);
                actualizarSize(metadata, posicion, 0);
                actualizarBlockCount(metadata,0);
                config_destroy(metadata);
                actualizarBlocks(bloqueABorrar, 0,path);
                tamStrBorrar -= posicion;
                // log_info(logger, "Tamanio string luego de borrado es:%d", tamStrBorrar);

            }else if(posicion < tamStrBorrar){ 
                // log_info(logger, "El tamanio que hay en el bloque es menor a lo que yo quiero borrar");

                memset(copiaBlocks + bloqueABorrar*tamanioBloque,' ', posicion); //borro esos 5 limpio todo
                tamStrBorrar -= posicion; //nuevo tamanio--> 1

                bitarray_clean_bit(bitmap,bloqueABorrar);
                memcpy(copiaSB+sizeof(int)*2,bitmap->bitarray,cantidadBloques/8);

                metadata = config_create(path);
                actualizarSize(metadata, posicion, 0);
                actualizarBlockCount(metadata,0);
                config_destroy(metadata);
                actualizarBlocks(bloqueABorrar, 0,path);
                // log_info(logger, "Tamanio string luego de borrado es:%d", tamStrBorrar);

            }else{
                // princio + el bloque + la posicion a borrar los recursos
                memset(copiaBlocks + (bloqueABorrar*tamanioBloque) + (tamanioBloque - tamStrBorrar),' ', tamStrBorrar);
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