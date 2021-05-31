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

// Globals
void *blocks_memory;
void *bitmap_memory;
int tamanioBloque;
int cantidadBloques;

#define _FSDIR_ "./fs"
char *global_route = _FSDIR_;
char *temp_dir;

char *getAbsolutePath(char *endpoint) {
    char *temp_dir;

    if (endpoint) {
        temp_dir = string_new();
        string_append(&temp_dir, global_route);
        string_append(&temp_dir, endpoint);
        return temp_dir;
    }

    return global_route;
}

void validarDirectorioFS(t_log* log){
    DIR* dir = opendir(getAbsolutePath(NULL));
    if(ENOENT == errno){
        log_info(log, "No existe directorio: Filesystem. Se crea.");
        mkdir(getAbsolutePath(NULL), 0664);
        temp_dir = getAbsolutePath("/bitacoras");
        mkdir(temp_dir, 0664);
        free(temp_dir);
        temp_dir = getAbsolutePath("/files");
        mkdir(temp_dir, 0664);
        free(temp_dir);
    }
    else{
        log_info(log, "Ya existe directorio");
    }
    closedir(dir);
}

void validarSuperBloque(t_log* log){
    log_info(log, "Validando existencia de superbloque.ims....");
    
    temp_dir = getAbsolutePath("/superBloque.ims");

    void *sb_memory;
    int superBloque;
    int err;

    if(access(temp_dir,F_OK) < 0){

        log_error(log, "No se encontró archivo superBloque.ims. Se crea archivo");

        log_info(log, "Ingresar tamaño de cada bloque");
        scanf("%d", &tamanioBloque);

        log_info(log, "Ingresar cantidad de bloques");
        scanf("%d", &cantidadBloques);

        while (cantidadBloques % 8 != 0) {
            log_error(log, "La cantidad de bloques debe ser divisible por 8");

            log_info(log, "Ingresar cantidad de bloques");
            scanf("%d", &cantidadBloques);
        }

        // File create
        superBloque = open(temp_dir, O_CREAT | O_RDWR);

        // Asigno memoria al archivo (2 enteros + tamanio de bitmap)
        posix_fallocate(superBloque, 0, sizeof(int) * 2 + cantidadBloques / 8);
        sb_memory = (char*) mmap(NULL, sizeof(int) * 2, PROT_READ | PROT_WRITE, MAP_SHARED, superBloque, 0);

        memcpy(sb_memory, &tamanioBloque, sizeof(int));
        memcpy(sb_memory + sizeof(int), &cantidadBloques, sizeof(int));

        close(superBloque);

        // Escribo en el archivo
        err = msync(sb_memory, 10, MS_ASYNC);
        if (err == -1) log_error(log, "msync");

        // Libero la memoria del archivo
        err = munmap(sb_memory, 10);
        if (err == -1) log_error(log, "munmap");
        log_info(log, "Se creó archivo superBloque.ims");

    }else{
        log_info(log, "Se encontró el archivo superBloque.ims");
        
        superBloque = open(temp_dir, O_CREAT | O_RDWR, 0664);
        sb_memory = mmap(NULL, sizeof(int) * 2, PROT_READ | PROT_WRITE, MAP_SHARED, superBloque, 0);

        memcpy(&tamanioBloque, sb_memory, sizeof(int));
        memcpy(&cantidadBloques, sb_memory + sizeof(int), sizeof(int));

        err = munmap(sb_memory, 10);
        if (err == -1) log_error(log, "munmap");
        close(superBloque);
        log_info(log, "Se muestra los datos del superBloque.");
        log_info(log, "Tamaño de bloque: %d", tamanioBloque);
        log_info(log, "Cantidad de bloques: %d", cantidadBloques);

    }
    free(temp_dir);
}

void generarBitmap(t_log* log){
    
    temp_dir = getAbsolutePath("/SuperBloque.ims");
    int archBitmap = open(temp_dir, O_CREAT | O_RDWR, 0664);
    
    bitmap_memory = mmap(NULL, sizeof(int) * 2 + (cantidadBloques / 8), PROT_READ | PROT_WRITE, MAP_SHARED, archBitmap, 0);

    t_bitarray* bitmap = bitarray_create_with_mode((char*)bitmap_memory + 8, cantidadBloques / 8, MSB_FIRST);  

    for(int i=0; i<cantidadBloques; i++){
        bitarray_clean_bit(bitmap,i);
        msync(bitmap->bitarray,cantidadBloques/8 ,0);
    }

    log_info(log, "Muestro mis valores del bitmap..");
    for(int i=0; i<cantidadBloques; i++){
        log_info(log,"%d",bitarray_test_bit(bitmap,i));
    }
    
    bitarray_destroy(bitmap);
    close(archBitmap);

    printf("[TEST bitmap] Tamaño bloque: %d\n",tamanioBloque);
    printf("[TEST bitmap] Cantidad de bloques:%d\n", cantidadBloques);

    free(temp_dir);
}

void validarBlocks(t_log* log){
    int err;

    log_info(log, "Validando existencia de Blocks.ims....");
    temp_dir = getAbsolutePath("/Blocks.ims");

    if(access(temp_dir,F_OK) < 0){
        log_error(log, "No se encontró archivo Blocks.ims. Se crea archivo");
        
        int archBlocks = open(temp_dir, O_CREAT | O_RDWR, 0664);

        // Asigno memoria al archivo
        posix_fallocate(archBlocks, 0, (tamanioBloque * cantidadBloques) + 1);
    
        blocks_memory = mmap(NULL, (tamanioBloque * cantidadBloques) + 1, PROT_READ | PROT_WRITE, MAP_SHARED, archBlocks, 0);
        
        char finArchivo = 'f';
        memcpy(blocks_memory + (tamanioBloque * cantidadBloques), &finArchivo, 1);

        close(archBlocks);

        // Escribo en el archivo
        err = msync(blocks_memory, 10, MS_ASYNC);
        if (err == -1) log_error(log, "msync");

        // Libero la memoria del archivo
        err = munmap(blocks_memory, 10);
        if (err == -1) log_error(log, "munmap");
        
        log_info(log, "Se creó archivo Blocks.ims");

        //TO DO: mapear a memoria el archivo entero.
    }else{
        log_info(log,"Existe archivo Blocks.ims.");
    }
    free(temp_dir);
}

void mapearBlocks(t_log* log){
    int err;

    printf("[TEST Map] Tamaño bloque: %d\n",tamanioBloque);
    printf("[TEST Map] Cantidad de bloques:%d\n", cantidadBloques);
    
    temp_dir = getAbsolutePath("/Blocks.ims");
    int archBlocks = open(temp_dir, O_CREAT | O_RDWR, 0664);

    blocks_memory = mmap(NULL, (tamanioBloque * cantidadBloques) + 1, PROT_READ | PROT_WRITE, MAP_SHARED, archBlocks, 0);
    
    char* test = "OOO";
    memcpy(blocks_memory, test, strlen(test));

    close(archBlocks);

    printf(".......Escribo OOO.........");
    err = msync(blocks_memory, 10, MS_ASYNC);
    if (err == -1) log_error(log, "msync");

    // Libero la memoria del archivo
    err = munmap(blocks_memory, 10);
    if (err == -1) log_error(log, "munmap");

    free(temp_dir);

}

int main() {

    t_log *log = log_create("../logs/test.log", "TEST", 1, LOG_LEVEL_TRACE);

    // -------------- TEST FILE SYSTEM -------------- //

    validarDirectorioFS(log);

    validarSuperBloque(log);
    generarBitmap(log);

    validarBlocks(log);
    mapearBlocks(log);

    log_destroy(log);

    return 0;
}