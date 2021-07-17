#include <stdlib.h>
#include <stdio.h>
#include <commons/string.h>
#include <commons/log.h>
#include <string.h>

char *get_tareas(char *ruta_archivo, t_log* logger) {
    FILE * fp;
    char * line = NULL;
    size_t len = 0;
    ssize_t read;

    int b_size = 0;
    int offset = 0;

    char *temp_string = string_new();

    fp = fopen(ruta_archivo, "r");
    if (fp == NULL)
        exit(EXIT_FAILURE);

    while ((read = getline(&line, &len, fp)) != -1) {

        if (line[ read - 1 ] == '\n') {
            read--;
            memset(line + read, 0, 1);
        }
        string_append(&temp_string, line);
    }

    fclose(fp);
    if (line)
        free(line);
    return temp_string;
}

int main(){

    t_log* logger = log_create("testing","testing",1,LOG_LEVEL_INFO);


    char * str = get_tareas("oxigeno.txt", logger);

    log_info(logger, "%s", str);
    free(str);

    return 1;
}