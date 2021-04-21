#include "handler.h"

void handler(int fd, char *id, int opcode, char *buffer, t_log *logger) {
    log_info(logger, "Recibi los siguientes datos:");
    log_info(logger, "ID Proceso:\t%s", id);
    log_info(logger, "Comando:\t\t%d", opcode);
    log_info(logger, "Message:\t\t%s", buffer);
}