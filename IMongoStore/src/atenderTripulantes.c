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


                //CREO EL STRING A GUARDAR "SE MUEVE DE X|Y a X'|Y'"
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

                int tamStr = string_length(temporal);

                printf("-------------------------------------------------------");
                printf("\n El string a pegar es: %s\n", temporal);
                printf("-------------------------------------------------------\n");


                //CREO EL PATH ../BITACORAS/TRIPULANTEN.ims
                char* path_fileTripulante = string_new();
                string_append(&path_fileTripulante,"/Bitacoras/");
                string_append(&path_fileTripulante,"Tripulante");
                posicion = string_itoa(idTripulante);
                string_append(&path_fileTripulante,posicion);
                free(posicion);
                string_append(&path_fileTripulante,".ims");

                char* pathCompleto = string_new();
                string_append(&pathCompleto,datosConfig->puntoMontaje);
                string_append(&pathCompleto,path_fileTripulante);
                

                if(access(pathCompleto,F_OK) < 0){
                    log_info(logger,"No existe archivo en bitácora..Se crea archivo para este tripulante...");
                    crearMetadataBitacora(pathCompleto,logger);
                
                    int cantidadBloquesAUsar = cantidad_bloques(temporal,logger);
                    int cantidadBloquesUsados = 0;
                    int contadorChars = 0;

                    err = validarBitsLibre(cantidadBloquesAUsar,logger);
                    if(err < 0){
                        log_error(logger, "No existe más espacio para guardar en filesystem");
                    }

                    // for(int i=0; i < bitarray_get_max_bit(bitmap) && cantidadBloquesUsados != cantidadBloquesAUsar; i++){
                    //     if(bitarray_test_bit(bitmap,i) == 0){
                    //         if((cantidadBloquesAUsar-cantidadBloquesUsados)==1){ //ultimo bloque a escribir
                    //             //POSIBLE FRAGMENTACION INTERNA!
                                
                    //             memcpy(blocks_memory + i*tamanioBloque,temporal+contadorChars*tamanioBloque,tamStr-contadorChars*tamanioBloque);
                    //             contadorChars ++;
                    //             cantidadBloquesUsados ++;
                                
                    //             //Actualizo metadata
                    //             t_config* bitacoraTripulante = config_create(pathCompleto(path_fileTripulante));
                            
                    //             int size = atoi(dictionary_get(bitacoraTripulante, "SIZE"));
                    //             size += tamanioBloque;
                    //             char* c_size = string_new();
                    //             strcpy(c_size,string_itoa(size));
                    //             dictionary_put(bitacoraTripulante,"SIZE",c_size);
                    //             free(c_size);

                    //             config_save_in_file(bitacoraTripulante,pathCompleto(path_fileTripulante));
                    //             config_destroy(bitacoraTripulante); 
                    //             //falta BLOCKS =[]

                    //         }else{
                            
                    //             //Escribo en bloque
                    //             memcpy(blocks_memory + i*tamanioBloque,temporal+contadorChars*tamanioBloque,tamanioBloque);
                    //             contadorChars ++;
                    //             cantidadBloquesUsados ++;

                    //             //Actualizo metadata
                    //             t_config* bitacoraTripulante = config_create(pathCompleto(path_fileTripulante));
                    //             int size = atoi(dictionary_get(bitacoraTripulante, "SIZE"));
                    //             size += tamanioBloque;
                    //             char* c_size = string_new();
                    //             strcpy(c_size,string_itoa(size));
                    //             dictionary_put(bitacoraTripulante,"SIZE",c_size);
                    //             free(c_size);


                    //             //falta BLOCKS =[]
                    //             config_save_in_file(bitacoraTripulante,"bitacora.ims");
                    //             config_destroy(bitacoraTripulante);

                    //         }
                    //     }
                }
                free(pathCompleto);
                free(path_fileTripulante);
                free(temporal);
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

void crearMetadataBitacora(char* path_tripulante, t_log* logger){
    //CREA ARCHIVO DE BITACORA (METADATA) Y LO SETEA EN 0
    int fd = creat(path_tripulante,0666);
    if(fd < 0){
        perror("Error:");
    }else{
        close(fd);
        //Lo creo de tipo clave-valor
        t_config* bitacoraTripulante = config_create(path_tripulante);

        dictionary_put(bitacoraTripulante->properties, "SIZE", "0");
        dictionary_put(bitacoraTripulante->properties, "BLOCK_COUNT", "0");
        dictionary_put(bitacoraTripulante->properties, "BLOCKS", "[]");
        dictionary_put(bitacoraTripulante->properties, "MD5_ARCHIVO", "0");


        //LO GUARDO EN EL ARCHIVO EN SI Y LO DESTRUYO A LA VARIABLE //falta config_destroy[ TIRA ERROR]
        config_save_in_file(bitacoraTripulante,path_tripulante);
        log_info(logger,"hola1");

        config_destroy(bitacoraTripulante);

    }
}


int cantidad_bloques(char* string, t_log* logger){
    double cantidad;
    double tamanioString = string_length(string);
    cantidad = tamanioString / cantidadBloques;
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

