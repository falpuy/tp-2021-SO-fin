#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <commons/config.h>
#include <math.h>


int main(){


    //-------------------TEST DE LEVANTAR LO QUE ESCRIBI EN ARCH BINARIO (SUPERBLOQUE) -->FUNCIONA!
    // int value = 1;
    // FILE* testSB = fopen("tsb","wb");
    // fwrite(&value,sizeof(int),1,testSB);
    // fclose(testSB);   

    // int value2;
    // FILE* testSB = fopen("tsb","rb");
    // fread(&value2,sizeof(int),1,testSB);
    // printf("%d", value2);
    // fclose(testSB); 

    //-------------------TEST DE MAPEAR A MEMORIA UN ARCHIVO TEXTO Y MODIFICARLO -SINCRONIZARLO --> FUNCIONA!!

        // FILE* testB = fopen("tb","w");
        // void* punteroTamanioBlocks = calloc(10*8, 1);
        // fwrite(punteroTamanioBlocks,10*8 ,1 , testB);
        // fseek(testB,10*8,SEEK_SET);
        
        // char finArchivo = 'f';
        // fwrite(&finArchivo,sizeof(char),1, testB);
        // fclose(testB);

        // int testB = open("tb", O_CREAT | O_RDWR, 0664);
        // char* mapMem = mmap(NULL, 10*8, PROT_READ | PROT_WRITE, MAP_SHARED, testB, 0);

        // char* test = "OOO";
        // memcpy(mapMem,test,strlen(test)+1);
        // printf("Escribo OOO.........");

        // msync(mapMem,10*8,0);

        // close(testB);

    //-------------------TEST CONEXION POR POOL THREADS (FUNCION GRUPO) !FUNCIONA
    
    //-------------------TEST SERIALIZACION PAYLOAD Y DESERIALIZACION !FUNCIONA

    //-------------------TEST CREAR BITACORA COMO CONFIG !FUNCA
    
        // FILE* testing = fopen("config.ims","w");
        // fclose(testing);

        // t_config* bitacora = config_create("config.ims");
        // //t_dictionary* diccionario = dictionary_create();
	    
        // dictionary_put(bitacora->properties, "SIZE", "132");
        
        
        // //SIZE=132
        // // BLOCK_COUNT=3
        // // BLOCKS=[1,2,3]
        // // MD5_ARCHIVO=BD1014D173BA92CC014850A7087E254E

	    // config_save_in_file(bitacora,"config.ims");

    //-------------------TEST ceiling
    
    // double valor1 = 23;
    // int valor2 = 8;
    // double total = ceil(valor1/(double) valor2);
    // printf("%f\n",total);


    // printf("%d\n",(int) total);





    return 0;
}