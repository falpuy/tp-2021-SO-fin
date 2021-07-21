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

char *absolute_path;

const char *getAbsolutePath(const char *new_path) {

    // char *temp = string_from_format("%s/%s", absolute_path, new_path);
    size_t len = strlen(absolute_path) + strlen(new_path) + 1;
    char* fullpath = malloc(len);
    strcpy(fullpath, absolute_path);
    strcat(fullpath, new_path);
    return fullpath;
}

int main () {

    t_config *config = config_create("./test.config");

    absolute_path = config_get_string_value(config, "PATH");

    printf("PATH: %s\n", absolute_path);
    mkdir(absolute_path, 0666);

    const char* p_tripulante = getAbsolutePath("/Tripulante.ims");

    printf("PATH: %s\n", p_tripulante);
    printf("PATH: %s\n", "/home/kali/Desktop/shared/vm/tp-2021-1c-Unnamed-Group/MiRAMHQ/test/fs/Tripulante.ims");
    
    int creaFile = open("/home/kali/Desktop/shared/vm/tp-2021-1c-Unnamed-Group/MiRAMHQ/test/fs/Tripulante.ims", O_CREAT | O_RDWR, 0640);
    printf("FD: %d", creaFile);
    perror("Error");
    close(creaFile);

    config_destroy(config);

    return 0;
}
