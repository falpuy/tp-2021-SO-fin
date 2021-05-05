#include "./headers/preparacionFS.h"

void crearSuperBloqueTest(){
    int cantidadBloques = 16;
    int tamanioBloque = 64;
    superBloque = fopen("../Filesystem/SuperBloque.ims","wb");
    
    fwrite(&tamanioBloque, sizeof(int), 1, superBloque);
    fwrite(&cantidadBloques, sizeof(int), 1, superBloque);

    fclose(superBloque);
}

void inicializacionFS(t_log* log){
    validarExistenciaFS(log);
}


void validarExistenciaFS(t_log* log){
    if(access("../Filesystem/Blocks.ims",F_OK) < 0){
        log_error(log, "No se encontró archivo Blocks.ims entonces no existe un Filesystem anterior. Se crea Uno");
        setearMetadataFS(log);
        //crear Blocks.ims
        generarBitmap(log);
        testearQueTodoFunco(log);
    }else{
        log_info(log,"Existe archivo Blocks.ims. Se levanta Filesystem anterior");
    }

}

void setearMetadataFS(t_log* log){
    superBloque = fopen("../Filesystem/SuperBloque.ims","rb");
    
    fread(&tamanioBloque,sizeof(int),1,superBloque);
    fseek(superBloque,sizeof(int),SEEK_SET);
    fread(&cantidadBloques,sizeof(int),1,superBloque);
    
    log_info(log, "Se buscaron valores del superbloque. TamañoBloque:%d y CantidadBloques:%d",tamanioBloque,cantidadBloques);
    fclose(superBloque);
    guardarEspacioBitmap(log);
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
    int cantidad=0;
    int archBitmap = open("../Filesystem/SuperBloque.ims", O_CREAT | O_RDWR, 0664);
    lseek(archBitmap, sizeof(int)*2, SEEK_SET);
    p_bitmap = mmap(NULL, cantidadBloques/8, PROT_READ | PROT_WRITE, MAP_SHARED, archBitmap, 0);
    t_bitarray* bitmap = bitarray_create_with_mode((char*)p_bitmap, cantidadBloques/8, MSB_FIRST);
    
    for(int i=0; i<cantidadBloques; i++){
        bitarray_clean_bit(bitmap,i);
        msync(bitmap->bitarray,cantidadBloques/8 ,0);
        cantidad+=1;
    }

    log_info(log,"Setee bitmap en 0 todo......%d",cantidad);
    bitarray_destroy(bitmap);
}

void testearQueTodoFunco(t_log* log){
    t_bitarray* bitmap = bitarray_create_with_mode((char*)p_bitmap, cantidadBloques/8, MSB_FIRST);
    
    for(int i=0; i<cantidadBloques; i++){
        
        log_info(log,"%d",bitarray_test_bit(bitmap,i));
    }

    log_info(log,"Testo bits Success");
    bitarray_destroy(bitmap);
}