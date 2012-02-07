#ifndef _SERVICES_H
#define _SERVICES_H

#include "actypes.h"
#include "acuity.h"
#include "module.h"
#include "entity.h"
#include "events.h"
#include "ircd.h"
#include "network.h"
#include "config.h"
#include "db.h"
#include <stdarg.h>
#include <time.h>

typedef struct _service {
    char *uid;
    char *nick;
    char *ident;
    char *host;
    char *gecos;
    user *U;
} service;

typedef struct _commandnode {
    char *cmd;
    void (*callback)(char *uid, char *msg);
    struct _commandnode *next;
} commandnode;

typedef struct _helpnode {
    char *command;
    char *shorthelp;
    void (*longhelp)(char *uid, char *msg);
    struct _helpnode *next;
} helpnode;

typedef struct _setnode {
    char *option;
    char *shorthelp;
    void (*longhelp)(char *uid, char *msg);
    void (*callback)(char *uid, char *target, char *msg);
    struct _setnode *next;
} setnode;

extern char defaultTZ;
extern usernode *serviceusers;

user *createService(char *nick, char *host, char *ident, char *gecos);
void service_message(user *U, char *uid, char *str, ...);
void vservice_message(user *U, char *uid, char *str, va_list args);
void addHelp(helpnode **list, char *command, char *shorthelp, void (*longhelp)(char *uid, char *msg));
void addSetOption(setnode **list, char *option, char *shorthelp, void (*longhelp)(char *uid, char *msg), void (*callback)(char *uid, char *target, char *msg));
void fireHelp(user *U, helpnode *list, char *uid, char *msg);
void fireSetOption(user *U, setnode *list, char *uid, char *target, char *msg);
void fireSetHelp(user *U, setnode *list, char *uid, char *msg);
void addServiceCommand(commandnode **cmds, char *cmd, void (*callback)(char *uid, char *msg));
void fireServiceCommand(commandnode **cmds, user *service, line *l);
char *getTimeString(char *tz, time_t from);

/*typedef struct _serverconn {
    char *host;
    char *port;
    char *password;
    int protocol;
} serverconn;

extern service nickserv;
extern service chanserv;
extern service operserv;
extern service memoserv;
extern serverconn server;*/

#endif /* _SERVICES_H */
