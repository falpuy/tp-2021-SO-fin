#include "headers/preparacionFS.h"
#include <commons/bitarray.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>



char* pathArchivo(char* puntoDeMontaje, char* nombreArchivo){
    char* path =string_new();
    string_append_with_format(&puntoDeMontaje, "%s", "/");
    string_append_with_format(&puntoDeMontaje, "%s", nombreArchivo);

    return path;
}

Bloques: 
    Metadata
    Bloques en si 

Tareas
    Metadata
    Files en si (llenado)


void crearEstructurasFS(t_log* logger){

    t_config* superbloque = config_create("/home/utnso/Escritorio/TP_Operativos/tp-2021-1c-Unnamed-Group/IMongoStore/Filesystem/SuperBloque.ims");

    cantidadBloques = config_get_int_value(superbloque, "BLOCKS");
    tamanioBloque = config_get_int_value(superbloque,"BLOCK_SIZE");
    
    int arch = open("/home/utnso/Escritorio/TP_Operativos/tp-2021-1c-Unnamed-Group/IMongoStore/Filesystem/SuperBloque.ims", O_RDWR, 666);
    //asignarle "espacio " para futuro bitmap (al final de archivo)

    char* test = mmap(NULL,100, PROT_READ | PROT_WRITE, MAP_SHARED, arch,0);
    t_bitarray * bitmap = bitarray_create_with_mode(test,1,LSB_FIRST);
    
   
   
   "BLOCK_SIZE=32 \n
    BLOCKS=9\n"

    cant bloques 
    tamanio 
    bitmap







    // // The mmap() function is used for mapping between a process address space and either files or devices. When a file is mappeOd to a process address space, the file can be accessed like an array in the program.





    // char* bits = config_get_string_value(superbloque,"BITMAP");

    // 

    // printf("%d", bitarray_test_bit(bitmap,3));



    // memcpy() is used to copy a block of memory from a location to another.
    // The msync() function writes out data in a mapped region to the permanent storage for the underlying object. The call to msync() ensures data integrity of the file. After the data is written out, any cached data may be invalidated if the MS_INVALIDATE flag was specified.




    // if(access("/home/utnso/Escritorio/TP_Operativos/tp-2021-1c-Unnamed-Group/IMongoStore/Filesystem/Blocks.ims",F_OK) < 0){
    //     log_info(logger, "No se encontró archivo Blocks.ims entonces se crea uno");

    //         FILE* arch_block = fopen("Blocks.ims","rw");

            // Este archivo será el encargado de almacenar todos los bloques de nuestro File System. Al inicializar el File System, si el mismo no está formateado, deberá crearse el archivo con el tamaño de la cantidad de bloques por el tamaño del bloque.
            // El archivo de bloques deberá encontrarse en  [Punto_Montaje]/Blocks.ims 

            // Internamente, este archivo deberá ser mapeado a memoria y se le realizará una copia sobre la cual se trabajará para lograr simular la concurrencia y asincronismo en las bajadas a disco de los sistemas operativos reales. El tiempo de sincronización hacia el archivo físico de Blocks.ims será especificado por archivo de configuración. Cualquier otra implementación es motivo de desaprobación directa.
             

    







}