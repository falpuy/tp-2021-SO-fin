#include "./headers/guardarBlocks.h"


void guardarEnBlocks(char* stringGuardar,char* path_fileTripulante,int flag,t_log* logger){ 
    //agregar checkeo de si en el ultimo bloque anterior hay fragmentacion interna debo rellenarla

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
            
            pthread_mutex_lock(&m_bitmap);
            bitarray_set_bit(bitmap,i);
            pthread_mutex_unlock(&m_bitmap);


            if((cantidadBloquesAUsar-cantidadBloquesUsados)==1){//ultimo bloque a escribir - posible fragmentación interna
                //Ejemplo-->"ME ||MUE||VO ||DE ||3|5|| a ||3|4||" --> bloques de 3 bytes
                
                //Me muevo al bloque en si a guardar | pego en string moviendome hasta donde guarde antes | Pego lo que me queda del string--> tamañoTotalStr - posicionAntEnStr*tamanioBloque
                pthread_mutex_lock(&m_blocks);
                memcpy(blocks_memory + i*tamanioBloque,stringGuardar+posEnString*tamanioBloque,tamStr-posEnString*tamanioBloque);
                pthread_mutex_unlock(&m_blocks);
                
                posEnString ++;                
                
                //--------------------------ACTUALIZO METADATA---------------------------
                pthread_mutex_lock(&m_metadata);
                t_config* metadata = config_create(path_fileTripulante);

                actualizarSize(metadata,logger);
                actualizarBlocks(metadata,i,logger);
                if(flag){
                    actualizarBlockCount(metadata,logger);
                    //setearMD5(path_fileTripulante,logger);
                }
                config_destroy(metadata);
                pthread_mutex_unlock(&m_metadata);
                
                cantidadBloquesUsados ++;

                
            }else{
                //Me muevo al bloque en si a guardar | pego en string moviendome hasta donde guarde antes | Pego todo el tamaño del bloque
                
                pthread_mutex_lock(&m_blocks);
                memcpy(blocks_memory + i*tamanioBloque,stringGuardar+posEnString*tamanioBloque,tamanioBloque);
                pthread_mutex_unlock(&m_blocks);
                
                posEnString ++;

                //--------------------------Actualizo metadata---------------------------
                pthread_mutex_lock(&m_metadata);
                t_config* metadata = config_create(path_fileTripulante);

                actualizarSize(metadata,logger);
                actualizarBlocks(metadata,i,logger);
                
                if(flag){
                    actualizarBlockCount(metadata,logger);
                    //setearMD5(path_fileTripulante,logger);
                }

                config_destroy(metadata); 
                pthread_mutex_unlock(&m_metadata);

                cantidadBloquesUsados ++;
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
        
        log_error(log, "Eliminando oxigenos..");
        
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

                actualizarSize(metadata,logger);
                actualizarBlocks(metadata,i,logger);
                
                if(flag){
                    actualizarBlockCount(metadata,logger);
                    //setearMD5(path_fileTripulante,logger);
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