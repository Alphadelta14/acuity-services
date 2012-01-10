#ifndef NETWORK_H
#define NETWORK_H

#include "entity.h"

usernode *userlist;
channode *chanlist;

int (*char2mode)(char modechar);
char *buildModes(int count, ...);

user *(*addUser)(char *uid, char *nick, char *ident, char *host, char *ip, char *vhost, char *gecos, char *modes);
user *(*getUser)(char *uid);
chan *(*addChannel)(char *name, char **modes);
chan *(*getChannel)(char *name);
statusnode *(*addChannelUser)(chan *C, user *U);
void (*delChannelUser)(chan *C, user *U);
int (*chanStatusAppend)(char *channame, char *status);
void changeMode(int *modeMinor, int *modeMajor, char *modes);
void changeNick(user *U, char *nick);

#endif /* NETWORK_H */
