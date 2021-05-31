#include "headers/preparacionFS.h"
#include <errno.h>

void inicializacionFS(t_log* log){
    validarDirectorioFS(log);
    validarSuperBloque(log);
    validarBlocks(log);
    generarBitmap(log);
}

void validarDirectorioFS(t_log* log){
    log_info(log, "Validando Existencia de directorios...");
    sleep(2);
    DIR* dir = opendir("../Filesystem");
    if(ENOENT == errno){
        log_info(log, "No existe directorio: Filesystem. Se crea.");
        sleep(2);
        mkdir("../Filesystem", 0664);
        mkdir("../Filesystem/Bitacoras",0664);
        mkdir("../Filesystem/Files",0664);
        closedir(dir);
    }
    else{
        log_info(log, "Ya existe directorio");
        closedir(dir);
    }
}

void validarSuperBloque(t_log* log){
    log_info(log, "-----------------------------------------------------");
    log_info(log, "Validando existencia de Superbloque.ims....");
    log_info(log, "-----------------------------------------------------");

    sleep(2);

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

        superBloque = open("../Filesystem/SuperBloque.ims", O_CREAT | O_RDWR,0664);
        
        if(superBloque<0){
            log_error(log, "Error al abrir/crear Superbloque");
        }
        // Asigno memoria al archivo (2 enteros + tamanio de bitmap)
        //posix_fallocate --> fallocate no funciona..

        posix_fallocate(superBloque, 0, sizeof(int) * 2 + cantidadBloques / 8);
        sb_memoria = (char*) mmap(NULL, sizeof(int) * 2, PROT_READ | PROT_WRITE, MAP_SHARED, superBloque, 0);
        
        memcpy(sb_memoria, &tamanioBloque, sizeof(int));
        memcpy(sb_memoria + sizeof(int), &cantidadBloques, sizeof(int));

        close(superBloque);
        
        // Escribo en el archivo
        err = msync(sb_memoria, sizeof(int)*2, MS_ASYNC);
        if (err == -1){
            log_error(log, "[SuperBloque] Error de sincronizar a disco tamañoBloque y cantidadBloques");
        }

        // Libero la memoria del archivo
        err = munmap(sb_memoria, sizeof(int)*2);
        if (err == -1){
            log_error(log, "[SuperBloque] Error al liberal la memoria mapeada de tamañoBloque y cantidadBloque");
        }
        log_info(log, "-----------------------------------------------------");
        log_info(log, "Se creó archivo superBloque.ims");
        log_info(log, "-----------------------------------------------------");


    }else{
        log_info(log, "Se encontró el archivo superBloque.ims");
        
        superBloque = open("../Filesystem/SuperBloque.ims", O_CREAT | O_RDWR, 0664);
        sb_memoria = mmap(NULL, sizeof(int) * 2, PROT_READ | PROT_WRITE, MAP_SHARED, superBloque, 0);

        memcpy(&tamanioBloque, sb_memoria, sizeof(int));
        memcpy(&cantidadBloques, sb_memoria + sizeof(int), sizeof(int));

        err = munmap(sb_memoria, sizeof(int)*2);
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


void validarBlocks(t_log* log){
    log_info(log, "Validando existencia de Blocks.ims....");
    sleep(2);

    if(access("../Filesystem/Blocks.ims",F_OK) < 0){
        log_error(log, "No se encontró archivo Blocks.ims. Se crea archivo");
        sleep(2);
        
        blocks = open("../Filesystem/Blocks.ims", O_CREAT | O_RDWR, 0664);
        int tamanioAGuardar = (tamanioBloque * cantidadBloques)*1;

        posix_fallocate(blocks, 0, tamanioAGuardar);
        blocks_memory = mmap(NULL,tamanioAGuardar, PROT_READ | PROT_WRITE, MAP_SHARED, blocks, 0);
        
        close(blocks);
        
        // Escribo en el archivo
        err = msync(blocks_memory, tamanioAGuardar, MS_ASYNC);
        if (err == -1){
            log_error(log, "[Blocks] Error de sincronizar a disco tamañoBloque y cantidadBloques");
        }
        
        log_info(log, "-----------------------------------------------------");
        log_info(log, "Se creó archivo Blocks.ims");
        log_info(log, "-----------------------------------------------------");

    }else{
        log_info(log, "-----------------------------------------------------");
        log_info(log,"Existe archivo Blocks.ims.");
        log_info(log, "-----------------------------------------------------");

    }
}


void generarBitmap(t_log* log){

    log_info(log, "Generando bitmap...");
    sleep(2);

    arch_bitmap = open("../Filesystem/SuperBloque.ims", O_CREAT | O_RDWR, 0664);
    bitmap_memory = mmap(NULL, sizeof(int) * 2 + (cantidadBloques / 8), PROT_READ | PROT_WRITE, MAP_SHARED, arch_bitmap, 0);
    bitmap = bitarray_create_with_mode((char*)bitmap_memory + 8, cantidadBloques / 8, MSB_FIRST);  


    for(int i=0; i<cantidadBloques; i++){
        bitarray_clean_bit(bitmap,i);
        msync(bitmap->bitarray,cantidadBloques/8 ,0);
    }

    // log_info(log, "Muestro mis valores del bitmap..");
    // for(int i=0; i<cantidadBloques; i++){
    //     log_info(log,"%d",bitarray_test_bit(bitmap,i));
    // }

    close(arch_bitmap);

    printf("\n[TEST bitmap] Tamaño bloque: %d",tamanioBloque);
    printf("\n[TEST bitmap] Cantidad de bloques:%d \n", cantidadBloques);

}

