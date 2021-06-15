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
#include <pthread.h>


///////////////////////////////FUNCIONES///////////////////////////////////////
void inicializacionFS(t_log* log);
void validarDirectorioFS(t_log* log);
void validarSuperBloque(t_log* log);
void validarBlocks(t_log* log);
void funcionSincronizar();
void escribirBloque(t_log* log);
void funcionSincronizar(t_log* log);
////////////////////////////////////////////////////////////////////////////////

// Globals
void *blocks_memory;
void* sb_memoria;
//t_log* log;

void *memBitmap;
int flag;

void* copiaBlocks;
void* copiaSB;

t_bitarray* bitmap;
int tiempoSincronizacion = 5;
uint32_t tamanioBloque = 3;
uint32_t cantidadBloques = 64;

int superBloque;
int blocks;

pthread_mutex_t m_blocks;
pthread_mutex_t m_bitmap;
pthread_mutex_t m_superBloque;
pthread_mutex_t m_sincronizar;


int main() {

    t_log* log = log_create("testBaseFS.log","TEST", 1, LOG_LEVEL_INFO);
    pthread_t hiloSincronizacion;
    int contador = 0;
    
    pthread_mutex_init(&m_blocks, NULL); 
    pthread_mutex_init(&m_superBloque, NULL); 
    pthread_mutex_init(&m_bitmap, NULL);
    pthread_mutex_init(&m_sincronizar, NULL);
    
    inicializacionFS(log);
    escribirBloque(log);
    flag=1;
    pthread_create(&hiloSincronizacion,NULL,(void*) funcionSincronizar,log);
    pthread_detach(hiloSincronizacion);

    while(1){
       sleep(1);
       printf("holis");
    }

    pthread_mutex_destroy(&m_blocks); 
    pthread_mutex_destroy(&m_superBloque); 
    pthread_mutex_destroy(&m_bitmap); 
    pthread_mutex_destroy(&m_sincronizar); 



    return 0;
}
void escribirBloque(t_log* log){
    char* oxigeno = string_repeat('O',3);
    log_info(log, "Muestro mis valores del bitmap ANTES DE COPIAR..");
    for(int i=0; i<cantidadBloques; i++){
        printf("%d",bitarray_test_bit(bitmap,i));

    }  
    pthread_mutex_lock(&m_blocks);
    memcpy(copiaBlocks,oxigeno,tamanioBloque);
    pthread_mutex_unlock(&m_blocks);

    pthread_mutex_lock(&m_bitmap);
    bitarray_set_bit(bitmap,0);
    memcpy(copiaSB+sizeof(int)*2,bitmap->bitarray,cantidadBloques/8);
    pthread_mutex_unlock(&m_bitmap);

    log_info(log, "Muestro mis valores del bitmap DESPUES DE COPIAR..");
    for(int i=0; i<cantidadBloques; i++){
        printf("%d",bitarray_test_bit(bitmap,i));

    }   
}


void funcionSincronizar(t_log* log){
    while(flag){
        sleep(tiempoSincronizacion);
        log_info(log, "Comienza actualización de Blocks.ims...");
        log_info(log, "Muestro mis valores del bitmap ANTES DE sincronizar..");
        for(int i=0; i<cantidadBloques; i++){
            printf("%d",bitarray_test_bit(bitmap,i));

        }  
        pthread_mutex_lock(&m_blocks);
        int archBloques = open("Filesystem/Blocks.ims", O_CREAT | O_RDWR, 0664);
        blocks_memory = mmap(NULL, tamanioBloque*cantidadBloques, PROT_READ | PROT_WRITE, MAP_SHARED, archBloques, 0);
        memcpy(blocks_memory,copiaBlocks, tamanioBloque*cantidadBloques);
        msync(blocks_memory,tamanioBloque*cantidadBloques,0);
        munmap(blocks_memory,tamanioBloque*cantidadBloques);
        log_info(log, "Se actualizo Blocks.ims");
        
        int archSB = open("Filesystem/SuperBloque.ims", O_CREAT | O_RDWR, 0664);
        sb_memoria = mmap(NULL, sizeof(int)*2 + cantidadBloques/8, PROT_READ | PROT_WRITE, MAP_SHARED, archSB, 0);
        memcpy(sb_memoria,copiaSB,sizeof(int)*2 + cantidadBloques/8);
        msync(sb_memoria,sizeof(int)*2 + cantidadBloques/8,0);
        munmap(sb_memoria,sizeof(int)*2 + cantidadBloques/8);
        
        log_info(log, "Se actualizo Bitmap en SuperBloque.ims");
        close(archBloques);
        close(archSB);
        pthread_mutex_unlock(&m_blocks);
        log_info(log, "Muestro mis valores del bitmap ANTES DE sincronizar..");
        for(int i=0; i<cantidadBloques; i++){
            printf("%d",bitarray_test_bit(bitmap,i));

        }  
    }
}

void inicializacionFS(t_log* log){
    validarDirectorioFS(log);
    validarSuperBloque(log);
    validarBlocks(log);
}

void validarDirectorioFS(t_log* log){
    log_info(log, "Validando Existencia de directorios...");
    DIR* dir = opendir("Filesystem");
    if(ENOENT == errno){
        log_info(log, "No existe directorio: Filesystem. Se crea.");
        mkdir("Filesystem", 0777);
        mkdir("Filesystem/Bitacoras",0777);
        mkdir("Filesystem/Files",0777);
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


    if(access("Filesystem/SuperBloque.ims",F_OK) < 0){
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

        superBloque = open("Filesystem/SuperBloque.ims", O_CREAT | O_RDWR,0664);
        
        if(superBloque<0){
            log_error(log, "Error al abrir/crear Superbloque");
        }
        
        posix_fallocate(superBloque, 0, sizeof(uint32_t) * 2 + cantidadBloques / 8);
        
        copiaSB = malloc(tamanioBloque*cantidadBloques + cantidadBloques/8);
        
        sb_memoria = (char*) mmap(NULL, sizeof(uint32_t) * 2, PROT_READ | PROT_WRITE, MAP_SHARED, superBloque, 0);
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

        //bitarray_destroy(bitmap);
        //free(memBitmap);
        //free(copiaSB);
        close(superBloque);

        log_info(log, "-----------------------------------------------------");
        log_info(log, "Se creó archivo superBloque.ims");
        log_info(log, "-----------------------------------------------------");


    }else{
        log_info(log, "Se encontró el archivo superBloque.ims");

        superBloque = open("Filesystem/SuperBloque.ims", O_CREAT | O_RDWR, 0664);
        sb_memoria = mmap(NULL, sizeof(uint32_t) * 2 , PROT_READ | PROT_WRITE, MAP_SHARED, superBloque, 0);
        
        memcpy(&tamanioBloque, sb_memoria, sizeof(uint32_t));
        memcpy(&cantidadBloques, sb_memoria + sizeof(uint32_t), sizeof(uint32_t));
        int err = munmap(sb_memoria, sizeof(uint32_t)*2);

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
        //free(memBitmap);
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
    

    if(access("Filesystem/Blocks.ims",F_OK) < 0){
        log_error(log, "No se encontró archivo Blocks.ims. Se crea archivo");
        
        blocks = open("Filesystem/Blocks.ims", O_CREAT | O_RDWR, 0664);
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

        int archBloques = open("Filesystem/Blocks.ims", O_CREAT | O_RDWR, 0664);
        copiaBlocks = malloc(tamanioBloque*cantidadBloques);

        blocks_memory = mmap(NULL, tamanioBloque*cantidadBloques, PROT_READ | PROT_WRITE, MAP_SHARED, archBloques, 0);
        memcpy(copiaBlocks, blocks_memory, tamanioBloque*cantidadBloques);
        munmap(blocks_memory,tamanioBloque*cantidadBloques);
        close(archBloques);

        log_info(log, "-----------------------------------------------------");
    }
}

    