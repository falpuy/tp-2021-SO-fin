#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unnamed/socket.h>
#include <unnamed/serialization.h>
#include <commons/log.h>
#include <string.h>
#include <unistd.h>
#include <commons/string.h>
#include <stdint.h>
#include <commons/bitarray.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdarg.h>
#include <sys/types.h>
#include <dirent.h>
#include <commons/process.h>
#include <pthread.h>
#include <semaphore.h>
#include <commons/collections/queue.h>
#include <commons/config.h>
#include <math.h>

uint32_t tamanioBloque = 4;
uint32_t cantidadBloques = 64;
void* copiaBlocks;
t_bitarray* bitmap;
void* memBitmap;
void* copiaSB;
t_log* logger;


char* crearNuevaListaBloques(char* listaVieja,int bloqueAModificar, int flagEsGuardar){
    int tamListaVieja = string_length(listaVieja);
    char *bloque = string_itoa(bloqueAModificar);
    
    char* listaNueva  = malloc(tamListaVieja);
    memcpy(listaNueva, listaVieja, tamListaVieja - 1); //copio hasta ']'
    listaNueva[tamListaVieja - 1] = '\0';

    if(flagEsGuardar){
        if (tamListaVieja > 2) { // [2,3,5
            string_append_with_format(&listaNueva, ",%s]", bloque);
        } 
        else{
            string_append_with_format(&listaNueva, "%s]", bloque);
        }
    }else{ //Es para borrar
        if (tamListaVieja > 2) { //[2,44

            int tamListaNueva = string_length(listaNueva);
            int aux = tamListaNueva;
            
            while(aux != 0 && memcmp(listaNueva + aux, ",", 1)) { 
                aux--; 
            }

            char* listaTemporal = malloc(aux+2);
            memcpy(listaTemporal, listaNueva,aux);//copio hasta antes del numero y su coma
            listaTemporal[aux] = ']';
            listaTemporal[aux+1] = '\0';
            //string_append(&listaTemporal, "]"); 
            free(listaNueva);

            //vuelvo a crear la listaNueva para poder retornarla
            listaNueva = malloc(aux+2);
            memcpy(listaNueva, listaTemporal,aux+1);
            listaNueva[aux+1] = '\0';
            free(listaTemporal);


        } else {//[X
            char* listaTemporal = malloc(strlen(listaNueva));
            int tamListaNueva = strlen(listaNueva);
            memcpy(listaTemporal, listaNueva,tamListaNueva  - 1);//copio hasta antes del numero
            listaTemporal[tamListaNueva- 1] = '\0';
            string_append(&listaTemporal, "]");
            free(listaNueva);
            
            //vuelvo a crear la listaNueva para poder retornarla
            listaNueva = malloc(string_length(listaTemporal)+1);
            memcpy(listaNueva, listaTemporal,string_length(listaTemporal));
            listaNueva[string_length(listaTemporal)] = '\0';
            free(listaTemporal);
        }
    }
    free(bloque);
    return listaNueva;
}

void actualizarBlocks(t_config* metadataBitacora,int bloque,int flagEsGuardar){
    char* lista = config_get_string_value(metadataBitacora,"BLOCKS"); 
    char* bloquesNuevos = crearNuevaListaBloques(lista,bloque,flagEsGuardar);

    config_set_value(metadataBitacora,"BLOCKS",bloquesNuevos);
    config_save(metadataBitacora);

    free(bloquesNuevos);
}

void setearMD5(char* pathMetadata){
    char *comando = string_new();

    t_config* metadata = config_create(pathMetadata);
    char** listaBloques = config_get_array_value(metadata,"BLOCKS");
    int contador = 0;
    char* string_temp = string_new();
    int bloquesHastaAhora = 0;
    int bloque;

    while(listaBloques[contador]){ 
        contador++;
    }

    for(int i = 0; i < contador; i++){
        if((contador - bloquesHastaAhora) != 0){ //no es el ultimo bloque-->no hay frag. interna
            
            bloque = atoi(listaBloques[bloquesHastaAhora]);
            char* temporalBloque = malloc(tamanioBloque+1);
            memcpy(temporalBloque, copiaBlocks + bloque*tamanioBloque, tamanioBloque);
            temporalBloque[tamanioBloque] = '\0';
                
            string_append(&string_temp,temporalBloque);
            bloquesHastaAhora++;
            free(temporalBloque);
        }else{
            bloque = atoi(listaBloques[bloquesHastaAhora]);

            int sizeVieja = config_get_int_value(metadata, "SIZE");
            int fragmentacion = contador*tamanioBloque - sizeVieja;

            char* temporalBloque = malloc(fragmentacion+1);
            memcpy(copiaBlocks + bloque*tamanioBloque, temporalBloque, fragmentacion);
            temporalBloque[fragmentacion] = '\0';
                
            string_append(&string_temp,temporalBloque);
            free(temporalBloque);
        }
            
    }
    log_info(logger, "String total para el MD5:%s", string_temp);
    
    FILE* archivo = fopen("temporal.txt","w");
    fprintf(archivo,"%s",string_temp);
    fclose(archivo);

    string_append(&comando, "md5sum temporal.txt > resultado.txt");
    system(comando);

    char* md5 = malloc(33); //32 + \0

    FILE* archivo2 = fopen("resultado.txt","r");
    fscanf(archivo2,"%s",md5);
    md5[33] = '\0';
    fclose(archivo2);

    int err = remove("temporal.txt");
    if (err < 0){
        log_error(logger, "Error al remover archivo temporal.txt");
    }
    
    err = remove("resultado.txt");
    if(err < 0 ){
        log_error(logger, "Error al remover archivo resultado.txt");
    }

    config_set_value(metadata,"MD5",md5);
    config_save(metadata);
    
    for(int i = 0; i < contador; i++){
        free(listaBloques[i]);
    }
    free(listaBloques);
    free(md5);
    free(string_temp);
    free(comando);
    config_destroy(metadata);
}

void actualizarBlockCount(t_config* metadataBitacora,int flagEsGuardar){
    int blockCount = config_get_int_value(metadataBitacora,"BLOCK_COUNT");
    if(flagEsGuardar){
        blockCount += 1;
    }else{
        blockCount -= 1;
    }
    //vuelvo a convertir en str de nuevo asi lo vuelvo a pegar
    char* str_blockCount = string_new();
    char* temp = string_itoa(blockCount);
    string_append(&str_blockCount, temp);
    free(temp);
    config_set_value(metadataBitacora,"BLOCK_COUNT",str_blockCount);
    free(str_blockCount);
    config_save(metadataBitacora);
}

void actualizarSize(t_config* metadataBitacora,int tamanio, int flagEsGuardar){
    
    int size = config_get_int_value(metadataBitacora,"SIZE");
    if(flagEsGuardar){
        size += tamanio;
    }else{
        size -= tamanio;
    }
    //vuelvo a convertir en str de nuevo asi lo vuelvo a pegar
    char* str_size = string_new();
    char* temp = string_itoa(size);
    string_append(&str_size, temp);
    free(temp);
    config_set_value(metadataBitacora,"SIZE",str_size);
    free(str_size);
    config_save(metadataBitacora);
}


void crearMetadataFiles(char* path,char* charLlenado){
    int fd = creat(path,0666);
    if(fd < 0){
        perror("Error:");
    }else{
        close(fd);
        t_config* file = config_create(path);
        
        config_set_value(file, "SIZE", "0");
        config_set_value(file, "BLOCK_COUNT", "0");
        config_set_value(file, "BLOCKS", "[]");
        config_set_value(file, "CARACTER_LLENADO", charLlenado);
        config_set_value(file, "MD5", "0");

        config_save_in_file(file,path);
        config_destroy(file);

    }
}

void crearMetadataBitacora(char* path){
    int fd = creat(path,0666);
    if(fd < 0){
        perror("Error:");
    }else{
        close(fd);
        t_config* bitacoraTripulante = config_create(path);
        
        config_set_value(bitacoraTripulante, "SIZE", "0");
        config_set_value(bitacoraTripulante, "BLOCKS", "[]");

        config_save_in_file(bitacoraTripulante,path);
        config_destroy(bitacoraTripulante);

    }
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



int validarBitsLibre(int cantidadBloquesAUsar){
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


int cantidad_bloques(char* string){
    double cantidad;
    double tamanioString = string_length(string);
    cantidad = tamanioString / tamanioBloque;
    int valorFinal = (int) ceil(cantidad);

    return valorFinal; //round up
}
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

int main(){
    logger = log_create("test.log","test",1,LOG_LEVEL_INFO);

    //--------------------------SUPERBLOQUE -------------

    int superBloque = open("SuperBloque.ims", O_CREAT | O_RDWR,0664);
        
    if(superBloque<0){
        log_error(logger, "Error al abrir/crear Superbloque");
    }
        
    posix_fallocate(superBloque, 0, sizeof(uint32_t) * 2 + cantidadBloques / 8);
        
    copiaSB = malloc(tamanioBloque*cantidadBloques + cantidadBloques/8);
    
    void* sb_memoria = (char*) mmap(NULL, sizeof(uint32_t) * 2, PROT_READ | PROT_WRITE, MAP_SHARED, superBloque, 0);
    memBitmap = malloc(cantidadBloques/8);
    bitmap = bitarray_create_with_mode((char*)memBitmap, cantidadBloques / 8, MSB_FIRST);  
    
    for(int i=0; i<cantidadBloques; i++){
        bitarray_clean_bit(bitmap,i);    
    }
        
    log_info(logger, "Muestro mis valores del bitmap..");
    for(int i=0; i<cantidadBloques; i++){
        printf("%d",bitarray_test_bit(bitmap,i));
    }   

    memcpy(copiaSB, &tamanioBloque, sizeof(uint32_t));
    memcpy(copiaSB + sizeof(uint32_t), &cantidadBloques, sizeof(uint32_t));
    memcpy(copiaSB + sizeof(uint32_t)*2,bitmap->bitarray, cantidadBloques/8);

    memcpy(sb_memoria,copiaSB,2*sizeof(uint32_t)+cantidadBloques/8);
        
    int err = msync(sb_memoria, 2*sizeof(uint32_t) + cantidadBloques/8, MS_SYNC);
    if(err == -1){
        log_error(logger, "[SuperBloque] Error de sincronizar a disco SuperBloque");
    }

    close(superBloque);
    err = munmap(sb_memoria, sizeof(uint32_t)*2);
    if (err == -1){
        log_error(logger, "[SuperBloque] Error al liberal la memoria mapeada de tamañoBloque y cantidadBloque");
    }

    close(superBloque);

    log_info(logger, "-----------------------------------------------------");
    log_info(logger, "Se creó archivo superBloque.ims");
    log_info(logger, "-----------------------------------------------------");


    //--------------------------BLOCKS -------------
    int blocks = open("Blocks.ims", O_CREAT | O_RDWR, 0664);
        
    int tamanioAGuardar = (tamanioBloque * cantidadBloques);
    copiaBlocks = malloc(tamanioBloque* cantidadBloques);
    memset(copiaBlocks,' ',tamanioBloque* cantidadBloques);
    
    posix_fallocate(blocks, 0, tamanioAGuardar);        
    close(blocks);
    

    log_info(logger, "-----------------------------------------------------");
    log_info(logger, "Se creó archivo Blocks.ims");
    log_info(logger, "-----------------------------------------------------");

    char* str = string_new();
    string_append(&str, "Hola como estas? Soy Delfina Bibe");
    int tam = string_length(str);
    crearMetadataBitacora("Test.ims");
    guardarEnBlocks(str,"Test.ims",0);
    msync(copiaBlocks,tamanioBloque * cantidadBloques,0);

    //Ahora tengo que checkear la idea de sabotajes










    return 0;
}

