#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>


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

    //-------------------TEST CONEXION POR POOL THREADS (FUNCION GRUPO)
    
    //-------------------TEST SERIALIZACION PAYLOAD Y DESERIALIZACION

    //-------------------TEST CREAR STRING DE COMANDO Y TRATAR DE GUARDAR EN BLOQUES MAXIMOS (PARA BLOCKS.IMS)
    
    
    
    






    return 0;
}