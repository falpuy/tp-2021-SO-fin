#include "headers/preparacionFS.h"
#include <errno.h>

void inicializacionFS(){
    validarDirectorioFS();
    validarSuperBloque();
    validarBlocks();
}

void validarDirectorioFS(){
    log_info(log, "Validando Existencia de directorios...");
    DIR* dir = opendir("../Filesystem");
    if(ENOENT == errno){
        log_info(log, "No existe directorio: Filesystem. Se crea.");
        mkdir("../Filesystem", 0777);
        mkdir("../Filesystem/Bitacoras",0777);
        mkdir("../Filesystem/Files",0777);
        closedir(dir);
    }
    else{
        log_info(log, "Ya existe directorio");
        closedir(dir);
    }
}

void validarSuperBloque(){
    log_info(log, "-----------------------------------------------------");
    log_info(log, "Validando existencia de Superbloque.ims....");
    log_info(log, "-----------------------------------------------------");


    if(access("../Filesystem/SuperBloque.ims",F_OK) < 0){
        log_error(log, "No se encontró archivo SuperBloque.ims. Se crea archivo");
           
        log_info(log, "Ingresar el tamaño de cada bloque");
        scanf("%d", &tamanioBloque);
        log_info(log, "Ingresar la cantidad de bloques");
        scanf("%d", &cantidadBloques);

        while (cantidadBloques % 8 != 0) {
            log_error(log, "La cantidad de bloques debe ser divisible por 8");
            log_info(log, "Ingresar cantidad de bloques");
            scanf("%d", &cantidadBloques);
        }

        int superBloque = open("../Filesystem/SuperBloque.ims", O_CREAT | O_RDWR,0664);
        
        if(superBloque<0){
            log_error(log, "Error al abrir/crear Superbloque");
        }
        
        posix_fallocate(superBloque, 0, sizeof(uint32_t) * 2 + cantidadBloques / 8);
        
        copiaSB = malloc(tamanioBloque*cantidadBloques + cantidadBloques/8);
        
        void* sb_memoria = (char*) mmap(NULL, sizeof(uint32_t) * 2, PROT_READ | PROT_WRITE, MAP_SHARED, superBloque, 0);
        memBitmap = malloc(cantidadBloques/8);
        bitmap = bitarray_create_with_mode((char*)memBitmap, cantidadBloques / 8, MSB_FIRST);  
        
        for(int i=0; i<cantidadBloques; i++){
            bitarray_clean_bit(bitmap,i);    
        }
        
        log_info(log, "Muestro mis valores del bitmap..");
        for(int i=0; i<cantidadBloques; i++){
            printf("%d",bitarray_test_bit(bitmap,i));

        }   

        memcpy(copiaSB, &tamanioBloque, sizeof(uint32_t));
        memcpy(copiaSB + sizeof(uint32_t), &cantidadBloques, sizeof(uint32_t));
        memcpy(copiaSB + sizeof(uint32_t)*2,bitmap->bitarray, cantidadBloques/8);

        memcpy(sb_memoria,copiaSB,2*sizeof(uint32_t)+cantidadBloques/8);
        
        int err = msync(sb_memoria, 2*sizeof(uint32_t) + cantidadBloques/8, MS_SYNC);
        if(err == -1){
            log_error(log, "[SuperBloque] Error de sincronizar a disco SuperBloque");
        }

        close(superBloque);
        err = munmap(sb_memoria, sizeof(uint32_t)*2);
        if (err == -1){
            log_error(log, "[SuperBloque] Error al liberal la memoria mapeada de tamañoBloque y cantidadBloque");
        }

        close(superBloque);

        log_info(log, "-----------------------------------------------------");
        log_info(log, "Se creó archivo superBloque.ims");
        log_info(log, "-----------------------------------------------------");

    }else{

        log_info(log, "Se encontró el archivo superBloque.ims");

        int superBloque = open("../Filesystem/SuperBloque.ims", O_CREAT | O_RDWR, 0664);
        
        //Mapeo para sacar el tamaño y cantidad solo
        void* sb_memoria = mmap(NULL, sizeof(uint32_t) * 2 , PROT_READ | PROT_WRITE, MAP_SHARED, superBloque, 0);
        memcpy(&tamanioBloque, sb_memoria, sizeof(uint32_t));
        memcpy(&cantidadBloques, sb_memoria + sizeof(uint32_t), sizeof(uint32_t));
        int err = munmap(sb_memoria, sizeof(uint32_t)*2);

        //Mapeo ahora para sacar el bitmap tmb
        sb_memoria = mmap(NULL, sizeof(uint32_t) * 2 + cantidadBloques/8 , PROT_READ | PROT_WRITE, MAP_SHARED, superBloque, 0);
        memBitmap = malloc(cantidadBloques/8);
        memcpy(memBitmap, sb_memoria + sizeof(uint32_t)*2, cantidadBloques/8);
        bitmap = bitarray_create_with_mode((char*)memBitmap, cantidadBloques / 8, MSB_FIRST);  
        
        log_info(log, "Muestro mis valores del bitmap..");
        for(int i=0; i<cantidadBloques; i++){
            printf("%d",bitarray_test_bit(bitmap,i));

        }  

        copiaSB = malloc(sizeof(uint32_t) * 2 + cantidadBloques/8);
        memcpy(copiaSB, sb_memoria,sizeof(uint32_t) * 2 + cantidadBloques/8);
        err = munmap(sb_memoria, sizeof(uint32_t)*2 + cantidadBloques/8);
        
        if (err == -1){
            log_error(log, "[SuperBloque] Error al liberal la memoria mapeada de tamañoBloque y cantidadBloque");
        }

        close(superBloque);

        log_info(log, "-----------------------------------------------------");
        log_info(log, "Se muestra los datos del superBloque.");
        log_info(log, "Tamaño de bloque: %d", tamanioBloque);
        log_info(log, "Cantidad de bloques: %d", cantidadBloques);
        log_info(log, "-----------------------------------------------------");
    }
}


void validarBlocks(){
    log_info(log, "Validando existencia de Blocks.ims....");
    

    if(access("../Filesystem/Blocks.ims",F_OK) < 0){
        log_error(log, "No se encontró archivo Blocks.ims. Se crea archivo");
        
        int blocks = open("../Filesystem/Blocks.ims", O_CREAT | O_RDWR, 0664);
        
        int tamanioAGuardar = (tamanioBloque * cantidadBloques);
        copiaBlocks = malloc(tamanioBloque* cantidadBloques);
        memset(copiaBlocks,' ',tamanioBloque* cantidadBloques);
        
        posix_fallocate(blocks, 0, tamanioAGuardar);        
        close(blocks);
    

        log_info(log, "-----------------------------------------------------");
        log_info(log, "Se creó archivo Blocks.ims");
        log_info(log, "-----------------------------------------------------");

    }else{
        log_info(log, "-----------------------------------------------------");
        log_info(log,"Existe archivo Blocks.ims.");
        
        int blocks = open("../Filesystem/Blocks.ims", O_CREAT | O_RDWR, 0664);
        copiaBlocks = malloc(tamanioBloque*cantidadBloques);

        void* blocks_memory = mmap(NULL, tamanioBloque*cantidadBloques, PROT_READ | PROT_WRITE, MAP_SHARED, blocks, 0);
        memcpy(copiaBlocks, blocks_memory, tamanioBloque*cantidadBloques);
        munmap(blocks_memory,tamanioBloque*cantidadBloques);
        close(blocks);

        log_info(log, "-----------------------------------------------------");

    }
}


