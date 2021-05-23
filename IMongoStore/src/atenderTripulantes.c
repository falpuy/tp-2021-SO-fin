#include "./headers/atenderTripulantes.h"

void handler(int client, char* identificador, int comando, void* payload, t_log* logger){

log_info(logger,"LLegó nuevo comando: %d", comando);
    
    switch(comando){
        case OBTENER_BITACORA: //760
            log_info(logger, "Llego comando: Obtener bitácora.....");
            break;

        case MOVER_TRIPULANTE:
            log_info(logger, "Llego comando: Mover Tripulante.....");
            
            int offset = 0, idTripulante,posX_v,posY_v, posX_n, posY_n;

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

            //CREO EL STRING A GUARDAR "SE MUEVE DE X|Y a X'|Y'"
            char* str_para_blocks = string_new();
            string_append(&str_para_blocks,"Se mueve de ");
            string_append(&str_para_blocks,string_itoa(posX_v));
            string_append(&str_para_blocks,"|");
            string_append(&str_para_blocks,string_itoa(posY_v));
            string_append(&str_para_blocks," a ");
            string_append(&str_para_blocks,string_itoa(posX_n));
            string_append(&str_para_blocks,"'|");
            string_append(&str_para_blocks,string_itoa(posY_n));
            string_append(&str_para_blocks,"'");

            //CREO EL PATH ../BITACORAS/TRIPULANTEN.ims
            char* path_fileTripulante = string_new();
            string_append(&path_fileTripulante,"Bitacoras/");
            string_append(&path_fileTripulante,"Tripulante");
            string_append(&path_fileTripulante,string_itoa(idTripulante));
            string_append(&path_fileTripulante,".ims");


            if(access(path(path_fileTripulante),F_OK) < 0){
                log_info(logger,"No existe archivo en bitácora..Se crea archivo para este tripulante...");

                //CREA ARCHIVO DE BITACORA (METADATA) Y LO SETEA EN 0
                FILE* creaFile = fopen(path(path_fileTripulante),"w");
                fclose(creaFile);

                //LO CREO TIPO CLAVE_VALOR
                t_config* bitacoraTripulante = config_create(path(path_fileTripulante));
                dictionary_put(bitacoraTripulante->properties, "SIZE", "0");
                dictionary_put(bitacoraTripulante->properties, "BLOCK_COUNT", "0");
                dictionary_put(bitacoraTripulante->properties, "BLOCKS", "[]");
                dictionary_put(bitacoraTripulante->properties, "MD5_ARCHIVO", "BD1014D173BA92CC014850A7087E254E");
                //LO GUARDO EN EL ARCHIVO EN SI Y LO DESTRUYO A LA VARIABLE //falta config_destroy[ TIRA ERROR]
                config_save_in_file(bitacoraTripulante,path(path_fileTripulante));
                config_destroy(bitacoraTripulante);

                int cantidadBloquesAUsar = cantidad_bloques(str_para_blocks);
                int size=0, block_count = 0,blocks[cantidadBloquesAUsar];
                
                
                while(cantidadBloquesAUsar != 0 ){
                    for(int i=0; i < cantidadBloques; i++){
                        if(bitarray_test_bit(bitmap,i) == 0){ 
                            //Si hay bit libre en bitmap..

                            int bloque = i,contador=0;
                            char* str_copiar = string_new();

                            //[TODO]--> pensar como copiar hasta x cantidad del string anterior

                            FILE* bloques = fopen("../Filesystem/Blocks.ims", "w");
                            fseek(bloques,tamanioBloque*i,SEEK_SET);
                            fwrite(&str_copiar,sizeof(str_copiar),1,bloques);
                            fclose(bloques);

                            //Cambio variables para luego agregar en el metadata despues
                            contador++;
                            block_count +=1;
                            block[contador] = i;
                            if(cantidadBloquesAUsar-block_count != 0){ //Si no es el ultimo bloque -->seteo todo, sino fragmentacion
                                size += tamanioBloque;
                            }else{
                                //sumar solo los ultimos caracteres
                            }

                            
                        }                    
                    }
                } 
                free(str_copiar);

                //Actualizo metadata luego de guardar en cada block
                t_config* bitacoraTripulante = config_create(path(path_fileTripulante));
                dictionary_put(bitacoraTripulante->properties, "SIZE", string_itoa(size));
                dictionary_put(bitacoraTripulante->properties, "BLOCK_COUNT",string_itoa(block_count) );
                
                //CREO LA LISTA DE BLOQUES
                char* ponerFormaListaBloques = string_new();
                string_append(&ponerFormaListaBloques,"[");
                
                for(int i=0; i<cantidadBloquesAUsar; i++){
                    string_append(&ponerFormaListaBloques,string_itoa(block[i]));
                    string_append(&ponerFormaListaBloques,",");
                }
                string_append(&ponerFormaListaBloques,"]");
                
                dictionary_put(bitacoraTripulante->properties, "BLOCKS",ponerFormaListaBloques);
                config_destroy(bitacoraTripulante);
                free(ponerFormaListaBloques);

            }else{
                //checkear que bloques estan libres por bitmap
                //ir guardando en cada bloque libre
                //actualizar la bitacora
            }

            free(path_fileTripulante);
            free(str_para_blocks);
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

char* path(char* str){
    char* path = string_from_format("%s/%s",datosConfig->puntoMontaje,str);
    return path;
}

int cantidad_bloques(char* string){
    double cantidad;
    double tamanioString = string_length(string);
    double cantidadBloques = 8; //USAR VAR GLOBAL
    cantidad = tamanioString / cantidadBloques;
    int valorFinal = (int) ceil(cantidad);
    return valorFinal; //round up
}