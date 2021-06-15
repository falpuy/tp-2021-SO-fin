#include "./headers/tareas.h"

int comandoTarea(char* tarea){
    if(!strcmp(tarea,"GENERAR_OXIGENO")) { return 1;}
    else if(!strcmp(tarea,"CONSUMIR_OXIGENO")) { return 2;}
    else if(!strcmp(tarea,"GENERAR_COMIDA")) { return 3;}
    else if(!strcmp(tarea,"GENERAR_BASURA")) { return 4;}
    else if(!strcmp(tarea,"DESCARTAR_BASURA")) { return 5;}
    else { return -1;}
}

char* pathCompleto(char* strConcatenar){
    return string_from_format("%s/%s",datosConfig->puntoMontaje,strConcatenar);
}

void finalizaEjecutarTarea(int lenTarea,char* tarea,int parametro,t_log* log){
    
    int comando = comandoTarea(tarea);
    switch(comando){
        case GENERAR_OXIGENO:
        break;
        case CONSUMIR_OXIGENO:
        break;
        case GENERAR_COMIDA:
        break;
        case GENERAR_BASURA:
        break;
        case DESCARTAR_BASURA:
        break;
        default:
            log_error(log,"[ComienzaEjecutarTarea] No existe comando");
        break;   
    }
}


void comienzaEjecutarTarea(int lenTarea,char* tarea,int parametro,t_log* log){
    
    int comando = comandoTarea(tarea);
    switch(comando){
        case GENERAR_OXIGENO:
            log_info(log,"Generando oxigeno...");
            generarOxigeno(log,parametro);
        break;
        case CONSUMIR_OXIGENO:
            log_info(log,"Consumiendo oxigeno...");
            consumirOxigeno(log,parametro);
        break;
        case GENERAR_COMIDA:
            log_info(log,"Generando comida...");
            generarComida(log,parametro);
        break;
        case CONSUMIR_COMIDA:
            log_info(log,"Consumiendo comida...");
            consumirComida(log,parametro);
        break;
        case GENERAR_BASURA:
            log_info(log,"Generando basura...");
            generarBasura(log,parametro);

        break;
        case DESCARTAR_BASURA:
            log_info(log,"Descartando basura...");
            descartarBasura(log,parametro);

        break;
        default:
            log_error(log,"[ComienzaEjecutarTarea] No existe comando");
        break;   
    }
}


/*------------------------------------FUNCIONES DE LAS TAREAS------------------------------------------------*/
void consumirOxigeno(t_log* log, int parametroTarea){
    char* path_oxigeno = pathCompleto("Tareas/Oxigeno.ims");
    
    if(access(path_oxigeno,F_OK)<0){
        log_error(log, "No existe Oxigeno.ims");
        log_error(log, "Finalizando Tarea..");
        //TODO: FINALIZAR TAREA?
    }

    char*strABorrar = string_repeat('O',parametroTarea);
    //borrarEnBlocks(strABorrar,path_oxigeno,1,log);
    free(strABorrar);
    free(path_oxigeno);
}

void consumirComida(t_log* log, int parametroTarea){
    char* path_comida = pathCompleto("Tareas/Comida.ims");
    
    if(access(path_comida,F_OK)<0){
        log_error(log, "No existe Comida.ims");
        log_error(log, "Finalizando Tarea..");
        //TODO: FINALIZAR TAREA?
    }

    char*strABorrar = string_repeat('C',parametroTarea);
    //borrarEnBlocks(strABorrar,path_oxigeno,1,log);
    free(strABorrar);
    free(path_comida);
}

void descartarBasura(t_log* log, int parametroTarea){
    char* path_basura = pathCompleto("Tareas/Basura.ims");
    
    if(access(path_basura,F_OK)<0){
        log_error(log, "No existe Basura.ims");
    }else{
        remove(path_basura);
    }
    free(path_basura);
}


void generarOxigeno(t_log* log, int parametroTarea){
    char* path_oxigeno = pathCompleto("Tareas/Oxigeno.ims");
    
    if(access(path_oxigeno,F_OK)<0){
        log_info(log, "No existe Oxigeno.ims...Se crea archivo");
        crearMetadataFiles(path_oxigeno,"O",log);
    }

    char* strGuardar = string_repeat('O',parametroTarea);
    
    pthread_mutex_lock(&m_blocks);
    guardarEnBlocks(strGuardar,path_oxigeno,1,log);
    pthread_mutex_unlock(&m_blocks);


    free(strGuardar);
    free(path_oxigeno);
}

void generarComida(t_log* log, int parametroTarea){
    char* path_comida = pathCompleto("Tareas/Basura.ims");
    
    if(access(path_comida,F_OK)<0){
        log_info(log, "No existe Oxigeno.ims...Se crea archivo");
        crearMetadataFiles(path_comida,"C",log);
    }

    char* strGuardar = string_repeat('C',parametroTarea);
    
    pthread_mutex_lock(&m_blocks);
    guardarEnBlocks(strGuardar,path_comida,1,log);
    pthread_mutex_unlock(&m_blocks);

    free(strGuardar);
    free(path_comida);
}

void generarBasura(t_log* log, int parametroTarea){
    char* path_basura = pathCompleto("Tareas/Basura.ims");
    
    if(access(path_basura,F_OK)<0){
        log_info(log, "No existe Oxigeno.ims...Se crea archivo");
        crearMetadataFiles(path_basura,"B",log);
    }

    char* strGuardar = string_repeat('B',parametroTarea);
    
    pthread_mutex_lock(&m_blocks);
    guardarEnBlocks(strGuardar,path_basura,1,log);
    pthread_mutex_unlock(&m_blocks);

    free(strGuardar);
    free(path_basura);
}


/*----------------------------------------------FUNCIONES EXTRAS------------------------------------------------*/

void crearMetadataFiles(char* path,char* charLlenado, t_log* logger){
    int fd = creat(path,0666);
    if(fd < 0){
        perror("Error:");
    }else{
        close(fd);
        t_config* file = config_create(path);
        
        config_set_value(file, "SIZE", "0");
        config_set_value(file, "BLOCK_COUNT", "0");
        config_set_value(file, "BLOCKS", "[]");
        config_set_value(file, "CARACTER_LLENADO", charLlenado);
        config_set_value(file, "MD5", "0");

        config_save_in_file(file,path);
        config_destroy(file);

    }
}

void crearMetadataBitacora(char* path_tripulante, t_log* logger){
    int fd = creat(path_tripulante,0666);
    if(fd < 0){
        perror("Error:");
    }else{
        close(fd);
        t_config* bitacoraTripulante = config_create(path_tripulante);
        
        config_set_value(bitacoraTripulante, "SIZE", "0");
        config_set_value(bitacoraTripulante, "BLOCKS", "[]");

        config_save_in_file(bitacoraTripulante,path_tripulante);
        config_destroy(bitacoraTripulante);

    }
}


int cantidad_bloques(char* string, t_log* logger){
    double cantidad;
    double tamanioString = string_length(string);
    cantidad = tamanioString / tamanioBloque;
    int valorFinal = (int) ceil(cantidad);

    return valorFinal; //round up
}



char* strMoverTripultante(int idTripulante,int posX_v,int posY_v,int posX_n,int posY_n){
    char* temporal = string_new();
    char* posicion  = string_itoa(posX_v);
    string_append(&temporal,"Se mueve de ");
    string_append(&temporal,posicion);
    free(posicion);
    string_append(&temporal,"|");
    posicion  = string_itoa(posY_v);
    string_append(&temporal,posicion);
    free(posicion);
    string_append(&temporal," a ");
    posicion  = string_itoa(posX_n);
    string_append(&temporal,posicion);
    free(posicion);
    string_append(&temporal,"|");
    posicion = string_itoa(posY_n);
    string_append(&temporal,posicion);
    free(posicion);
    return temporal;
}

char* crearStrTripulante(int idTripulante){
    char* posicion = string_itoa(idTripulante);
    char* tripulante = string_new();
    string_append(&tripulante,"Tripulante");
    string_append(&tripulante,posicion);
    free(posicion);
    string_append(&tripulante,".ims");
    return tripulante;
}

int validarBitsLibre(int cantidadBloquesAUsar, t_log* log){
    int contador = 0;

    for(int i=0; i < bitarray_get_max_bit(bitmap); i++){
        if(bitarray_test_bit(bitmap,i) == 0){
            contador++;
            if(contador == cantidadBloquesAUsar){
                return 1;
            }
        }
    }
    return -1;
}