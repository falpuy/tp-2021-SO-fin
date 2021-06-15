#include "headers/syncFile.h"

void actualizarArchivoBlocks(){
     
     while(flagEnd){
        sleep(datosConfig->tiempoSincronizacion);
        log_info(logger, "Comienza actualización de Blocks.ims...");
        
        pthread_mutex_lock(&blocks_bitmap);
        log_info(logger, "Muestro mis valores del bitmap antes de sincronizar..");
        for(int i=0; i<cantidadBloques; i++){
            printf("%d",bitarray_test_bit(bitmap,i));
        }  

        int archBloques = open("../Filesystem/Blocks.ims", O_CREAT | O_RDWR, 0664);
        void* blocks_memory = mmap(NULL, tamanioBloque*cantidadBloques, PROT_READ | PROT_WRITE, MAP_SHARED, archBloques, 0);
        memcpy(blocks_memory,copiaBlocks, tamanioBloque*cantidadBloques);
        msync(blocks_memory,tamanioBloque*cantidadBloques,0);
        munmap(blocks_memory,tamanioBloque*cantidadBloques);
        log_info(logger, "Se actualizo Blocks.ims");
        
        int archSB = open("../Filesystem/SuperBloque.ims", O_CREAT | O_RDWR, 0664);
        void* sb_memoria = mmap(NULL, sizeof(int)*2 + cantidadBloques/8, PROT_READ | PROT_WRITE, MAP_SHARED, archSB, 0);
        memcpy(sb_memoria,copiaSB,sizeof(int)*2 + cantidadBloques/8);
        msync(sb_memoria,sizeof(int)*2 + cantidadBloques/8,0);
        munmap(sb_memoria,sizeof(int)*2 + cantidadBloques/8);
        
        log_info(logger, "Se actualizo Bitmap en SuperBloque.ims");

        close(archBloques);
        close(archSB);
        
        
        log_info(logger, "Muestro mis valores del bitmap despues sincronizar..");
        for(int i=0; i<cantidadBloques; i++){
            printf("%d",bitarray_test_bit(bitmap,i));
        }  
        pthread_mutex_unlock(&blocks_bitmap);
    }
}
