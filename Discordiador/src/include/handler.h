#ifndef DISCORD_HANDLER_LIB
#define DISCORD_HANDLER_LIB

#include <commons/log.h>

void handler(int fd, char *id, int opcode, char *buffer, t_log *logger);

#endif