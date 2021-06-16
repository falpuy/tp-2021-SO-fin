#include "./headers/tareas.h"

int comandoTarea(char* tarea){
    if(!strcmp(tarea,"GENERAR_OXIGENO")) { return 1;}
    else if(!strcmp(tarea,"CONSUMIR_OXIGENO")) { return 2;}
    else if(!strcmp(tarea,"GENERAR_COMIDA")) { return 3;}
    else if(!strcmp(tarea,"GENERAR_BASURA")) { return 4;}
    else if(!strcmp(tarea,"DESCARTAR_BASURA")) { return 5;}
    else { return -1;}
}


void finalizaEjecutarTarea(int lenTarea,char* tarea,int parametro){
    
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
            log_error(logger,"[ComienzaEjecutarTarea] No existe comando");
        break;   
    }
}


void comienzaEjecutarTarea(int lenTarea,char* tarea,int parametro){
    
    int comando = comandoTarea(tarea);
    switch(comando){
        case GENERAR_OXIGENO:
            log_info(logger,"Generando oxigeno...");
            generarOxigeno(parametro);
        break;
        case CONSUMIR_OXIGENO:
            log_info(logger,"Consumiendo oxigeno...");
            consumirOxigeno(parametro);
        break;
        case GENERAR_COMIDA:
            log_info(logger,"Generando comida...");
            generarComida(parametro);
        break;
        case CONSUMIR_COMIDA:
            log_info(logger,"Consumiendo comida...");
            consumirComida(parametro);
        break;
        case GENERAR_BASURA:
            log_info(logger,"Generando basura...");
            generarBasura(parametro);

        break;
        case DESCARTAR_BASURA:
            log_info(logger,"Descartando basura...");
            descartarBasura(parametro);

        break;
        default:
            log_error(logger,"[ComienzaEjecutarTarea] No existe comando");
        break;   
    }
}


/*------------------------------------FUNCIONES DE LAS TAREAS------------------------------------------------*/
void consumirOxigeno(int parametroTarea){
    char* path_oxigeno = pathCompleto("Tareas/Oxigeno.ims");
    
    if(access(path_oxigeno,F_OK)<0){
        log_error(logger, "No existe Oxigeno.ims");
        log_error(logger, "Finalizando Tarea..");
        //TODO: FINALIZAR TAREA?
    }

    char*strABorrar = string_repeat('O',parametroTarea);
    //borrarEnBlocks(strABorrar,path_oxigeno,1,log);
    free(strABorrar);
    free(path_oxigeno);
}

void consumirComida(int parametroTarea){
    char* path_comida = pathCompleto("Tareas/Comida.ims");
    
    if(access(path_comida,F_OK)<0){
        log_error(logger, "No existe Comida.ims");
        log_error(logger, "Finalizando Tarea..");
        //TODO: FINALIZAR TAREA?
    }

    char*strABorrar = string_repeat('C',parametroTarea);
    //borrarEnBlocks(strABorrar,path_oxigeno,1,log);
    free(strABorrar);
    free(path_comida);
}

void descartarBasura(int parametroTarea){
    char* path_basura = pathCompleto("Tareas/Basura.ims");
    
    if(access(path_basura,F_OK)<0){
        log_error(logger, "No existe Basura.ims");
    }else{
        remove(path_basura);
    }
    free(path_basura);
}


void generarOxigeno(int parametroTarea){
    char* path_oxigeno = pathCompleto("Tareas/Oxigeno.ims");
    
    if(access(path_oxigeno,F_OK)<0){
        log_info(logger, "No existe Oxigeno.ims...Se crea archivo");
        crearMetadataFiles(path_oxigeno,"O");
    }

    char* strGuardar = string_repeat('O',parametroTarea);
    
    // pthread_mutex_lock(&m_blocks);
    guardarEnBlocks(strGuardar,path_oxigeno,1);
    // pthread_mutex_unlock(&m_blocks);


    free(strGuardar);
    free(path_oxigeno);
}

void generarComida(int parametroTarea){
    char* path_comida = pathCompleto("Tareas/Basura.ims");
    
    if(access(path_comida,F_OK)<0){
        log_info(logger, "No existe Oxigeno.ims...Se crea archivo");
        crearMetadataFiles(path_comida,"C");
    }

    char* strGuardar = string_repeat('C',parametroTarea);
    
    // pthread_mutex_lock(&m_blocks);
    guardarEnBlocks(strGuardar,path_comida,1);
    // pthread_mutex_unlock(&m_blocks);

    free(strGuardar);
    free(path_comida);
}

void generarBasura(int parametroTarea){
    char* path_basura = pathCompleto("Tareas/Basura.ims");
    
    if(access(path_basura,F_OK)<0){
        log_info(logger, "No existe Oxigeno.ims...Se crea archivo");
        crearMetadataFiles(path_basura,"B");
    }

    char* strGuardar = string_repeat('B',parametroTarea);
    
    // pthread_mutex_lock(&m_blocks);
    guardarEnBlocks(strGuardar,path_basura,1);
    // pthread_mutex_unlock(&m_blocks);

    free(strGuardar);
    free(path_basura);
}

