#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <commons/config.h>
#include <math.h>
#include <commons/log.h>
#include <commons/bitarray.h>
#include <commons/string.h>
#include <unistd.h> 

char* path = "~/Escritorio/TP/tp-2021-1c-Unnamed-Group/IMongoStore/test";

// char* string_from_format(const char* format, ...) {
// 	char* nuevo;
// 	va_list arguments;
// 	va_start(arguments, format);
// 	nuevo = string_from_vformat(format, arguments);
// 	va_end(arguments);
// 	return nuevo;
// }



// char* pathCompleto(const char* str, ...){
// 	char* temporal = string_from_format("%s/%s", path, str);

// 	va_list parametros;
// 	va_start(parametros, str);
// 	char* pathAbsoluto = string_from_vformat(temporal, parametros);
// 	va_end(parametros);
	
//     free(temporal);
// 	return pathAbsoluto;
// }


// char* pathCompleto(char* string){
//     return string_from_format("%s/%s", path, string);
// }


int main(){
    mkdir("Bitacoras", 0664);
    char* p_tripulante = string_from_format("%s/%s", path, "Tripulante.ims");

    printf("%s", p_tripulante);
    // if(access(p_tripulante,F_OK)<0){
    //     printf("No existe archivo\n");
    //     printf("%s\n", p_tripulante);
        
    int creaFile = open(p_tripulante, O_CREAT | O_RDWR,0664);
    if(creaFile < 0){
        perror("Error: ");
    }
    //close(creaFile);
    // }

    //     int cantidadBloques = 64;
    //     int tamanioBloque = 64;
    //     void* blocks_memory;

    //     int posX_v, posY_v = 0;
    //     int posX_n = 3;
    //     int posY_n = 4;

    //     t_log *log = log_create("test.log", "TEST", 1, LOG_LEVEL_TRACE);

    //     FILE* testing = fopen("bitacora.ims","w");
    //     fclose(testing);
    //     t_config* bitacora = config_create("bitacora.ims");

    //     int blocks = open("blocks.ims", O_CREAT | O_RDWR, 0664);
    //     int tamanioAGuardar = (tamanioBloque * cantidadBloques);

    //     posix_fallocate(blocks, 0, tamanioAGuardar);
    //     blocks_memory = mmap(NULL,tamanioAGuardar, PROT_READ | PROT_WRITE, MAP_SHARED, blocks, 0);
    //     close(blocks);
        
    //     // Escribo en el archivo
    //     int err = msync(blocks_memory, tamanioAGuardar, MS_ASYNC);
    //     if (err == -1){
    //         log_error(log, "[Blocks] Error de sincronizar a disco tamañoBloque y cantidadBloques");
    //     }

    //     dictionary_put(bitacora->properties, "SIZE","0");
    //     dictionary_put(bitacora->properties, "BLOCKS","[]");
    //     dictionary_put(bitacora->properties, "MD5","-");
    //     config_save_in_file(bitacora,"config.ims");
        
    //     printf("%s", (char*) dictionary_get(bitacora->properties,"SIZE"));
    //     printf("%s", (char*) dictionary_get(bitacora->properties,"BLOCKS"));
    //     printf("%s", (char*) dictionary_get(bitacora->properties,"MD5"));
        
    //     char* stringCopiar = string_new();
    //     string_append(&stringCopiar,"Se mueve de ");
    //     string_append(&stringCopiar,string_itoa(posX_v));
    //     string_append(&stringCopiar,"|");
    //     string_append(&stringCopiar,string_itoa(posY_v));
    //     string_append(&stringCopiar," a ");
    //     string_append(&stringCopiar,string_itoa(posX_n));
    //     string_append(&stringCopiar,"'|");
    //     string_append(&stringCopiar,string_itoa(posY_n));
    //     string_append(&stringCopiar,"'");


    //     arch_bitmap = open("superBloque.ims", O_CREAT | O_RDWR, 0664);
    //     bitmap_memory = mmap(NULL, sizeof(int) * 2 + (cantidadBloques / 8), PROT_READ | PROT_WRITE, MAP_SHARED, arch_bitmap, 0);
    //     bitmap = bitarray_create_with_mode((char*)bitmap_memory + 8, cantidadBloques / 8, MSB_FIRST);  


    //     for(int i=0; i<cantidadBloques; i++){
    //         bitarray_clean_bit(bitmap,i);
    //         msync(bitmap->bitarray,cantidadBloques/8 ,0);
    //     }

    //     log_info(log, "Muestro mis valores del bitmap..");
    //     for(int i=0; i<cantidadBloques; i++){
    //         log_info(log,"%d",bitarray_test_bit(bitmap,i));
    //     }





    //     char* str= string_new();
    //     string_append(&str, "Hola como estas?. Mi nombre es Delfina Bibé"); 
    //     log_info(log,"%d", string_length(str));

    //     memcpy(blocks_memory + 3*tamanioBloque, str, string_length(str));
    //     msync(blocks_memory,string_length(str),MS_ASYNC);
    //     free(str);

    //     char* str_tarea="OOOOOOOOOOOOOOOOOOOOOOOOOOOO";
    //     memcpy(blocks_memory + 2*tamanioBloque, str, string_length(str_tarea));
    //     msync(blocks_memory,string_length(str_tarea),MS_ASYNC);
    //     free(str_tarea);


    //     int cantidadBloquesUsados = 0;
    //     int vueltas = 0;
    //     int cantidadBloquesAUsar = cantidadBloques_necesito(tamanioBloque,stringCopiar,cantidadBloques);
    //     int tamanioStr = string_length(stringCopiar);
    //     int contadorChars = 0;
    //     /*---------------------------------ACA EMPIEZA TESTEO LOGICA----------------------------*/

    //      for(int i=0; i < bitarray_get_max_bit(bitmap) && cantidadBloquesUsados != cantidadBloquesAUsar; i++){
    //         if(bitarray_test_bit(bitmap,i) == 0){
    //             if((cantidadBloquesAUsar-cantidadBloquesUsados)==1){ //ultimo bloque a escribir
    //                 //POSIBLE FRAGMENTACION INTERNA!
                    
    //                 memcpy(blocks_memory + i*tamanioBloque,stringCopiar+contadorChars*tamanioBloque,tamanioStr-contadorChars*tamanioBloque);
    //                 contadorChars ++;
    //                 cantidadBloquesUsados ++;
                    
    //                 //Actualizo metadata
    //                 t_config* bitacoraTripulante = config_create("bitacora.ims");
                   
    //                 int size = atoi(dictionary_get(bitacoraTripulante, "SIZE"));
    //                 size += tamanioBloque;
    //                 char* c_size = string_new();
    //                 strcpy(c_size,string_itoa(size));
    //                 dictionary_put(bitacoraTripulante,"SIZE",c_size);
    //                 free(c_size);

    //             }else{
                
    //                 //Escribo en bloque
    //                 memcpy(blocks_memory + i*tamanioBloque,stringCopiar+contadorChars*tamanioBloque,tamanioBloque);
    //                 contadorChars ++;
    //                 cantidadBloquesUsados ++;

    //                 //Actualizo metadata
    //                 t_config* bitacoraTripulante = config_create("bitacora.ims");
                   
    //                 int size = atoi(dictionary_get(bitacoraTripulante, "SIZE"));
    //                 size += tamanioBloque;
    //                 char* c_size = string_new();
    //                 strcpy(c_size,string_itoa(size));
    //                 dictionary_put(bitacoraTripulante,"SIZE",c_size);
    //                 free(c_size);


    //                 // dictionary_put(bitacoraTripulante->properties, "BLOCKS", );
    //                 // dictionary_put(bitacoraTripulante->properties, "BLOCKS", "[]");
    //                 // dictionary_put(bitacoraTripulante->properties, "MD5_ARCHIVO", "-");
                    
    //                 //LO GUARDO EN EL ARCHIVO EN SI Y LO DESTRUYO A LA VARIABLE
    //                 config_save_in_file(bitacoraTripulante,"bitacora.ims");
    //                 config_destroy(bitacoraTripulante);

    //             }
    //         }
    //      }
    //    // dictionary_destroy(bitacora->properties);
    //    // config_destroy(bitacora);






    return 0;
}
