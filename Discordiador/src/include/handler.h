#ifndef DISCORD_HANDLER_LIB
#define DISCORD_HANDLER_LIB

#include <stdio.h>
#include <commons/log.h>
#include <commons/config.h>

void handler(int fd, char *id, int opcode, char *buffer, t_log *logger);

#endif