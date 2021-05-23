#include "headers/preparacionFS.h"
#include <errno.h>

void inicializacionFS(t_log* log){
    validarDirectorioFS(log);
    validarSuperBloque(log);
    validarBlocks(log);
    generarBitmap(log);
    mapearBlocks(log);
}

void validarDirectorioFS(t_log* log){
    DIR* dir = opendir("../Filesystem");
    if(ENOENT == errno){
        log_info(log, "No existe directorio: Filesystem. Se crea.");
        mkdir("../Filesystem", 0664);
        mkdir("../Filesystem/Bitacoras",0664);
        mkdir("../Filesystem/Files",0664);
    }
    else{
        log_info(log, "Ya existe directorio");
    }
}

void validarSuperBloque(t_log* log){
    
    log_info(log, "Validando existencia de superbloque.ims....");
    if(access("../Filesystem/superBloque.ims",F_OK) < 0){
        log_error(log, "No se encontró archivo superBloque.ims. Se crea archivo");
        
        log_info(log, "Ingresar tamaño de cada bloque");
        scanf("%d", &tamanioBloque);
        log_info(log, "Ingresar cantidad de bloques");
        scanf("%d", &cantidadBloques);

        FILE* superBloque = fopen("../Filesystem/SuperBloque.ims","wb");

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
        
        FILE* superBloque = fopen("../Filesystem/SuperBloque.ims","rb");
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


void validarBlocks(t_log* log){
    log_info(log, "Validando existencia de Blocks.ims....");

    if(access("../Filesystem/Blocks.ims",F_OK) < 0){
        log_error(log, "No se encontró archivo Blocks.ims. Se crea archivo");
        
        FILE* arch_bloques = fopen("../Filesystem/Blocks.ims","w");
        void* punteroTamanioBlocks = calloc(tamanioBloque*cantidadBloques, 1);
        fwrite(punteroTamanioBlocks,tamanioBloque*cantidadBloques ,1 , arch_bloques);
        fseek(arch_bloques,tamanioBloque*cantidadBloques,SEEK_SET);
        
        char finArchivo = 'f';
        fwrite(&finArchivo,sizeof(char),1, arch_bloques);
        fclose(arch_bloques);
        free(punteroTamanioBlocks);
        log_info(log, "Se creó archivo Blocks.ims");

        //TO DO: mapear a memoria el archivo entero.
    }else{
        log_info(log,"Existe archivo Blocks.ims.");
    }
}


void guardarEspacioBitmap(t_log* log){
    
    FILE* superBloque = fopen("../Filesystem/SuperBloque.ims","wb");
    fseek(superBloque,sizeof(int)*2,SEEK_SET);
	void* punteroABitmap = calloc(cantidadBloques/8, 1);
    fwrite(punteroABitmap, 1, cantidadBloques/8, superBloque);
    free(punteroABitmap);
    fclose(superBloque);

    log_info(log, "Guarde espacio para el bitmap en superbloque");
}

void generarBitmap(t_log* log){

    log_info(log, "Generando bitmap...");
    guardarEspacioBitmap(log);
    
    int archBitmap = open("../Filesystem/SuperBloque.ims", O_CREAT | O_RDWR, 0664);
    lseek(archBitmap, sizeof(int)*2, SEEK_SET);
    
    p_bitmap = mmap(NULL, cantidadBloques/8, PROT_READ | PROT_WRITE, MAP_SHARED, archBitmap, 0);
    t_bitarray* bitmap = bitarray_create_with_mode((char*)p_bitmap, cantidadBloques/8, MSB_FIRST);  


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

    printf("\n[TEST bitmap] Tamaño bloque: %d",tamanioBloque);
    printf("\n[TEST bitmap] Cantidad de bloques:%d", cantidadBloques);
}

void mapearBlocks(t_log* log){

    printf("\n[TEST Map] Tamaño bloque: %d",tamanioBloque);
    printf("\n[TEST Map] Cantidad de bloques:%d", cantidadBloques);
    

    int bloques = open("../Filesystem/Blocks.ims", O_CREAT | O_RDWR, 0664);    
    mapArchivo = mmap(NULL,tamanioBloque*cantidadBloques, PROT_READ | PROT_WRITE, MAP_SHARED, bloques, 0);

    char* test = "OOO";
    memcpy(mapArchivo,test,strlen(test)+1);
    printf(".......Escribo OOO.........");
}   


// void generarFile(char* nombre,t_log* log){
//     // El tamaño del archivo expresado en bytes
//     // La cantidad de bloques del archivo
//     // La lista de bloques que lo conforman
//     // El caracter con el cual se llena el archivo
//     // MD5 del archivo, deberá actualizarse cada vez que se realicen modificaciones en el mismo

// }

// void actualizarFile(char* nombre, t_log* log){

// }