#ifndef NETWORK_H
#define NETWORK_H

#include "entity.h"

usernode *userlist;
channode *chanlist;

int (*char2mode)(char modechar);
char *buildModes(int count, ...); /* Use this output in conjunction with setMode() */
char checkModes(int *modeM, int count, ...);
signed char irccmp(char *str1, char *str2);/* IRC casemapping versions of strcmp; use for nick comparison */
signed char irccasecmp(char *str1, char *str2);

/* Module developers: These functions aren't interesting to you in the
 * least. Seriously. They're just used internally to keep track of
 * users/channels and their changes; you shouldn't need to fiddle with
 * them unless you're coding an IRCd module.
 */
user *(*addUser)(char *uid, char *nick, char *ident, char *host, char *ip, char *vhost, char *gecos, char *modes);
user *(*getUser)(char *uid);
chan *(*addChannel)(char *name, char **modes);
chan *(*getChannel)(char *name);
statusnode *(*addChannelUser)(chan *C, user *U);
void (*delChannelUser)(chan *C, user *U);
int (*chanStatusAppend)(char *channame, char *status);
void changeMode(int *modeM, char *modes);
void changeNick(user *U, char *nick);
user *getUserByNick(char *nick);

#endif /* NETWORK_H */
