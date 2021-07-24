#include "./headers/tareas.h"

int comandoTarea(char* tarea){
    if(!strcmp(tarea,"GENERAR_OXIGENO")){return GENERAR_OXIGENO;}
    else if (!strcmp(tarea,"CONSUMIR_OXIGENO")){return CONSUMIR_OXIGENO;}
    else if(!strcmp(tarea,"GENERAR_BASURA")){return GENERAR_BASURA;}
    else if(!strcmp(tarea,"DESCARTAR_BASURA")){return DESCARTAR_BASURA;}
    else if(!strcmp(tarea,"GENERAR_COMIDA")){return GENERAR_COMIDA;}
    else if(!strcmp(tarea,"CONSUMIR_COMIDA")){return CONSUMIR_COMIDA;}
    return TAREA_NO_IO;
}

void finalizaEjecutarTarea(int lenTarea,char* tarea,int idTripulante){ //preguntar que hace finalizar tarea ademas de escribir bitacora
    
    int comando = comandoTarea(tarea);
    switch(comando){
        case GENERAR_OXIGENO:
            log_info(logger,"Finaliza tarea:Generando oxigeno...");
            pthread_mutex_lock(&blocks_bitmap);
            escribirEnBitacoraFinalizaTarea(tarea,idTripulante);
            pthread_mutex_unlock(&blocks_bitmap);
        break;
        case CONSUMIR_OXIGENO:
            log_info(logger,"Finaliza tarea:Consumiendo oxigeno...");
            pthread_mutex_lock(&blocks_bitmap);
            escribirEnBitacoraFinalizaTarea(tarea,idTripulante);
            pthread_mutex_unlock(&blocks_bitmap);
        break;
        case GENERAR_COMIDA:
            log_info(logger,"Finaliza tarea:Generando comida...");
            pthread_mutex_lock(&blocks_bitmap);
            escribirEnBitacoraFinalizaTarea(tarea,idTripulante);
            pthread_mutex_unlock(&blocks_bitmap);
        break;
        case CONSUMIR_COMIDA:
            log_info(logger,"Finaliza tarea:Consumiendo comida...");
            pthread_mutex_lock(&blocks_bitmap);
            escribirEnBitacoraFinalizaTarea(tarea,idTripulante);
            pthread_mutex_unlock(&blocks_bitmap);
        break;
        case GENERAR_BASURA:
            log_info(logger,"Finaliza tarea:Generando basura...");

            pthread_mutex_lock(&blocks_bitmap);
            escribirEnBitacoraFinalizaTarea(tarea,idTripulante);
            pthread_mutex_unlock(&blocks_bitmap);
        break;
        case DESCARTAR_BASURA:
            log_info(logger,"Finaliza tarea:Descartando basura...");

            pthread_mutex_lock(&blocks_bitmap);
            escribirEnBitacoraFinalizaTarea(tarea,idTripulante);
            pthread_mutex_unlock(&blocks_bitmap);
        break;
        case TAREA_NO_IO:
            log_info(logger,"Finaliza tarea de no IO...");

            pthread_mutex_lock(&blocks_bitmap);
            escribirEnBitacoraFinalizaTarea(tarea,idTripulante);
            pthread_mutex_unlock(&blocks_bitmap);
        break;
        default:
            log_error(logger,"[ComienzaEjecutarTarea] No existe comando");
        break;   
    }
}


void comienzaEjecutarTarea(int lenTarea,char* tarea,int parametro,int idTripulante){
    
    int comando = comandoTarea(tarea);
    switch(comando){
        case GENERAR_OXIGENO:
            log_info(logger,"Generando oxigeno...");
            pthread_mutex_lock(&blocks_bitmap);
            escribirEnBitacoraComienzaTarea(tarea,idTripulante);
            generarOxigeno(parametro);
            pthread_mutex_unlock(&blocks_bitmap);
        break;
        case CONSUMIR_OXIGENO:
            log_info(logger,"Consumiendo oxigeno...");
            pthread_mutex_lock(&blocks_bitmap);
            escribirEnBitacoraComienzaTarea(tarea,idTripulante);
            consumirOxigeno(parametro);
            pthread_mutex_unlock(&blocks_bitmap);
        break;
        case GENERAR_COMIDA:
            log_info(logger,"Generando comida...");
            pthread_mutex_lock(&blocks_bitmap);
            escribirEnBitacoraComienzaTarea(tarea,idTripulante);
            generarComida(parametro);
            pthread_mutex_unlock(&blocks_bitmap);
        break;
        case CONSUMIR_COMIDA:
            log_info(logger,"Consumiendo comida...");
            pthread_mutex_lock(&blocks_bitmap);
            escribirEnBitacoraComienzaTarea(tarea,idTripulante);
            consumirComida(parametro);
            pthread_mutex_unlock(&blocks_bitmap);
        break;
        case GENERAR_BASURA:
            log_info(logger,"Generando basura...");

            pthread_mutex_lock(&blocks_bitmap);
            escribirEnBitacoraComienzaTarea(tarea,idTripulante);
            generarBasura(parametro);
            pthread_mutex_unlock(&blocks_bitmap);
        break;
        case DESCARTAR_BASURA:
            log_info(logger,"Descartando basura...");

            pthread_mutex_lock(&blocks_bitmap);
            escribirEnBitacoraComienzaTarea(tarea,idTripulante);
            descartarBasura(parametro);
            pthread_mutex_unlock(&blocks_bitmap);
        break;
        case TAREA_NO_IO:
            log_info(logger,"Ejecutando tarea de no IO...");

            pthread_mutex_lock(&blocks_bitmap);
            escribirEnBitacoraComienzaTarea(tarea,idTripulante);
            pthread_mutex_unlock(&blocks_bitmap);
        break;
        default:
            log_error(logger,"[ComienzaEjecutarTarea] No existe comando");
        break;   
    }
}


/*------------------------------------FUNCIONES DE LAS TAREAS------------------------------------------------*/


void escribirEnBitacoraFinalizaTarea(char* tarea, int idTripulante){
    char* stringTemporal = string_new();
    string_append(&stringTemporal, "FINALIZA_EJECUCION_TAREA ");
    string_append(&stringTemporal,tarea);
    string_append(&stringTemporal, "|");

    char* tripulante = crearStrTripulante(idTripulante);//Creo el path de tripulanteN.ims
            
    char* bitacora = string_new();
    string_append(&bitacora, "Bitacoras/");
    string_append(&bitacora,tripulante);


    char* path_fileTripulante = pathCompleto(bitacora);
    free(bitacora);
    free(tripulante);

    if(access(path_fileTripulante,F_OK) < 0){
        log_info(logger,"No existe archivo en bitácora...Se crea archivo para este tripulante...");
        crearMetadataBitacora(path_fileTripulante);
    }

    guardarEnBlocks(stringTemporal,path_fileTripulante,0);
    free(stringTemporal);
    free(path_fileTripulante);
}

void escribirEnBitacoraComienzaTarea(char* tarea, int idTripulante){
    char* stringTemporal = string_new();
    string_append(&stringTemporal, "COMIENZA_EJECUCION_TAREA ");
    string_append(&stringTemporal,tarea);
    string_append(&stringTemporal,"|");

    char* tripulante = crearStrTripulante(idTripulante); //Creo el path de tripulanteN.ims

    char* bitacora = string_new();
    string_append(&bitacora, "Bitacoras/");
    string_append(&bitacora,tripulante);

    char* path_fileTripulante = pathCompleto(bitacora);
    free(bitacora);
    free(tripulante);

    if(access(path_fileTripulante,F_OK) < 0){
        log_info(logger,"No existe archivo en bitácora...Se crea archivo para este tripulante...");
        crearMetadataBitacora(path_fileTripulante);
    }
    guardarEnBlocks(stringTemporal,path_fileTripulante,0);
    free(stringTemporal);
    free(path_fileTripulante);
}

void consumirOxigeno(int parametroTarea){
    char* path_oxigeno = pathCompleto("Files/Oxigeno.ims");
    log_info(logger, "Path de consumir oxigeno:%s", path_oxigeno);
    
    if(access(path_oxigeno,F_OK)<0){
        log_error(logger, "No existe Oxigeno.ims");
        log_error(logger, "Finalizando Tarea..");
    }
    char* strABorrar = string_repeat('O',parametroTarea);
    
    borrarEnBlocks(strABorrar,path_oxigeno,1);

    free(strABorrar);
    free(path_oxigeno);
}

void consumirComida(int parametroTarea){
    char* path_comida = pathCompleto("Files/Comida.ims");
    log_info(logger, "Path de (Consumir)comida:%s", path_comida);
    
    if(access(path_comida,F_OK)<0){
        log_error(logger, "No existe Comida.ims");
        log_error(logger, "Finalizando Tarea..");
    }

    char* strABorrar = string_repeat('C',parametroTarea);
    
    borrarEnBlocks(strABorrar,path_comida,1);
    
    free(strABorrar);
    free(path_comida);
}

void descartarBasura(int parametroTarea){
    char* path_basura = pathCompleto("Files/Basura.ims");
    
    if(access(path_basura,F_OK)<0){
        log_error(logger, "No existe Basura.ims");
    }else{
        t_config* metadata = config_create(path_basura);
        char** listaBloques = config_get_array_value(metadata,"BLOCKS");
        int contador = 0;
        int bloque;
        char* string_temp = string_new();

        while(listaBloques[contador]){ 
            contador++;
        }

        int bloquesHastaAhora = 0;
        for(int i = 0; i <= contador; i++){
            if((contador - bloquesHastaAhora) > 1){
                bloque = atoi(listaBloques[bloquesHastaAhora]);

                char* temporalBloque = malloc(tamanioBloque+1);
                memcpy(temporalBloque, copiaBlocks + bloque*tamanioBloque, tamanioBloque);
                temporalBloque[tamanioBloque] = '\0';
                
                string_append(&string_temp,temporalBloque);
                bloquesHastaAhora++;
                free(temporalBloque);
            }else{
                bloque = atoi(listaBloques[bloquesHastaAhora]);

                int sizeVieja = config_get_int_value(metadata, "SIZE");
                int fragmentacion = contador*tamanioBloque - sizeVieja;

                char* temporalBloque = malloc(fragmentacion+1);
                memcpy(temporalBloque, copiaBlocks + bloque*tamanioBloque, fragmentacion);
                temporalBloque[fragmentacion] = '\0';
                
                string_append(&string_temp,temporalBloque);
                free(temporalBloque);
            }
        }

        config_destroy(metadata);
        for(int i = 0; i < contador; i++){
            free(listaBloques[i]);
        }
        free(listaBloques);

        borrarEnBlocks(string_temp,path_basura,1);
        free(string_temp);
        remove(path_basura);

    }
    free(path_basura);
}


void generarOxigeno(int parametroTarea){
    char* path_oxigeno = pathCompleto("Files/Oxigeno.ims");
    log_info(logger, "Path de generar oxigeno:%s", path_oxigeno);

    if(access(path_oxigeno,F_OK)<0){
        log_info(logger, "No existe Oxigeno.ims...Se crea archivo");
        crearMetadataFiles(path_oxigeno,"O");
    }

    char* strGuardar = string_repeat('O',parametroTarea);
    
    guardarEnBlocks(strGuardar,path_oxigeno,1);

    free(strGuardar);
    free(path_oxigeno);
}

void generarComida(int parametroTarea){
    char* path_comida = pathCompleto("Files/Comida.ims");
    log_info(logger, "Path de comida:%s", path_comida);

    if(access(path_comida,F_OK)<0){
        log_info(logger, "No existe Comida.ims...Se crea archivo");
        crearMetadataFiles(path_comida,"C");
    }

    char* strGuardar = string_repeat('C',parametroTarea);
    
    guardarEnBlocks(strGuardar,path_comida,1);

    free(strGuardar);
    free(path_comida);
}

void generarBasura(int parametroTarea){
    char* path_basura = pathCompleto("Files/Basura.ims");
    log_info(logger, "Path de basura:%s", path_basura);

    if(access(path_basura,F_OK)<0){
        log_info(logger, "No existe Basura.ims...Se crea archivo");
        crearMetadataFiles(path_basura,"B");
    }

    char* strGuardar = string_repeat('B',parametroTarea);
    
    guardarEnBlocks(strGuardar,path_basura,1);

    free(strGuardar);
    free(path_basura);
}

