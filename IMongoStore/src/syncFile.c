#include "headers/syncFile.h"

void actualizarArchivoBlocks(){
     
     while(flagEnd){
        log_info(log,"Comienza a esperar para actualizar el archivo blocks.ims");
        printf("%d", datosConfig->tiempoSincronizacion);
        sleep(datosConfig->tiempoSincronizacion);
        log_info(log, "Comienza actualización de Blocks.ims...");
        
        log_info(log, "Muestro mis valores del bitmap antes de sincronizar..");
        for(int i=0; i<cantidadBloques; i++){
            printf("%d",bitarray_test_bit(bitmap,i));

        }  

        pthread_mutex_lock(&m_blocks);
        int archBloques = open("../Filesystem/Blocks.ims", O_CREAT | O_RDWR, 0664);
        void* blocks_memory = mmap(NULL, tamanioBloque*cantidadBloques, PROT_READ | PROT_WRITE, MAP_SHARED, archBloques, 0);
        memcpy(blocks_memory,copiaBlocks, tamanioBloque*cantidadBloques);
        msync(blocks_memory,tamanioBloque*cantidadBloques,0);
        munmap(blocks_memory,tamanioBloque*cantidadBloques);
        log_info(log, "Se actualizo Blocks.ims");
        
        int archSB = open("../Filesystem/SuperBloque.ims", O_CREAT | O_RDWR, 0664);
        void* sb_memoria = mmap(NULL, sizeof(int)*2 + cantidadBloques/8, PROT_READ | PROT_WRITE, MAP_SHARED, archSB, 0);
        memcpy(sb_memoria,copiaSB,sizeof(int)*2 + cantidadBloques/8);
        msync(sb_memoria,sizeof(int)*2 + cantidadBloques/8,0);
        munmap(sb_memoria,sizeof(int)*2 + cantidadBloques/8);
        
        log_info(log, "Se actualizo Bitmap en SuperBloque.ims");
        close(archBloques);
        close(archSB);
        pthread_mutex_unlock(&m_blocks);
        
        
        log_info(log, "Muestro mis valores del bitmap despues sincronizar..");
        for(int i=0; i<cantidadBloques; i++){
            printf("%d",bitarray_test_bit(bitmap,i));

        }  
    }
}
