#include "./headers/preparacionFS.h"

void inicializacionFS(t_log* log){
    validarSuperBloque(log);
    validarBlocks(log);
    generarBitmap(log);
}


void validarSuperBloque(t_log* log){
    
    log_info(log, "Validando existencia de superbloque.ims....");

    if(access("../Filesystem/superBloque.ims",F_OK) < 0){
        log_error(log, "No se encontró archivo superBloque.ims. Se crea archivo");
        
        log_info(log, "Ingresar tamaño de cada bloque");
        scanf("%d", &tamanioBloque);
        log_info(log, "Ingresar cantidad de bloques");
        scanf("%d", &cantidadBloques);

        superBloque = fopen("../Filesystem/SuperBloque.ims","wb");
        fwrite(&tamanioBloque, sizeof(int), 1, superBloque);
        fseek(superBloque,sizeof(int),SEEK_SET);
        fwrite(&cantidadBloques, sizeof(int), 1, superBloque);
        fclose(superBloque);

        log_info(log, "Se creó archivo superBloque.ims");
    }else{
        log_info(log, "Se encontró el archivo superBloque.ims");

        superBloque = fopen("../Filesystem/SuperBloque.ims","rb");
        fread(&tamanioBloque, sizeof(int), 1, superBloque);
        fseek(superBloque,sizeof(int),SEEK_SET);
        fread(&cantidadBloques, sizeof(int), 1, superBloque);
        fclose(superBloque);

        log_info(log, "Se muestra los datos del superBloque.\nTamaño de bloque:%d\nCantidad de bloques:%d\n",tamanioBloque, cantidadBloques);
    }
}


void validarBlocks(t_log* log){
    log_info(log, "Validando existencia de Blocks.ims....");

    if(access("../Filesystem/Blocks.ims",F_OK) < 0){
        log_error(log, "No se encontró archivo Blocks.ims. Se crea archivo");
        
        //Creo archivo blocks.ims--> con tamaño fijo por config
        arch_bloques = fopen("../Filesystem/Blocks.ims","w");
        void* punteroTamanioBlocks = calloc(tamanioBloque*cantidadBloques, 1);
        fwrite(punteroTamanioBlocks,tamanioBloque*cantidadBloques ,1 , arch_bloques);
        fseek(arch_bloques,tamanioBloque*cantidadBloques,SEEK_SET);
        
        char finArchivo = 'f';
        fwrite(&finArchivo,sizeof(char),1, arch_bloques);
        fclose(arch_bloques);
        log_info(log, "Se creó archivo Blocks.ims");

    }else{
        log_info(log,"Existe archivo Blocks.ims.");
    }
}


void guardarEspacioBitmap(t_log* log){
    superBloque = fopen("../Filesystem/SuperBloque.ims","wb");
    fseek(superBloque,sizeof(int)*2,SEEK_SET);
	void* punteroABitmap = calloc(cantidadBloques/8, 1);
    fwrite(punteroABitmap, 1, cantidadBloques/8, superBloque);
    
    free(punteroABitmap);
    fclose(superBloque);
    log_info(log, "Guarde espacio para el bitmap en superbloque");
}

void generarBitmap(t_log* log){

    log_info(log, "Generando bitmap...");
    int cantidad = 0;

    //checkear: a veces no me lee las var globales seteadas antes--> fix: las busco de nuevo y las seteo
    superBloque = fopen("../Filesystem/SuperBloque.ims","rb");
    fread(&tamanioBloque, sizeof(int), 1, superBloque);
    fseek(superBloque,sizeof(int),SEEK_SET);
    fread(&cantidadBloques, sizeof(int), 1, superBloque);
    fclose(superBloque);

    printf("Vuelvo a buscar los valores de los bloques, asi puedo guardar mem para bitmap");
    printf("\n%d", tamanioBloque);
    printf("\n%d", cantidadBloques);
    
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
}