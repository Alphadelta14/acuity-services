#ifndef _IRCD_H
#define _IRCD_H

#include "actypes.h"
#include "acuity.h"
#include "entity.h"

#define IRC_INSPIRCD 0x1


void (*send_raw_line)(const char *buff);
int (*connectIRC)(char *host, char *port);
int (*eventloopIRC)(void);
user *(*createUser)(char *uid, char *nick, char *ident, char *host, char *ip, char *vhost, char *gecos, char *modes);
char *(*generateUID)(void);
char (*isValidNick)(char *nick);
extern char isUser(char *target);
extern void setMode(char *senderid, char *target, char *modes);

int irc_socket;
metanode *remoteconf;
extern char cuid[];

#endif /* _IRCD_H */
