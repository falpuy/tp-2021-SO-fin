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

#define _FSDIR_ "./fs"

char *get_fs_dir(char *endpoint) {
    char *route = string_new();

    string_append(&route, _FSDIR_);

    if (endpoint) {
        string_append(&route, endpoint);
    }

    return route;
}


// void validarBlocks(t_log* log){
//     log_info(log, "Validando existencia de Blocks.ims....");

//     if(access("../Filesystem/Blocks.ims",F_OK) < 0){
//         log_error(log, "No se encontró archivo Blocks.ims. Se crea archivo");
        
//         FILE* arch_bloques = fopen("../Filesystem/Blocks.ims","w");
//         void* punteroTamanioBlocks = calloc(tamanioBloque*cantidadBloques, 1);
//         fwrite(punteroTamanioBlocks,tamanioBloque*cantidadBloques ,1 , arch_bloques);
//         fseek(arch_bloques,tamanioBloque*cantidadBloques,SEEK_SET);
        
//         char finArchivo = 'f';
//         fwrite(&finArchivo,sizeof(char),1, arch_bloques);
//         fclose(arch_bloques);
//         free(punteroTamanioBlocks);
//         log_info(log, "Se creó archivo Blocks.ims");

//         //TO DO: mapear a memoria el archivo entero.
//     }else{
//         log_info(log,"Existe archivo Blocks.ims.");
//     }


// }


// void guardarEspacioBitmap(t_log* log){
    
//     FILE* superBloque = fopen("../Filesystem/SuperBloque.ims","wb");
//     fseek(superBloque,sizeof(int)*2,SEEK_SET);
// 	void* punteroABitmap = calloc(cantidadBloques/8, 1);
//     fwrite(punteroABitmap, 1, cantidadBloques/8, superBloque);
//     free(punteroABitmap);
//     fclose(superBloque);

//     log_info(log, "Guarde espacio para el bitmap en superbloque");
// }

// void generarBitmap(t_log* log){

//     log_info(log, "Generando bitmap...");
//     guardarEspacioBitmap(log);
    
//     int archBitmap = open("../Filesystem/SuperBloque.ims", O_CREAT | O_RDWR, 0664);
//     lseek(archBitmap, sizeof(int)*2, SEEK_SET);
    
//     p_bitmap = mmap(NULL, cantidadBloques/8, PROT_READ | PROT_WRITE, MAP_SHARED, archBitmap, 0);
//     t_bitarray* bitmap = bitarray_create_with_mode((char*)p_bitmap, cantidadBloques/8, MSB_FIRST);  


//     for(int i=0; i<cantidadBloques; i++){
//         bitarray_clean_bit(bitmap,i);
//         msync(bitmap->bitarray,cantidadBloques/8 ,0);
//     }

//     log_info(log, "Muestro mis valores del bitmap..");
//     for(int i=0; i<cantidadBloques; i++){
//         log_info(log,"%d",bitarray_test_bit(bitmap,i));
//     }
    
//     bitarray_destroy(bitmap);
//     close(archBitmap);

//     printf("\n[TEST bitmap] Tamaño bloque: %d",tamanioBloque);
//     printf("\n[TEST bitmap] Cantidad de bloques:%d", cantidadBloques);
// }

// void mapearBlocks(t_log* log){

//     printf("\n[TEST Map] Tamaño bloque: %d",tamanioBloque);
//     printf("\n[TEST Map] Cantidad de bloques:%d", cantidadBloques);
    

//     int bloques = open("../Filesystem/Blocks.ims", O_CREAT | O_RDWR, 0664);    
//     mapArchivo = mmap(NULL,tamanioBloque*cantidadBloques, PROT_READ | PROT_WRITE, MAP_SHARED, bloques, 0);

//     char* test = "OOO";
//     memcpy(mapArchivo,test,strlen(test)+1);
//     printf(".......Escribo OOO.........");
// } 

void validarDirectorioFS(t_log* log){
    DIR* dir = opendir(get_fs_dir(NULL));
    if(ENOENT == errno){
        log_info(log, "No existe directorio: Filesystem. Se crea.");
        mkdir(get_fs_dir(NULL), 0664);
        mkdir(get_fs_dir("/bitacoras"), 0664);
        mkdir(get_fs_dir("/files"), 0664);
    }
    else{
        log_info(log, "Ya existe directorio");
    }
}

void validarSuperBloque(t_log* log){
    
    log_info(log, "Validando existencia de superbloque.ims....");
    if(access(get_fs_dir("/superBloque.ims"),F_OK) < 0){
        log_error(log, "No se encontró archivo superBloque.ims. Se crea archivo");
        
        log_info(log, "Ingresar tamaño de cada bloque");
        scanf("%d", &tamanioBloque);
        log_info(log, "Ingresar cantidad de bloques");
        scanf("%d", &cantidadBloques);

        FILE* superBloque = fopen(get_fs_dir("/SuperBloque.ims"),"wb");

        if(superBloque != NULL){    
            fwrite(&tamanioBloque, sizeof(int), 1, superBloque);
            fseek(superBloque,sizeof(int),SEEK_SET);
            fwrite(&cantidadBloques, sizeof(int), 1, superBloque);
            fclose(superBloque);
        }else
            log_error(log,"Error al abrir Superbloque.ims");
        log_info(log, "Se creó archivo superBloque.ims");
    }else{
        log_info(log, "Se encontró el archivo superBloque.ims");
        
        FILE* superBloque = fopen(get_fs_dir("/SuperBloque.ims"),"rb");
        if(superBloque != NULL){
            fread(&tamanioBloque, sizeof(int), 1, superBloque);
            fseek(superBloque,sizeof(int),SEEK_SET);
            fread(&cantidadBloques, sizeof(int), 1, superBloque);
            fclose(superBloque);
        }else
            log_error(log,"Error al abrir Superbloque.ims");

        log_info(log, "\nSe muestra los datos del superBloque.\nTamaño de bloque: %d\nCantidad de bloques: %d\n",tamanioBloque, cantidadBloques);
    }
}

int main() {

    t_log *log = log_create("../logs/test.log", "TEST", 1, LOG_LEVEL_TRACE);

    // -------------- TEST FILE SYSTEM -------------- //

    validarDirectorioFS(log);
    // validarSuperBloque(log);
    // validarBlocks(log);
    // generarBitmap(log);
    // mapearBlocks(log);

    // --------------- TEST DIR ROUTE -------------- //

    // log_info(log, "Testing..");
    // log_info(log, "FS Root: %s", get_fs_dir(NULL));
    // log_info(log, "FS /Blocks: %s", get_fs_dir("/blocks"));

    log_destroy(log);

    return 0;
}

// int main()
// {
//     // char str[] = "comando blabla [1,2,3,4,5] bla bla";
//     // remove_all_chars(str, '[');
//     // remove_all_chars(str, ']');
//     void *str = malloc(strlen("STRING DE PRUEBBA\0"));

//     memcpy(str, "STRING DE PRUEBBA\0", strlen("STRING DE PRUEBBA\0"));

//     printf("str '%s'\n", str);
    
//     int start = 3;
//     int end = 6;

//     // Delete space allocated by data in hq memory
//     memset(str + start, '\0', end - start);

//     printf("str '%s'\n", str + end);

//     for(int i = 0; i < strlen("STRING DE PRUEBBA\0"); i ++) {
//         if (memcmp(str + i, "\0", 1)) {
//             printf("CHAR: %s\n", (char*)&(str[i]));
//         } else {
//             printf("EMPTY: %s\n", (char*)&(str[i]));
//         }
//     }

//     // Search for free spaces
//     // for -> primero elemento vacio
//     // for -> validar qe haya elementos vacios desde el primero q matchea
//     // hhasta el tamanio necesario
//     // si se encuentra una valor distinto de vacio en el medio de la segunda busqueda
//     // colocar el primer for a partir de este valor y seguir buscando hhasta finalizar

//     // Trim all values in memory
//     // Use segment table to iterate the memory
//     // Copy all values inside a new buffer next to each other
//     // delete the memory buffer and set the new values

//     free(str);
//     return 0;
// }