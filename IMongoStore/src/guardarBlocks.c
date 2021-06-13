#include "./headers/guardarBlocks.h"


void guardarEnBlocks(char* stringGuardar,char* path_fileTripulante,int flag,t_log* logger){
    int tamStr = string_length(stringGuardar);
    int cantidadBloquesAUsar = cantidad_bloques(stringGuardar,logger);
    int cantidadBloquesUsados = 0;
    int posEnString = 0;
    int blockCount = 0;
    char* posicionLista;
                    
    err = validarBitsLibre(cantidadBloquesAUsar,logger);
    if(err < 0){
        log_error(logger, "No existe más espacio para guardar en filesystem");
        log_error(logger,"Finalizando programa...");
        exit(-1);
    }
    
    for(int i=0; i < bitarray_get_max_bit(bitmap) && cantidadBloquesUsados != cantidadBloquesAUsar; i++){
        if(bitarray_test_bit(bitmap,i) == 0){
            
            bitarray_set_bit(bitmap,i);

            if((cantidadBloquesAUsar-cantidadBloquesUsados)==1){//ultimo bloque a escribir - posible fragmentación interna
                //Ejemplo-->"ME ||MUE||VO ||DE ||3|5|| a ||3|4||" --> bloques de 3 bytes
                
                //Me muevo al bloque en si a guardar | pego en string moviendome hasta donde guarde antes | Pego lo que me queda del string--> tamañoTotalStr - posicionAntEnStr*tamanioBloque
                memcpy(blocks_memory + i*tamanioBloque,stringGuardar+posEnString*tamanioBloque,tamStr-posEnString*tamanioBloque);
                posEnString ++;                
                
                //--------------------------ACTUALIZO METADATA---------------------------
                t_config* metadata = config_create(path_fileTripulante);

                actualizarSize(metadata,logger);
                actualizarBlocks(metadata,i,logger);
                if(flag){
                    actualizarBlockCount(metadata,logger);
                    setearMD5(path_fileTripulante,logger);
                }
                cantidadBloquesUsados ++;
                config_destroy(metadata);

                
            }else{
                //Me muevo al bloque en si a guardar | pego en string moviendome hasta donde guarde antes | Pego todo el tamaño del bloque
                memcpy(blocks_memory + i*tamanioBloque,stringGuardar+posEnString*tamanioBloque,tamanioBloque);
                posEnString ++;

                //--------------------------Actualizo metadata---------------------------
                t_config* metadata = config_create(path_fileTripulante);

                actualizarSize(metadata,logger);
                actualizarBlocks(metadata,i,logger);
                
                if(flag){
                    actualizarBlockCount(metadata,logger);
                    setearMD5(path_fileTripulante,logger);
                }

                cantidadBloquesUsados ++;
                config_destroy(metadata); 
            }
        }
    }
}
// -->OOO -->1 bloque
// |OOOO|OOO |hola|como|esta|s?  |



void borrarEnBlocks(char* strABorrar,char* path,int flag,t_log* log){

    /*TODO:
        Si la cantidad a borrar es menor a la que tengo 
            busco por bloque de metadata --> y pego en blocks un string vacio del tamanio del bloque
            actualizo la metadata correspondiente (blocks, block_count, size, md5)
        Borro metadata entera y borro cada bloque de blocks
    */
   
    t_config* metadata = config_create(path);
    char* blocks = config_get_string_value(metadata,"BLOCKS");
    int block_count = config_get_int_value(metadata,"BLOCK_COUNT");
    int cantidadBloquesNecesito = cantidad_bloques(strABorrar, log);
    int tamanioString = strlen(strABorrar);
    
    if(block_count*tamanioBloque > cantidadBloquesNecesito*tamanioBloque){
        log_error(log, "Eliminando oxigenos");

        t_list* listaBloques = crearListaDeBloques(blocks, block_count);
        char* strPegar = string_repeat(" ",tamanioString);
        
        for(int i=0; i < list_size(listaBloques); i++){
            
        }


    }else{

    }


}

t_list* crearListaDeBloques(char* blocks, int block_count){
    //todo
}

int validarBitsLibre(int cantidadBloquesAUsar, t_log* log){
    int contador = 0;

    for(int i=0; i < bitarray_get_max_bit(bitmap); i++){
        if(bitarray_test_bit(bitmap,i) == 0){
            contador++;
            if(contador == cantidadBloquesAUsar){
                return 1;
            }
        }
    }

    return -1;
}



