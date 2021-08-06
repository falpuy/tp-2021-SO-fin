#include "headers/preparacionFS.h"
#include <errno.h>

void inicializacionFS(){
    validarDirectorioFS();
    validarSuperBloque();
    validarBlocks();
}

void validarDirectorioFS(){
    log_info(logger, "Validando Existencia de directorios...");
    DIR* dir = opendir("./Filesystem");
    if(ENOENT == errno){
        log_info(logger, "No existe directorio: Filesystem. Se crea.");
        mkdir("./Filesystem", 0777);
        mkdir("./Filesystem/Bitacoras",0777);
        mkdir("./Filesystem/Files",0777);
        closedir(dir);
    }
    else{
        log_info(logger, "Ya existe directorio");
        closedir(dir);
    }
}

void validarSuperBloque(){
    log_info(logger, "-----------------------------------------------------");
    log_info(logger, "Validando existencia de Superbloque.ims....");
    log_info(logger, "-----------------------------------------------------");

    char* pathSuperBloque = pathCompleto("SuperBloque.ims");

    if(access(pathSuperBloque,F_OK) < 0){
        log_error(logger, "No se encontró archivo SuperBloque.ims. Se crea archivo");
           
        log_info(logger, "Ingresar el tamaño de cada bloque");
        scanf("%d", &tamanioBloque);
        log_info(logger, "Ingresar la cantidad de bloques");
        scanf("%d", &cantidadBloques);

        while (cantidadBloques % 8 != 0) {
            log_error(logger, "La cantidad de bloques debe ser divisible por 8");
            log_info(logger, "Ingresar cantidad de bloques");
            scanf("%d", &cantidadBloques);
        }

        int superBloque = open(pathSuperBloque, O_CREAT | O_RDWR,0664);
        
        if(superBloque<0){
            log_error(logger, "Error al abrir/crear Superbloque");
            perror("El error al abrir:");
        }
        
        posix_fallocate(superBloque, 0, sizeof(uint32_t) * 2 + cantidadBloques / 8);
        
        // copiaSB = malloc(tamanioBloque*cantidadBloques + cantidadBloques/8);
        
        void* sb_memoria = (char*) mmap(NULL, sizeof(uint32_t) * 2 + cantidadBloques / 8, PROT_READ | PROT_WRITE, MAP_SHARED, superBloque, 0);
        if(sb_memoria == MAP_FAILED){
            perror("El error al abrir:");
        }
        memBitmap = malloc(cantidadBloques/8);
        memcpy(memBitmap, sb_memoria + sizeof(uint32_t) * 2, cantidadBloques/8);
        bitmap = bitarray_create_with_mode((char*)memBitmap, cantidadBloques / 8, MSB_FIRST);  

        for(int i=0; i<cantidadBloques; i++){
            bitarray_clean_bit(bitmap,i);    
        }
        
        log_info(logger, "Muestro mis valores del bitmap..");
        for(int i=0; i<cantidadBloques; i++){
            printf("%d",bitarray_test_bit(bitmap,i));

        }   
            
        memcpy(sb_memoria, &tamanioBloque, sizeof(uint32_t));
        memcpy(sb_memoria + sizeof(uint32_t), &cantidadBloques, sizeof(uint32_t));
        memcpy(sb_memoria + sizeof(uint32_t)*2,bitmap->bitarray, cantidadBloques/8);

        // memcpy(sb_memoria,copiaSB,2*sizeof(uint32_t)+cantidadBloques/8);
        
        int err = msync(sb_memoria, 2*sizeof(uint32_t) + cantidadBloques/8, MS_SYNC);
        if(err == -1){
            log_error(logger, "[SuperBloque] Error de sincronizar a disco SuperBloque");
        }

        err = munmap(sb_memoria, sizeof(uint32_t)*2 + cantidadBloques / 8);
        if (err == -1){
            log_error(logger, "[SuperBloque] Error al liberal la memoria mapeada de tamañoBloque y cantidadBloque");
        }

        free(pathSuperBloque);
        close(superBloque);
        free(memBitmap);
        bitarray_destroy(bitmap);
        

        log_info(logger, "-----------------------------------------------------");
        log_info(logger, "Se creó archivo superBloque.ims");
        log_info(logger, "-----------------------------------------------------");

    }else{

        log_info(logger, "Se encontró el archivo superBloque.ims");

        int superBloque = open(pathSuperBloque, O_CREAT | O_RDWR, 0664);
        
        //Mapeo para sacar el tamaño y cantidad solo
        sb_memoria = mmap(NULL, sizeof(uint32_t) * 2 + cantidadBloques/8 , PROT_READ | PROT_WRITE, MAP_SHARED, superBloque, 0);
        memcpy(&tamanioBloque, sb_memoria, sizeof(uint32_t));
        memcpy(&cantidadBloques, sb_memoria + sizeof(uint32_t), sizeof(uint32_t));
        // int err = munmap(sb_memoria, sizeof(uint32_t)*2);

        //Mapeo ahora para sacar el bitmap tmb
        memBitmap = malloc(cantidadBloques/8);
        memcpy(memBitmap, sb_memoria + sizeof(uint32_t)*2, cantidadBloques/8);
        bitmap = bitarray_create_with_mode((char*)memBitmap, cantidadBloques / 8, MSB_FIRST);  
        
        log_info(logger, "Muestro mis valores del bitmap..");
        for(int i=0; i<cantidadBloques; i++){
            printf("%d",bitarray_test_bit(bitmap,i));

        }  

        // copiaSB = malloc(sizeof(uint32_t) * 2 + cantidadBloques/8);
        // memcpy(copiaSB, sb_memoria,sizeof(uint32_t) * 2 + cantidadBloques/8);
        int err = munmap(sb_memoria, sizeof(uint32_t)*2 + cantidadBloques/8);
        
        if (err == -1){
            log_error(logger, "[SuperBloque] Error al liberal la memoria mapeada de tamañoBloque y cantidadBloque");
        }

        close(superBloque);
        free(pathSuperBloque);
        bitarray_destroy(bitmap);
        free(memBitmap);


        log_info(logger, "-----------------------------------------------------");
        log_info(logger, "Se muestra los datos del superBloque.");
        log_info(logger, "Tamaño de bloque: %d", tamanioBloque);
        log_info(logger, "Cantidad de bloques: %d", cantidadBloques);
        log_info(logger, "-----------------------------------------------------");
    }
}


void validarBlocks(){
    log_info(logger, "Validando existencia de Blocks.ims....");
    
    char* pathBlocks = pathCompleto("Blocks.ims");

    if(access(pathBlocks,F_OK) < 0){
        log_error(logger, "No se encontró archivo Blocks.ims. Se crea archivo");
        
        int blocks = open(pathBlocks, O_CREAT | O_RDWR, 0664);
        int tamanioAGuardar = (tamanioBloque * cantidadBloques);
        posix_fallocate(blocks, 0, tamanioAGuardar);        
        
        copiaBlocks = malloc(tamanioBloque* cantidadBloques);
        
        void* blocks_memory = mmap(NULL, tamanioBloque*cantidadBloques, PROT_READ | PROT_WRITE, MAP_SHARED, blocks, 0);
        memcpy(copiaBlocks, blocks_memory, tamanioBloque*cantidadBloques);
        munmap(blocks_memory,tamanioBloque*cantidadBloques);
                
        close(blocks);
        free(pathBlocks);

        log_info(logger, "-----------------------------------------------------");
        log_info(logger, "Se creó archivo Blocks.ims");
        log_info(logger, "-----------------------------------------------------");

    }else{
        log_info(logger, "-----------------------------------------------------");
        log_info(logger,"Existe archivo Blocks.ims.");
        
        int blocks = open(pathBlocks, O_CREAT | O_RDWR, 0664);
        copiaBlocks = malloc(tamanioBloque*cantidadBloques);

        void* blocks_memory = mmap(NULL, tamanioBloque*cantidadBloques, PROT_READ | PROT_WRITE, MAP_SHARED, blocks, 0);
        memcpy(copiaBlocks, blocks_memory, tamanioBloque*cantidadBloques);
        munmap(blocks_memory,tamanioBloque*cantidadBloques);
        close(blocks);
        free(pathBlocks);

        log_info(logger, "-----------------------------------------------------");

    }
}


