#ifndef OPERSERV_H
#define OPERSERV_H

#include "nickserv.h"

extern user *operserv;
void registerOperServCommand(char *cmd, void (*callback)(char *uid, char *msg));
void fireOperServCommand(line *l);
void os_message(char *uid, char *str, ...);

#endif /* OPERSERV_H */
