#include "./headers/atenderTripulantes.h"

/*
    TAREAS QUE ME PUEDEN LLEGAR
        GENERAR_OXIGENO 12;2;3;5
        CONSUMIR_OXIGENO 120;2;3;1
        GENERAR_COMIDA 4;2;3;1
        CONSUMIR_COMIDA 1;2;3;4
        GENERAR_BASURA 12;2;3;5
        DESCARTAR_BASURA 0;3;1;7

    METADATA FILES
        SIZE=132
        BLOCK_COUNT=3
        BLOCKS=[1,2,3]
        CARACTER_LLENADO=O
        MD5_ARCHIVO=BD1014D173BA92CC014850A7087E254E --> esto se tiene que actualizar cada vez q se cambia

    METADATA BITACORA
        SIZE=140
        BLOCKS=[4,8,2]

        BITACORAS --
            Se mueve de X|Y a X’|Y’
            Comienza ejecución de tarea X
            Se finaliza la tarea X
            Se corre en pánico hacia la ubicación del sabotaje
            Se resuelve el sabotaje

*/






void handler(int client, char* identificador, int comando, void* payload, t_log* logger){

    log_info(logger,"LLegó nuevo comando: %d", comando);
        
        switch(comando){
            case OBTENER_BITACORA: //760
                log_info(logger, "Llego comando: Obtener bitácora.....");
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
                string_append(&temporal,"'|");
                posicion = string_itoa(posY_n);
                string_append(&temporal,posicion);
                string_append(&temporal,"'");
                free(posicion);

                printf("-------------------------------------------------------");
                printf("\n El string a pegar es: %s\n", temporal);
                printf("-------------------------------------------------------");


                //CREO EL PATH ../BITACORAS/TRIPULANTEN.ims
                char* path_fileTripulante = string_new();
                string_append(&path_fileTripulante,"Bitacoras/");
                string_append(&path_fileTripulante,"Tripulante");
                posicion = string_itoa(idTripulante);
                string_append(&path_fileTripulante,posicion);
                free(posicion);
                string_append(&path_fileTripulante,".ims");
               
                //char* pathAbsoluto = string_from_format("%s/%s",datosConfig->puntoMontaje,path_fileTripulante);
                
                if(access(pathCompleto(path_fileTripulante),F_OK) < 0){
                    log_info(logger,"No existe archivo en bitácora..Se crea archivo para este tripulante...");
                    printf("\n%s\n", pathCompleto(path_fileTripulante));
                    //crearMetadataBitacora(pathCompleto(path_fileTripulante));
                }

                    // int cantidadBloquesAUsar = cantidad_bloques(str_para_blocks);
                    // int cantidadBloquesUsados = 0;

                    // err = validarBitsLibre(cantidadBloquesAUsar,);
                    // if(err < 0){
                    //     log_error(log, "No existe más espacio para guardar en filesystem");
                    // }

                    // for(int i=0; i < bitarray_get_max_bit(bitmap) && cantidadBloquesUsados != cantidadBloquesAUsar; i++){
                    //     if(bitarray_test_bit(bitmap,i) == 0){
                    //         if((cantidadBloquesAUsar-cantidadBloquesUsados)==1){ //ultimo bloque a escribir
                    //             //POSIBLE FRAGMENTACION INTERNA!



                    //         }else{
                    //             memcpy(blocks_memory + i*tamanioBloque,str_para_blocks,tamanioBloque);
                    //             t_config* bitacoraTripulante = config_create(path_tripulante);
                                
                    //             int size = atoi(dictionary_get(bitacoraTripulante, "SIZE"));
                    //             size += tamanioBloque;

                    //             dictionary_remove_and_destroy(bitacoraTripulante,"SIZE",data_destroyer);
                    //             dictionary_put(bitacoraTripulante->properties, "SIZE", string_itoa(size));


                    //             // dictionary_remove_and_destroy(bitacoraTripulante,"BLOCKS",data_destroyer);
                    //             // dictionary_put(bitacoraTripulante->properties, "BLOCKS", );
                    //             // dictionary_put(bitacoraTripulante->properties, "BLOCKS", "[]");
                    //             // dictionary_put(bitacoraTripulante->properties, "MD5_ARCHIVO", "-");
                                
                    //             //LO GUARDO EN EL ARCHIVO EN SI Y LO DESTRUYO A LA VARIABLE
                    //             config_save_in_file(bitacoraTripulante,path_tripulante);
                    //             config_destroy(bitacoraTripulante);

                    //         }
                    //     }
                    //}

                    //Actualizar cantidadBloques --> en config;




                // }else{
                //     //checkear que bloques estan libres por bitmap
                //     //ir guardando en cada bloque libre
                //     //actualizar la bitacora
                // }

                free(path_fileTripulante);
                //free(pathAbsoluto);
                free(temporal);
                break;

        
            
            case COMIENZA_EJECUCION_TAREA:
                log_info(logger, "Llego comando: Comienza ejecucion de tarea.....");

            //TAREA PARAMETROS;POS X;POS Y;TIEMPO
            //GENERAR_OXIGENOS 12; 3,2,7

            //logeo en bitacora
            //logeo en files

                log_info(logger,"buenas");
                //Escribir en Bitacora
                //Escribir en la tarea.ims
                log_info(logger,"%s",payload);
                break;
            
            case FINALIZA_TAREA: // idTarea
                log_info(logger, "Llego comando: Finaliza tarea.....");

                //Escribir en Bitacora
                //Escribir en la tarea.ims
                log_info(logger,"%s",payload);

                break;
            
            case ATENDER_SABOTAJE: //hacer conjunto a sabotaje
                log_info(logger, "Llego comando: Atender Sabotaje.....");

                //Escribir en Bitacora
                //protocolo fsck?
                log_info(logger,"%s",payload);

                break;
            
            case RESUELTO_SABOTAJE:
                log_info(logger, "Llego comando:Se resolvio Sabotaje.....");

                //escribir en bitacora
                log_info(logger,"%s",payload);

                break;
    }
   
}

void crearMetadataBitacora(char* path_tripulante){
    //CREA ARCHIVO DE BITACORA (METADATA) Y LO SETEA EN 0
    FILE* creaFile = fopen(path_tripulante,"w");
    fclose(creaFile);

    //Lo creo de tipo clave-valor
    t_config* bitacoraTripulante = config_create(path_tripulante);
    dictionary_put(bitacoraTripulante->properties, "SIZE", "0");
    dictionary_put(bitacoraTripulante->properties, "BLOCK_COUNT", "0");
    dictionary_put(bitacoraTripulante->properties, "BLOCKS", "[]");
    dictionary_put(bitacoraTripulante->properties, "MD5_ARCHIVO", "-");
    
    //LO GUARDO EN EL ARCHIVO EN SI Y LO DESTRUYO A LA VARIABLE //falta config_destroy[ TIRA ERROR]
    config_save_in_file(bitacoraTripulante,path_tripulante);
    config_destroy(bitacoraTripulante);
}


int cantidad_bloques(char* string){
    double cantidad;
    double tamanioString = string_length(string);
    double cantidadBloques = 8; //USAR VAR GLOBAL
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

char* pathCompleto(const char* str, ...){
	char* temporal = string_from_format("%s/%s", datosConfig->puntoMontaje, str);
	va_list parametros;
	va_start(parametros, str);
	char* pathAbsoluto = string_from_vformat(temporal, parametros);
	va_end(parametros);
	free(temporal);
	return temporal;
}