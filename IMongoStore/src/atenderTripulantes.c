#include "./headers/atenderTripulantes.h"

void handler(int client, char* identificador, int comando, void* payload, t_log* logger){

    log_info(logger,"LLegó nuevo comando: %d", comando);
        
        switch(comando){
            case OBTENER_BITACORA: //760
                log_info(logger,"-----------------------------------------------------");
                log_info(logger, "Llego comando: Obtener bitácora de.....");

                int o_idTripulante;
                memcpy(&o_idTripulante,payload,sizeof(int));
                log_info(logger, "ID Tripulante:         %d", o_idTripulante);
                log_info(logger,"-----------------------------------------------------");

                break;  

            case MOVER_TRIPULANTE:
                log_info(logger,"-----------------------------------------------------");
                log_info(logger, "Llego comando: Mover Tripulante.....");
                
                int idTripulante,posX_v,posY_v, posX_n, posY_n;

                //SEPARO LO QUE ME MANDA
                memcpy(&idTripulante,payload,sizeof(int));
                memcpy(&posX_v,payload + sizeof(int), sizeof(int));
                memcpy(&posY_v,payload + sizeof(int)*2, sizeof(int));
                memcpy(&posX_n,payload + sizeof(int)*3, sizeof(int));
                memcpy(&posY_n,payload + sizeof(int)*4, sizeof(int));

                log_info(logger, "ID Tripulante:         %d", idTripulante);
                log_info(logger, "Posicion Vieja en X:   %d", posX_v);
                log_info(logger, "Posicion Vieja en Y:   %d", posY_v);
                log_info(logger, "Posicion Nueva en X:   %d", posX_n);
                log_info(logger, "Posicion Nueva en Y:   %d", posY_n);
                log_info(logger,"-----------------------------------------------------");


                char* stringGuardar = strMoverTripultante(idTripulante,posX_v,posY_v,posX_n,posY_n);

                printf("-------------------------------------------------------");
                printf("\n El string a pegar es: %s\n", stringGuardar);
                printf("-------------------------------------------------------\n");


                //Creo el path de tripulanteN.ims
                char* tripulante = crearStrTripulante(idTripulante);
                
                char* bitacora = string_new();
                string_append(&bitacora, "Bitacoras/");
                string_append(&bitacora,tripulante);

                char* path_fileTripulante = pathCompleto(bitacora);
                free(bitacora);
                
                if(access(path_fileTripulante,F_OK) < 0){
                    log_info(logger,"No existe archivo en bitácora...\tSe crea archivo para este tripulante...");
                    crearMetadataBitacora(path_fileTripulante,logger);
                }
                guardarEnBlocks(stringGuardar,path_fileTripulante,logger);
                free(tripulante);
                free(path_fileTripulante);
                free(stringGuardar);



            break;

            case COMIENZA_EJECUCION_TAREA: 
                log_info(logger,"-----------------------------------------------------");
                log_info(logger, "Llego comando: Comienza ejecucion de tarea.....");

                int c_tamanioTarea,c_posX,c_posY,c_tiempo;
                int c_offset = 0;
                char* c_tarea;

                //SEPARO LO QUE ME MANDA
                memcpy(&c_tamanioTarea,payload,sizeof(int));
                c_offset += sizeof(int);
                
                c_tarea = malloc(c_tamanioTarea+1);
                memcpy(c_tarea,payload + c_offset,15);
                c_tarea[c_tamanioTarea] = '\0';

                c_offset+= c_tamanioTarea;

                memcpy(&c_posX,payload + c_offset, sizeof(int));
                c_offset += sizeof(int);

                memcpy(&c_posY,payload + c_offset, sizeof(int));
                c_offset += sizeof(int);

                memcpy(&c_tiempo,payload + c_offset, sizeof(int));

                log_info(logger, "Tam tarea:         %d", c_tamanioTarea);
                log_info(logger, "Tarea:         %s", c_tarea);
                log_info(logger, "Posicion en X: %d", c_posX);
                log_info(logger, "Posicion en Y: %d", c_posY);
                log_info(logger, "Tiempo:        %d", c_tiempo);
                log_info(logger,"-----------------------------------------------------");
                break;
            
            case FINALIZA_TAREA: // 
                log_info(logger,"-----------------------------------------------------");
                log_info(logger, "Llego comando: Finaliza tarea.....");

                int f_tamTarea;
                int f_offset = 0;
                char* f_tarea;

                //SEPARO LO QUE ME MANDA
                memcpy(&f_tamTarea,payload,sizeof(int));
                f_offset += sizeof(int);
                
                f_tarea = malloc(f_tamTarea);
                memcpy(f_tarea,payload + sizeof(int),f_tamTarea);
                f_tarea[f_tamTarea] = '\0';
                f_offset+= f_tamTarea;

                log_info(logger, "Tarea:         %s", f_tarea);
                log_info(logger,"-----------------------------------------------------");

                break;
            
            case ATENDER_SABOTAJE:  //idTripulante
                log_info(logger,"-----------------------------------------------------");
                log_info(logger, "Llego comando: Atender Sabotaje.....");

                int a_idTripulante;
                memcpy(&a_idTripulante,payload,sizeof(int));
                log_info(logger, "ID Tripulante:         %d", a_idTripulante);
                log_info(logger,"-----------------------------------------------------");

                break;
            
            case RESUELTO_SABOTAJE: //idTripulante
                log_info(logger,"-----------------------------------------------------");
                log_info(logger, "Llego comando:Se resolvio Sabotaje.....");
                
                int r_idTripulante;
                memcpy(&r_idTripulante,payload,sizeof(int));
                log_info(logger, "ID Tripulante:         %d", r_idTripulante);
                log_info(logger,"-----------------------------------------------------");
                break;
    }
   
}

void guardarEnBlocks(char* stringGuardar,char* path_fileTripulante,t_log* logger){
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
            if((cantidadBloquesAUsar-cantidadBloquesUsados)==1){//ultimo bloque a escribir - posible fragmentación interna
                //Ejemplo-->"ME ||MUE||VO ||DE ||3|5|| a ||3|4||" --> bloques de 3 bytes
                
                //Escribo en bloque:
                //1) Me muevo al bloque en si a guardar
                //2) pego en string moviendome hasta donde guarde antes
                //3) Pego lo que me queda del string--> tamañoTotalStr - posicionAntEnStr*tamanioBloque

                memcpy(blocks_memory + i*tamanioBloque,stringGuardar+posEnString*tamanioBloque,tamStr-posEnString*tamanioBloque);
                posEnString ++;                
                //--------------------------ACTUALIZO METADATA---------------------------
                t_config* bitacoraTripulante = config_create(path_fileTripulante);
                
                //--------------------------------SIZE-----------------------------------
                int size = config_get_int_value(bitacoraTripulante,"SIZE");
                size += tamanioBloque;

                //vuelvo a convertir en str de nuevo asi lo vuelvo a pegar
                char* str_size = string_new();
                char* temp = string_itoa(size);
                string_append(&str_size, temp);
                free(temp);
                config_set_value(bitacoraTripulante,"SIZE",str_size);
                free(str_size);
                config_save(bitacoraTripulante);


                //--------------------------------BLOCK_COUNT-----------------------------------
                int blockCount = config_get_int_value(bitacoraTripulante,"BLOCK_COUNT");
                blockCount += 1;

                //vuelvo a convertir en str de nuevo asi lo vuelvo a pegar
                char* str_blockCount = string_new();
                temp = string_itoa(blockCount);
                string_append(&str_blockCount, temp);
                free(temp);
                config_set_value(bitacoraTripulante,"BLOCK_COUNT",str_blockCount);
                free(str_blockCount);
                config_save(bitacoraTripulante);
                
                    
                //--------------------------------BLOCKS-----------------------------------

                char** listaBloques_ = config_get_array_value(bitacoraTripulante,"BLOCKS");
                char* bloquesNuevos = crearNuevaListaBloques(listaBloques_,cantidadBloquesUsados);
                eliminarLista(listaBloques_,cantidadBloquesUsados);

                config_set_value(bitacoraTripulante,"BLOCKS",bloquesNuevos);
                config_save(bitacoraTripulante);

                cantidadBloquesUsados ++;
                config_destroy(bitacoraTripulante);

                
            }else{
                //Escribo en bloque
                //1) Me muevo al bloque en si a guardar
                //2) pego en string moviendome hasta donde guarde antes
                //3) Pego todo el tamaño del bloque

                memcpy(blocks_memory + i*tamanioBloque,stringGuardar+posEnString*tamanioBloque,tamanioBloque);
                
                posEnString ++;
                    
                //--------------------------Actualizo metadata---------------------------
                t_config* bitacoraTripulante = config_create(path_fileTripulante);

                //--------------------------------SIZE-----------------------------------
                int size = config_get_int_value(bitacoraTripulante,"SIZE");
                size += tamanioBloque;

                //vuelvo a convertir en str de nuevo asi lo vuelvo a pegar
                char* str_size = string_new();
                char* temporal = string_itoa(size);
                string_append(&str_size, temporal);
                free(temporal);
                config_set_value(bitacoraTripulante,"SIZE",str_size);
                free(str_size);
                config_save(bitacoraTripulante);

                //--------------------------------BLOCK_COUNT-----------------------------------
                int blockCount = config_get_int_value(bitacoraTripulante,"BLOCK_COUNT");
                blockCount += 1;

                //vuelvo a convertir en str de nuevo asi lo vuelvo a pegar
                char* str_blockCount = string_new();
                temporal = string_itoa(blockCount);
                string_append(&str_blockCount, temporal);
                free(temporal);
                config_set_value(bitacoraTripulante,"BLOCK_COUNT",str_blockCount);
                free(str_blockCount);
                config_save(bitacoraTripulante);

                //--------------------------------BLOCKS-----------------------------------

                if(cantidadBloquesUsados==0){
                    char* listaBloques = string_new();
                    string_append(&listaBloques,"[");
                    char* posicionLista = string_itoa(i);
                    string_append(&listaBloques,posicionLista);
                    free(posicionLista);
                    string_append(&listaBloques,"]");
                    config_set_value(bitacoraTripulante,"BLOCKS",listaBloques);
                    free(listaBloques);
                    config_save(bitacoraTripulante);
                }else{
                    char** listaBloques_ = config_get_array_value(bitacoraTripulante,"BLOCKS"); //TODO:liberarMemoria
                    char* bloquesNuevos = crearNuevaListaBloques(listaBloques_,cantidadBloquesUsados);
                    eliminarLista(listaBloques_,cantidadBloquesUsados);
                    config_set_value(bitacoraTripulante,"BLOCKS",bloquesNuevos);
                    config_save(bitacoraTripulante);
                }

                //--------------------------------MD5-----------------------------------
                //TODO
                cantidadBloquesUsados ++;
                config_destroy(bitacoraTripulante); 
                
                
            }
        }
    }

}

void eliminarLista (char** listaBloques,int cantidadBloques){
    for(int i=0; i< cantidadBloques; i++){
        free(listaBloques[i]);
    }
    free(listaBloques);
}

char* crearNuevaListaBloques(char** listaBloques,int cantidadBloques){
    char* temporal = string_new();
    for(int i = 0; i < cantidadBloques; i++){
        string_append(&temporal,listaBloques[i]);
        if((cantidadBloques - i)!=1){
            string_append(&temporal,",");
        }
    }
    return temporal;
}



void crearMetadataBitacora(char* path_tripulante, t_log* logger){//CREA ARCHIVO DE BITACORA (METADATA) Y LO SETEA EN 0
    int fd = creat(path_tripulante,0666);
    if(fd < 0){
        perror("Error:");
    }else{
        close(fd);
        //Lo creo de tipo clave-valor
        t_config* bitacoraTripulante = config_create(path_tripulante);
        
        config_set_value(bitacoraTripulante, "SIZE", "0");
        config_set_value(bitacoraTripulante, "BLOCK_COUNT", "0");
        config_set_value(bitacoraTripulante, "BLOCKS", "[]");
        config_set_value(bitacoraTripulante, "MD5_ARCHIVO", "0");


        //LO GUARDO EN EL ARCHIVO EN SI Y LO DESTRUYO A LA VARIABLE //falta config_destroy[ TIRA ERROR]
        config_save_in_file(bitacoraTripulante,path_tripulante);
        config_destroy(bitacoraTripulante);

    }
}


int cantidad_bloques(char* string, t_log* logger){
    double cantidad;
    double tamanioString = string_length(string);
    cantidad = tamanioString / tamanioBloque;
    int valorFinal = (int) ceil(cantidad);

    return valorFinal; //round up
}

int validarBitsLibre(int cantidadBloquesAUsar, t_log* log){
    int contador = 0;

    for(int i=0; i < bitarray_get_max_bit(bitmap); i++){
        if(bitarray_test_bit(bitmap,i) == 0){
            log_info(log, "[ValidarBitsLibres] Se encontró una posicion libre:  %d", i);
            contador++;

            if(contador == cantidadBloquesAUsar){
                return 1;
            }
        }
    }

    return -1;
}


char* strMoverTripultante(int idTripulante,int posX_v,int posY_v,int posX_n,int posY_n){
    char* temporal = string_new();
    char* posicion  = string_itoa(posX_v);
    string_append(&temporal,"Se mueve de ");
    string_append(&temporal,posicion);
    free(posicion);
    string_append(&temporal,"|");
    posicion  = string_itoa(posY_v);
    string_append(&temporal,posicion);
    free(posicion);
    string_append(&temporal," a ");
    posicion  = string_itoa(posX_n);
    string_append(&temporal,posicion);
    free(posicion);
    string_append(&temporal,"|");
    posicion = string_itoa(posY_n);
    string_append(&temporal,posicion);
    free(posicion);
    return temporal;
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

