#include "./headers/actualizarMetadata.h"


char* crearNuevaListaBloques(char* listaVieja,int bloqueAgregar){
    int tamListaVieja = strlen(listaVieja);
    char *bloque = string_itoa(bloqueAgregar);
    char* listaNueva;

    listaNueva = malloc(tamListaVieja);
    memcpy(listaNueva, listaVieja, tamListaVieja - 1); //copio hasta ']'
    listaNueva[tamListaVieja - 1] = '\0';

    if (tamListaVieja > 2) {
        string_append_with_format(&listaNueva, ",%s]", bloque);
    } else {
        string_append_with_format(&listaNueva, "%s]", bloque);
    }

    free(bloque);
    
    return listaNueva;
}

void actualizarBlocks(t_config* metadataBitacora,int bloque,t_log*log){
    char* lista = config_get_string_value(metadataBitacora,"BLOCKS"); 

    log_info(log,"[ActualizarBlocks] Muestro lista levantada vieja de metadata:%s\t",lista);
    char* bloquesNuevos = crearNuevaListaBloques(lista,bloque);
    log_info(log,"\n[ActualizarBlocks] Lista de bloques nuevos: %s\n",bloquesNuevos);

    config_set_value(metadataBitacora,"BLOCKS",bloquesNuevos);
    config_save(metadataBitacora);

    free(bloquesNuevos);
}

int setearMD5(char* pathMetadata, t_log* log){
    char *comando = string_new();
    string_append(&comando, "md5sum ");
    string_append(&comando, pathMetadata);
    return system(comando);
}

void actualizarBlockCount(t_config* metadataBitacora,t_log*log){
    int blockCount = config_get_int_value(metadataBitacora,"BLOCK_COUNT");
    blockCount += 1;

    //vuelvo a convertir en str de nuevo asi lo vuelvo a pegar
    char* str_blockCount = string_new();
    char* temp = string_itoa(blockCount);
    string_append(&str_blockCount, temp);
    free(temp);
    config_set_value(metadataBitacora,"BLOCK_COUNT",str_blockCount);
    free(str_blockCount);
    config_save(metadataBitacora);
}

void actualizarSize(t_config* metadataBitacora,t_log*log){
    int size = config_get_int_value(metadataBitacora,"SIZE");
    size += tamanioBloque;

    //vuelvo a convertir en str de nuevo asi lo vuelvo a pegar
    char* str_size = string_new();
    char* temp = string_itoa(size);
    string_append(&str_size, temp);
    free(temp);
    config_set_value(metadataBitacora,"SIZE",str_size);
    free(str_size);
    config_save(metadataBitacora);

}